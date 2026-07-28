#include "ParameterLayout.h"
#include "ParameterIDs.h"

namespace
{
    constexpr int paramVersion = 1;

    juce::NormalisableRange<float> makeFreqRange (float minHz, float maxHz, float centreHz)
    {
        juce::NormalisableRange<float> range (minHz, maxHz, 1.0f);
        range.setSkewForCentre (centreHz);
        return range;
    }

    juce::NormalisableRange<float> makeQRange()
    {
        juce::NormalisableRange<float> range (0.1f, 10.0f, 0.01f);
        range.setSkewForCentre (1.0f);
        return range;
    }

    juce::NormalisableRange<float> makeGainRange()
    {
        return { -24.0f, 24.0f, 0.1f };
    }

    std::unique_ptr<juce::AudioParameterFloat> makeFreqParam (const juce::String& id, const juce::String& name,
                                                                float minHz, float maxHz, float defaultHz)
    {
        return std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { id, paramVersion }, name,
            makeFreqRange (minHz, maxHz, defaultHz), defaultHz,
            juce::AudioParameterFloatAttributes().withLabel ("Hz"));
    }

    std::unique_ptr<juce::AudioParameterFloat> makeGainParam (const juce::String& id, const juce::String& name)
    {
        return std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { id, paramVersion }, name,
            makeGainRange(), 0.0f,
            juce::AudioParameterFloatAttributes().withLabel ("dB"));
    }

    std::unique_ptr<juce::AudioParameterFloat> makeQParam (const juce::String& id, const juce::String& name,
                                                             float defaultQ)
    {
        return std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { id, paramVersion }, name, makeQRange(), defaultQ);
    }

    std::unique_ptr<juce::AudioParameterChoice> makeTypeParam (const juce::String& id, const juce::String& name,
                                                                 const juce::StringArray& choices)
    {
        return std::make_unique<juce::AudioParameterChoice> (
            juce::ParameterID { id, paramVersion }, name, choices, 0);
    }

    std::unique_ptr<juce::AudioParameterChoice> makeSlopeParam (const juce::String& id)
    {
        return std::make_unique<juce::AudioParameterChoice> (
            juce::ParameterID { id, paramVersion }, "Slope",
            juce::StringArray { "12 dB/oct", "24 dB/oct", "36 dB/oct" }, 0);
    }

    // Bands 1 & 5: switchable Pass/Shelf type plus a slope selector.
    void addOuterBandParams (juce::AudioProcessorValueTreeState::ParameterLayout& layout,
                              const juce::String& typeId, const juce::String& freqId,
                              const juce::String& gainId, const juce::String& qId, const juce::String& slopeId,
                              const juce::StringArray& typeChoices,
                              float minHz, float maxHz, float defaultHz, float defaultQ)
    {
        layout.add (makeTypeParam (typeId, "Type", typeChoices));
        layout.add (makeFreqParam (freqId, "Freq", minHz, maxHz, defaultHz));
        layout.add (makeGainParam (gainId, "Gain"));
        layout.add (makeQParam (qId, "Q", defaultQ));
        layout.add (makeSlopeParam (slopeId));
    }

    // Bands 2-4: fixed Bell/Peak, no type or slope selector.
    void addPeakBandParams (juce::AudioProcessorValueTreeState::ParameterLayout& layout,
                             const juce::String& freqId, const juce::String& gainId, const juce::String& qId,
                             float defaultHz)
    {
        layout.add (makeFreqParam (freqId, "Freq", 20.0f, 20000.0f, defaultHz));
        layout.add (makeGainParam (gainId, "Gain"));
        layout.add (makeQParam (qId, "Q", 1.0f));
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    addOuterBandParams (layout, ParamIDs::band1Type, ParamIDs::band1FreqHz, ParamIDs::band1GainDb,
                         ParamIDs::band1Q, ParamIDs::band1Slope,
                         { "High-Pass", "Low Shelf" }, 20.0f, 500.0f, 80.0f, 0.707f);

    addPeakBandParams (layout, ParamIDs::band2FreqHz, ParamIDs::band2GainDb, ParamIDs::band2Q, 300.0f);
    addPeakBandParams (layout, ParamIDs::band3FreqHz, ParamIDs::band3GainDb, ParamIDs::band3Q, 1000.0f);
    addPeakBandParams (layout, ParamIDs::band4FreqHz, ParamIDs::band4GainDb, ParamIDs::band4Q, 3000.0f);

    addOuterBandParams (layout, ParamIDs::band5Type, ParamIDs::band5FreqHz, ParamIDs::band5GainDb,
                         ParamIDs::band5Q, ParamIDs::band5Slope,
                         { "Low-Pass", "High Shelf" }, 500.0f, 20000.0f, 8000.0f, 0.707f);

    layout.add (makeGainParam (ParamIDs::outputGainDb, "Output Trim"));
    layout.add (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { ParamIDs::bypass, paramVersion }, "Bypass", false));

    return layout;
}
