#pragma once

#include "SlopeFilterChain.h"

enum class HighBandType
{
    LowPass,
    HighShelf
};

// Band 5: switchable Low-Pass / High Shelf with a 12/24/36 dB/oct slope.
class HighBand
{
public:
    void prepare (const juce::dsp::ProcessSpec& spec);
    void reset();
    void setParameters (HighBandType type, float freqHz, float gainDb, float q, int slopeDbPerOctave,
                         int numSamples);
    void process (juce::dsp::ProcessContextReplacing<float>& context);

    // Pure, stateless: safe to call from any thread.
    static float getMagnitudeForFrequency (HighBandType type, double queryFreqHz, double sampleRate, float freqHz,
                                            float gainDb, float q, int slopeDbPerOctave);

private:
    SlopeFilterChain filterChain;
    double sampleRate = 44100.0;

    juce::SmoothedValue<float> freqSmoothed, gainSmoothed, qSmoothed;

    HighBandType lastType = HighBandType::LowPass;
    float lastFreqHz = -1.0f;
    float lastGainDb = 0.0f;
    float lastQ = -1.0f;
    int lastSlope = -1;
    bool firstUpdate = true;
};
