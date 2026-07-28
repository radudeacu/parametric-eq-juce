#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Parameters/ParameterLayout.h"
#include "Parameters/ParameterIDs.h"

ParametricEQAudioProcessor::ParametricEQAudioProcessor()
    : AudioProcessor (BusesProperties()
                           .withInput ("Input", juce::AudioChannelSet::stereo(), true)
                           .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
{
}

void ParametricEQAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    const juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) samplesPerBlock,
                                         (juce::uint32) getTotalNumOutputChannels() };

    lowBand.prepare (spec);
    lowMidBand.prepare (spec);
    midBand.prepare (spec);
    highMidBand.prepare (spec);
    highBand.prepare (spec);

    outputGain.prepare (spec);
    outputGain.setRampDurationSeconds (0.02);

    testToneGenerator.prepare (spec);

    analyzerFifo.prepare (sampleRate, samplesPerBlock);
}

void ParametricEQAudioProcessor::releaseResources()
{
    lowBand.reset();
    lowMidBand.reset();
    midBand.reset();
    highMidBand.reset();
    highBand.reset();
    outputGain.reset();
    testToneGenerator.reset();
    analyzerFifo.reset();
}

bool ParametricEQAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::mono()
        || layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

LowBandType ParametricEQAudioProcessor::decodeLowBandType (float rawChoiceValue)
{
    return rawChoiceValue < 0.5f ? LowBandType::HighPass : LowBandType::LowShelf;
}

HighBandType ParametricEQAudioProcessor::decodeHighBandType (float rawChoiceValue)
{
    return rawChoiceValue < 0.5f ? HighBandType::LowPass : HighBandType::HighShelf;
}

int ParametricEQAudioProcessor::decodeSlope (float rawChoiceValue)
{
    return 12 * (1 + (int) rawChoiceValue);
}

void ParametricEQAudioProcessor::updateBandParameters (int numSamples)
{
    const auto band1Type = decodeLowBandType (apvts.getRawParameterValue (ParamIDs::band1Type)->load());
    const int band1Slope = decodeSlope (apvts.getRawParameterValue (ParamIDs::band1Slope)->load());

    lowBand.setParameters (band1Type,
                            apvts.getRawParameterValue (ParamIDs::band1FreqHz)->load(),
                            apvts.getRawParameterValue (ParamIDs::band1GainDb)->load(),
                            apvts.getRawParameterValue (ParamIDs::band1Q)->load(),
                            band1Slope, numSamples);

    lowMidBand.setParameters (apvts.getRawParameterValue (ParamIDs::band2FreqHz)->load(),
                               apvts.getRawParameterValue (ParamIDs::band2GainDb)->load(),
                               apvts.getRawParameterValue (ParamIDs::band2Q)->load(), numSamples);

    midBand.setParameters (apvts.getRawParameterValue (ParamIDs::band3FreqHz)->load(),
                            apvts.getRawParameterValue (ParamIDs::band3GainDb)->load(),
                            apvts.getRawParameterValue (ParamIDs::band3Q)->load(), numSamples);

    highMidBand.setParameters (apvts.getRawParameterValue (ParamIDs::band4FreqHz)->load(),
                                apvts.getRawParameterValue (ParamIDs::band4GainDb)->load(),
                                apvts.getRawParameterValue (ParamIDs::band4Q)->load(), numSamples);

    const auto band5Type = decodeHighBandType (apvts.getRawParameterValue (ParamIDs::band5Type)->load());
    const int band5Slope = decodeSlope (apvts.getRawParameterValue (ParamIDs::band5Slope)->load());

    highBand.setParameters (band5Type,
                             apvts.getRawParameterValue (ParamIDs::band5FreqHz)->load(),
                             apvts.getRawParameterValue (ParamIDs::band5GainDb)->load(),
                             apvts.getRawParameterValue (ParamIDs::band5Q)->load(),
                             band5Slope, numSamples);

    outputGain.setGainDecibels (apvts.getRawParameterValue (ParamIDs::outputGainDb)->load());
}

void ParametricEQAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    testToneGenerator.setParameters (apvts.getRawParameterValue (ParamIDs::testToneEnabled)->load() > 0.5f,
                                      apvts.getRawParameterValue (ParamIDs::testToneFreqHz)->load(),
                                      apvts.getRawParameterValue (ParamIDs::testToneGainDb)->load());
    testToneGenerator.process (buffer);

    // Pre-EQ tap: runs after the test tone substitution but before filtering,
    // and even while bypassed.
    analyzerFifo.pushSamples (buffer);

    if (apvts.getRawParameterValue (ParamIDs::bypass)->load() > 0.5f)
        return;

    updateBandParameters (buffer.getNumSamples());

    juce::dsp::AudioBlock<float> block (buffer);
    juce::dsp::ProcessContextReplacing<float> context (block);

    lowBand.process (context);
    lowMidBand.process (context);
    midBand.process (context);
    highMidBand.process (context);
    highBand.process (context);
    outputGain.process (context);
}

juce::AudioProcessorEditor* ParametricEQAudioProcessor::createEditor()
{
    return new ParametricEQAudioProcessorEditor (*this);
}

void ParametricEQAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();

    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void ParametricEQAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

float ParametricEQAudioProcessor::getMagnitudeForFrequencyDb (double freqHz) const
{
    float result = 0.0f;
    getMagnitudeResponseDb (&freqHz, &result, 1);
    return result;
}

void ParametricEQAudioProcessor::getMagnitudeResponseDb (const double* frequenciesHz, float* magnitudesDbOut,
                                                          int numPoints) const
{
    if (apvts.getRawParameterValue (ParamIDs::bypass)->load() > 0.5f)
    {
        for (int i = 0; i < numPoints; ++i)
            magnitudesDbOut[i] = 0.0f;

        return;
    }

    const double sampleRate = analyzerFifo.getSampleRate();

    const auto band1Type = decodeLowBandType (apvts.getRawParameterValue (ParamIDs::band1Type)->load());
    const int band1Slope = decodeSlope (apvts.getRawParameterValue (ParamIDs::band1Slope)->load());
    const float band1Freq = apvts.getRawParameterValue (ParamIDs::band1FreqHz)->load();
    const float band1Gain = apvts.getRawParameterValue (ParamIDs::band1GainDb)->load();
    const float band1Q = apvts.getRawParameterValue (ParamIDs::band1Q)->load();

    const float band2Freq = apvts.getRawParameterValue (ParamIDs::band2FreqHz)->load();
    const float band2Gain = apvts.getRawParameterValue (ParamIDs::band2GainDb)->load();
    const float band2Q = apvts.getRawParameterValue (ParamIDs::band2Q)->load();

    const float band3Freq = apvts.getRawParameterValue (ParamIDs::band3FreqHz)->load();
    const float band3Gain = apvts.getRawParameterValue (ParamIDs::band3GainDb)->load();
    const float band3Q = apvts.getRawParameterValue (ParamIDs::band3Q)->load();

    const float band4Freq = apvts.getRawParameterValue (ParamIDs::band4FreqHz)->load();
    const float band4Gain = apvts.getRawParameterValue (ParamIDs::band4GainDb)->load();
    const float band4Q = apvts.getRawParameterValue (ParamIDs::band4Q)->load();

    const auto band5Type = decodeHighBandType (apvts.getRawParameterValue (ParamIDs::band5Type)->load());
    const int band5Slope = decodeSlope (apvts.getRawParameterValue (ParamIDs::band5Slope)->load());
    const float band5Freq = apvts.getRawParameterValue (ParamIDs::band5FreqHz)->load();
    const float band5Gain = apvts.getRawParameterValue (ParamIDs::band5GainDb)->load();
    const float band5Q = apvts.getRawParameterValue (ParamIDs::band5Q)->load();

    const float outputGainDb = apvts.getRawParameterValue (ParamIDs::outputGainDb)->load();

    for (int i = 0; i < numPoints; ++i)
    {
        const double freqHz = frequenciesHz[i];

        float magnitude = LowBand::getMagnitudeForFrequency (band1Type, freqHz, sampleRate, band1Freq, band1Gain,
                                                              band1Q, band1Slope);
        magnitude *= PeakBand::getMagnitudeForFrequency (freqHz, sampleRate, band2Freq, band2Gain, band2Q);
        magnitude *= PeakBand::getMagnitudeForFrequency (freqHz, sampleRate, band3Freq, band3Gain, band3Q);
        magnitude *= PeakBand::getMagnitudeForFrequency (freqHz, sampleRate, band4Freq, band4Gain, band4Q);
        magnitude *= HighBand::getMagnitudeForFrequency (band5Type, freqHz, sampleRate, band5Freq, band5Gain,
                                                          band5Q, band5Slope);

        magnitudesDbOut[i] = juce::Decibels::gainToDecibels (magnitude) + outputGainDb;
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ParametricEQAudioProcessor();
}
