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

    using Coefficients = juce::dsp::IIR::Coefficients<float>;

    void prepare (const juce::dsp::ProcessSpec& spec);
    void reset();
    void update (SlopeFilterKind kind, double sampleRate, float freqHz, float q, float gainDb, int slopeDbPerOctave);
    void process (juce::dsp::ProcessContextReplacing<float>& context);

    // Pure, stateless: builds the same per-stage coefficients update() would
    // assign, without touching any live Filter state. Safe to call from any
    // thread (each call produces independent, freshly-allocated objects).
    static juce::ReferenceCountedArray<Coefficients> buildStageCoefficients (
        SlopeFilterKind kind, double sampleRate, float freqHz, float q, float gainDb, int slopeDbPerOctave);

    static float getMagnitudeForFrequency (SlopeFilterKind kind, double queryFreqHz, double sampleRate,
                                            float freqHz, float q, float gainDb, int slopeDbPerOctave);

private:
    using Filter = juce::dsp::IIR::Filter<float>;

    static juce::ReferenceCountedArray<Coefficients> buildPassFilterCoefficients (
        SlopeFilterKind kind, double sampleRate, float freqHz, int slopeDbPerOctave);
    static juce::ReferenceCountedArray<Coefficients> buildShelfFilterCoefficients (
        SlopeFilterKind kind, double sampleRate, float freqHz, float q, float gainDb, int slopeDbPerOctave);

    std::array<Filter, (size_t) maxStages> stages;
    int numActiveStages = 1;
};
