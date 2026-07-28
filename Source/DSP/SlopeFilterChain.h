#pragma once

#include <juce_dsp/juce_dsp.h>
#include <array>

enum class SlopeFilterKind
{
    HighPass,
    LowPass,
    LowShelf,
    HighShelf
};

// A cascade of up to 3 biquad stages giving a selectable 12/24/36 dB/octave
// response for either a Pass filter (Butterworth, JUCE-designed per-stage
// coefficients) or a Shelf filter (identical shelf stages with gain split
// evenly across stages so total plateau gain stays correct).
class SlopeFilterChain
{
public:
    static constexpr int maxStages = 3;

    void prepare (const juce::dsp::ProcessSpec& spec);
    void reset();
    void update (SlopeFilterKind kind, double sampleRate, float freqHz, float q, float gainDb, int slopeDbPerOctave);
    void process (juce::dsp::ProcessContextReplacing<float>& context);

private:
    using Filter = juce::dsp::IIR::Filter<float>;
    using Coefficients = juce::dsp::IIR::Coefficients<float>;

    void updatePassFilter (SlopeFilterKind kind, double sampleRate, float freqHz, int slopeDbPerOctave);
    void updateShelfFilter (SlopeFilterKind kind, double sampleRate, float freqHz, float q, float gainDb,
                             int slopeDbPerOctave);

    std::array<Filter, (size_t) maxStages> stages;
    int numActiveStages = 1;
};
