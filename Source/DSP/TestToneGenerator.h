#pragma once

#include <juce_dsp/juce_dsp.h>

// A sine test signal that replaces the incoming audio (device input or host
// track) when enabled, so the EQ and analyzer can be exercised without an
// external audio source.
class TestToneGenerator
{
public:
    void prepare (const juce::dsp::ProcessSpec& spec);
    void reset();
    void setParameters (bool enabled, float freqHz, float gainDb);
    void process (juce::AudioBuffer<float>& buffer);

private:
    juce::dsp::Oscillator<float> oscillator { [] (float x) { return std::sin (x); } };
    juce::SmoothedValue<float> gainSmoothed;

    bool isEnabled = false;
    float lastFreqHz = -1.0f;
};
