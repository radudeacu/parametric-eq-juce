#include "SlopeFilterChain.h"

void SlopeFilterChain::prepare (const juce::dsp::ProcessSpec& spec)
{
    for (auto& stage : stages)
        stage.prepare (spec);
}

void SlopeFilterChain::reset()
{
    for (auto& stage : stages)
        stage.reset();
}

void SlopeFilterChain::update (SlopeFilterKind kind, double sampleRate, float freqHz, float q, float gainDb,
                                int slopeDbPerOctave)
{
    const auto newCoeffs = buildStageCoefficients (kind, sampleRate, freqHz, q, gainDb, slopeDbPerOctave);
    const int newNumStages = juce::jmin (newCoeffs.size(), maxStages);

    for (int i = 0; i < newNumStages; ++i)
    {
        stages[(size_t) i].coefficients = newCoeffs[i];

        if (i >= numActiveStages)
            stages[(size_t) i].reset();
    }

    numActiveStages = newNumStages;
}

void SlopeFilterChain::process (juce::dsp::ProcessContextReplacing<float>& context)
{
    for (int i = 0; i < numActiveStages; ++i)
        stages[(size_t) i].process (context);
}

juce::ReferenceCountedArray<SlopeFilterChain::Coefficients> SlopeFilterChain::buildStageCoefficients (
    SlopeFilterKind kind, double sampleRate, float freqHz, float q, float gainDb, int slopeDbPerOctave)
{
    if (kind == SlopeFilterKind::HighPass || kind == SlopeFilterKind::LowPass)
        return buildPassFilterCoefficients (kind, sampleRate, freqHz, slopeDbPerOctave);

    return buildShelfFilterCoefficients (kind, sampleRate, freqHz, q, gainDb, slopeDbPerOctave);
}

float SlopeFilterChain::getMagnitudeForFrequency (SlopeFilterKind kind, double queryFreqHz, double sampleRate,
                                                   float freqHz, float q, float gainDb, int slopeDbPerOctave)
{
    const auto coeffs = buildStageCoefficients (kind, sampleRate, freqHz, q, gainDb, slopeDbPerOctave);

    float magnitude = 1.0f;
    for (auto* c : coeffs)
        magnitude *= (float) c->getMagnitudeForFrequency (queryFreqHz, sampleRate);

    return magnitude;
}

juce::ReferenceCountedArray<SlopeFilterChain::Coefficients> SlopeFilterChain::buildPassFilterCoefficients (
    SlopeFilterKind kind, double sampleRate, float freqHz, int slopeDbPerOctave)
{
    const int order = juce::jlimit (2, maxStages * 2, (slopeDbPerOctave / 12) * 2);

    return (kind == SlopeFilterKind::HighPass)
        ? juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod (freqHz, sampleRate, order)
        : juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod (freqHz, sampleRate, order);
}

juce::ReferenceCountedArray<SlopeFilterChain::Coefficients> SlopeFilterChain::buildShelfFilterCoefficients (
    SlopeFilterKind kind, double sampleRate, float freqHz, float q, float gainDb, int slopeDbPerOctave)
{
    const int numStages = juce::jlimit (1, maxStages, slopeDbPerOctave / 12);
    const float perStageGainFactor = juce::Decibels::decibelsToGain (gainDb / (float) numStages);

    juce::ReferenceCountedArray<Coefficients> result;

    for (int i = 0; i < numStages; ++i)
        result.add (kind == SlopeFilterKind::LowShelf
            ? Coefficients::makeLowShelf (sampleRate, freqHz, q, perStageGainFactor)
            : Coefficients::makeHighShelf (sampleRate, freqHz, q, perStageGainFactor));

    return result;
}
