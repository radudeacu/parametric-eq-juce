#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "DSP/LowBand.h"
#include "DSP/HighBand.h"
#include "DSP/PeakBand.h"
#include "DSP/TestToneGenerator.h"
#include "Analyzer/SpectrumAnalyzerFifo.h"

class ParametricEQAudioProcessor : public juce::AudioProcessor
{
public:
    ParametricEQAudioProcessor();

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // Pure UI-thread query: independently rebuilds each band's coefficients
    // from APVTS parameter values rather than reading any live audio-thread
    // DSP state, so it's safe to call from the editor at any time.
    float getMagnitudeForFrequencyDb (double freqHz) const;
    void getMagnitudeResponseDb (const double* frequenciesHz, float* magnitudesDbOut, int numPoints) const;

    juce::AudioProcessorValueTreeState apvts;
    SpectrumAnalyzerFifo analyzerFifo;

private:
    void updateBandParameters (int numSamples);

    static LowBandType decodeLowBandType (float rawChoiceValue);
    static HighBandType decodeHighBandType (float rawChoiceValue);
    static int decodeSlope (float rawChoiceValue);

    LowBand lowBand;
    PeakBand lowMidBand, midBand, highMidBand;
    HighBand highBand;
    juce::dsp::Gain<float> outputGain;
    TestToneGenerator testToneGenerator;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParametricEQAudioProcessor)
};
