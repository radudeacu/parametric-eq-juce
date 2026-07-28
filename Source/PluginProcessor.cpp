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

    analyzerFifo.prepare (samplesPerBlock);
}

void ParametricEQAudioProcessor::releaseResources()
{
    lowBand.reset();
    lowMidBand.reset();
    midBand.reset();
    highMidBand.reset();
    highBand.reset();
    outputGain.reset();
    analyzerFifo.reset();
}

bool ParametricEQAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::mono()
        || layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void ParametricEQAudioProcessor::updateBandParameters (int numSamples)
{
    const auto band1Type = apvts.getRawParameterValue (ParamIDs::band1Type)->load() < 0.5f
                                ? LowBandType::HighPass : LowBandType::LowShelf;
    const int band1Slope = 12 * (1 + (int) apvts.getRawParameterValue (ParamIDs::band1Slope)->load());

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

    const auto band5Type = apvts.getRawParameterValue (ParamIDs::band5Type)->load() < 0.5f
                                ? HighBandType::LowPass : HighBandType::HighShelf;
    const int band5Slope = 12 * (1 + (int) apvts.getRawParameterValue (ParamIDs::band5Slope)->load());

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

    // Pre-EQ tap: runs before filtering and even while bypassed.
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

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ParametricEQAudioProcessor();
}
