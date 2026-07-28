#pragma once

#include <juce_dsp/juce_dsp.h>

// Bands 2-4: fixed Bell/Peak filter, single biquad, no type or slope selector.
class PeakBand
{
public:
    using Coefficients = juce::dsp::IIR::Coefficients<float>;

    void prepare (const juce::dsp::ProcessSpec& spec);
    void reset();
    void setParameters (float freqHz, float gainDb, float q, int numSamples);
    void process (juce::dsp::ProcessContextReplacing<float>& context);

    // Pure, stateless: safe to call from any thread.
    static Coefficients::Ptr buildCoefficients (double sampleRate, float freqHz, float gainDb, float q);
    static float getMagnitudeForFrequency (double queryFreqHz, double sampleRate, float freqHz, float gainDb,
                                            float q);

private:
    using Filter = juce::dsp::IIR::Filter<float>;

    Filter filter;
    double sampleRate = 44100.0;

    juce::SmoothedValue<float> freqSmoothed, gainSmoothed, qSmoothed;

    float lastFreqHz = -1.0f;
    float lastGainDb = 0.0f;
    float lastQ = -1.0f;
    bool firstUpdate = true;
};
