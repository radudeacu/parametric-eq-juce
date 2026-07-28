#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <cmath>
#include <iostream>
#include "DSP/SlopeFilterChain.h"
#include "DSP/LowBand.h"
#include "DSP/HighBand.h"
#include "DSP/PeakBand.h"
#include "DSP/TestToneGenerator.h"
#include "Parameters/ParameterLayout.h"
#include "Parameters/ParameterIDs.h"

namespace
{
    // Minimal AudioProcessor host purely so createParameterLayout() can be
    // exercised through a real AudioProcessorValueTreeState in isolation
    // from the plugin client (which needs JucePlugin_* macros this test
    // executable doesn't define).
    class DummyProcessor : public juce::AudioProcessor
    {
    public:
        DummyProcessor()
            : juce::AudioProcessor (BusesProperties()
                                         .withInput ("In", juce::AudioChannelSet::stereo())
                                         .withOutput ("Out", juce::AudioChannelSet::stereo())),
              apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
        {
        }

        void prepareToPlay (double, int) override {}
        void releaseResources() override {}
        void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override {}
        juce::AudioProcessorEditor* createEditor() override { return nullptr; }
        bool hasEditor() const override { return false; }
        const juce::String getName() const override { return "Test"; }
        bool acceptsMidi() const override { return false; }
        bool producesMidi() const override { return false; }
        double getTailLengthSeconds() const override { return 0.0; }
        int getNumPrograms() override { return 1; }
        int getCurrentProgram() override { return 0; }
        void setCurrentProgram (int) override {}
        const juce::String getProgramName (int) override { return {}; }
        void changeProgramName (int, const juce::String&) override {}
        void getStateInformation (juce::MemoryBlock&) override {}
        void setStateInformation (const void*, int) override {}

        juce::AudioProcessorValueTreeState apvts;
    };

    juce::AudioBuffer<float> makeNoiseBuffer (int numSamples)
    {
        juce::AudioBuffer<float> buffer (1, numSamples);
        juce::Random random (1234);

        for (int i = 0; i < numSamples; ++i)
            buffer.setSample (0, i, random.nextFloat() * 2.0f - 1.0f);

        return buffer;
    }

    bool allSamplesFinite (const juce::AudioBuffer<float>& buffer)
    {
        for (int i = 0; i < buffer.getNumSamples(); ++i)
            if (! std::isfinite (buffer.getSample (0, i)))
                return false;

        return true;
    }
}

class ParameterLayoutTests : public juce::UnitTest
{
public:
    ParameterLayoutTests() : juce::UnitTest ("ParameterLayout") {}

    void runTest() override
    {
        DummyProcessor processor;

        beginTest ("Creates the expected 24 parameters");
        expectEquals (processor.getParameters().size(), 24);

        beginTest ("Key parameter IDs are present");
        expect (processor.apvts.getParameter (ParamIDs::band1Type) != nullptr);
        expect (processor.apvts.getParameter (ParamIDs::band5Slope) != nullptr);
        expect (processor.apvts.getParameter (ParamIDs::bypass) != nullptr);
        expect (processor.apvts.getParameter (ParamIDs::outputGainDb) != nullptr);
        expect (processor.apvts.getParameter (ParamIDs::testToneEnabled) != nullptr);
        expect (processor.apvts.getParameter (ParamIDs::testToneFreqHz) != nullptr);
        expect (processor.apvts.getParameter (ParamIDs::testToneGainDb) != nullptr);
    }
};

class SlopeFilterChainTests : public juce::UnitTest
{
public:
    SlopeFilterChainTests() : juce::UnitTest ("SlopeFilterChain") {}

    void runTest() override
    {
        constexpr double sampleRate = 48000.0;
        constexpr int blockSize = 512;
        const juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) blockSize, 1 };

        beginTest ("HighPass output stays finite across all slope options");
        for (int slope : { 12, 24, 36 })
        {
            SlopeFilterChain chain;
            chain.prepare (spec);
            chain.update (SlopeFilterKind::HighPass, sampleRate, 200.0f, 0.707f, 0.0f, slope);

            auto buffer = makeNoiseBuffer (blockSize);
            juce::dsp::AudioBlock<float> block (buffer);
            juce::dsp::ProcessContextReplacing<float> context (block);
            chain.process (context);

            expect (allSamplesFinite (buffer), "finite at slope " + juce::String (slope));
        }

        beginTest ("LowShelf output stays finite with gain applied across all slope options");
        for (int slope : { 12, 24, 36 })
        {
            SlopeFilterChain chain;
            chain.prepare (spec);
            chain.update (SlopeFilterKind::LowShelf, sampleRate, 200.0f, 0.707f, 6.0f, slope);

            auto buffer = makeNoiseBuffer (blockSize);
            juce::dsp::AudioBlock<float> block (buffer);
            juce::dsp::ProcessContextReplacing<float> context (block);
            chain.process (context);

            expect (allSamplesFinite (buffer), "finite at slope " + juce::String (slope));
        }

        beginTest ("getMagnitudeForFrequency reads back ~-3dB at a Butterworth HighPass cutoff");
        for (int slope : { 12, 24, 36 })
        {
            const float magnitude = SlopeFilterChain::getMagnitudeForFrequency (
                SlopeFilterKind::HighPass, 200.0, sampleRate, 200.0f, 0.707f, 0.0f, slope);
            const float magnitudeDb = juce::Decibels::gainToDecibels (magnitude);

            expect (std::abs (magnitudeDb - (-3.01f)) < 1.0f,
                    "expected ~-3dB at cutoff for slope " + juce::String (slope) + ", got " + juce::String (magnitudeDb));
        }
    }
};

class BandSmoothingTests : public juce::UnitTest
{
public:
    BandSmoothingTests() : juce::UnitTest ("Band parameter smoothing") {}

    void runTest() override
    {
        constexpr double sampleRate = 48000.0;
        constexpr int blockSize = 256;
        constexpr int numBlocks = 50;
        const juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) blockSize, 1 };

        beginTest ("PeakBand stays finite while gain is automated across many blocks");
        {
            PeakBand band;
            band.prepare (spec);

            for (int b = 0; b < numBlocks; ++b)
            {
                const float targetGainDb = juce::jmap ((float) b, 0.0f, (float) (numBlocks - 1), -18.0f, 18.0f);
                band.setParameters (1000.0f, targetGainDb, 1.0f, blockSize);

                auto buffer = makeNoiseBuffer (blockSize);
                juce::dsp::AudioBlock<float> block (buffer);
                juce::dsp::ProcessContextReplacing<float> context (block);
                band.process (context);

                expect (allSamplesFinite (buffer), "finite at block " + juce::String (b));
            }
        }

        beginTest ("LowBand stays finite while switching type mid-stream");
        {
            LowBand band;
            band.prepare (spec);

            for (int b = 0; b < numBlocks; ++b)
            {
                const auto type = (b < numBlocks / 2) ? LowBandType::HighPass : LowBandType::LowShelf;
                band.setParameters (type, 100.0f, 6.0f, 0.707f, 24, blockSize);

                auto buffer = makeNoiseBuffer (blockSize);
                juce::dsp::AudioBlock<float> block (buffer);
                juce::dsp::ProcessContextReplacing<float> context (block);
                band.process (context);

                expect (allSamplesFinite (buffer), "finite at block " + juce::String (b));
            }
        }

        beginTest ("PeakBand::getMagnitudeForFrequency reads back its own gain at center frequency");
        for (float gainDb : { -12.0f, 0.0f, 12.0f })
        {
            const float magnitude = PeakBand::getMagnitudeForFrequency (1000.0, sampleRate, 1000.0f, gainDb, 1.0f);
            const float magnitudeDb = juce::Decibels::gainToDecibels (magnitude);

            expect (std::abs (magnitudeDb - gainDb) < 0.1f,
                    "expected ~" + juce::String (gainDb) + "dB at center, got " + juce::String (magnitudeDb));
        }
    }
};

class TestToneGeneratorTests : public juce::UnitTest
{
public:
    TestToneGeneratorTests() : juce::UnitTest ("TestToneGenerator") {}

    void runTest() override
    {
        constexpr double sampleRate = 48000.0;
        constexpr int blockSize = 512;
        const juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) blockSize, 2 };

        beginTest ("Disabled generator leaves the buffer untouched");
        {
            TestToneGenerator generator;
            generator.prepare (spec);
            generator.setParameters (false, 1000.0f, -6.0f);

            auto buffer = makeNoiseBuffer (blockSize);
            juce::AudioBuffer<float> original;
            original.makeCopyOf (buffer);

            generator.process (buffer);

            bool unchanged = true;
            for (int i = 0; i < blockSize; ++i)
                unchanged &= (buffer.getSample (0, i) == original.getSample (0, i));

            expect (unchanged);
        }

        beginTest ("Enabled generator produces a finite, level-bounded sine");
        {
            TestToneGenerator generator;
            generator.prepare (spec);

            juce::AudioBuffer<float> buffer (2, blockSize);

            // Run several blocks so the 20ms level ramp settles.
            for (int b = 0; b < 10; ++b)
            {
                generator.setParameters (true, 1000.0f, -6.0f);
                generator.process (buffer);
            }

            const float expectedPeak = juce::Decibels::decibelsToGain (-6.0f);
            bool allFinite = true;
            float maxAbs = 0.0f;

            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
                for (int i = 0; i < blockSize; ++i)
                {
                    const float sample = buffer.getSample (ch, i);
                    allFinite &= std::isfinite (sample);
                    maxAbs = juce::jmax (maxAbs, std::abs (sample));
                }

            expect (allFinite);
            expect (maxAbs < expectedPeak * 1.01f, "peak should not exceed the target level");
            expect (maxAbs > expectedPeak * 0.9f, "peak should have settled near the target level");
        }
    }
};

static ParameterLayoutTests parameterLayoutTests;
static SlopeFilterChainTests slopeFilterChainTests;
static BandSmoothingTests bandSmoothingTests;
static TestToneGeneratorTests testToneGeneratorTests;

int main()
{
    juce::UnitTestRunner runner;
    runner.runAllTests();

    bool anyFailures = false;

    for (int i = 0; i < runner.getNumResults(); ++i)
    {
        const auto* result = runner.getResult (i);
        std::cout << result->unitTestName << " / " << result->subcategoryName
                   << ": " << result->passes << " passed, " << result->failures << " failed\n";

        if (result->failures > 0)
        {
            anyFailures = true;
            for (const auto& message : result->messages)
                std::cout << "    " << message << "\n";
        }
    }

    return anyFailures ? 1 : 0;
}
