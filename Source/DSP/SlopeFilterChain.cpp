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
    if (kind == SlopeFilterKind::HighPass || kind == SlopeFilterKind::LowPass)
        updatePassFilter (kind, sampleRate, freqHz, slopeDbPerOctave);
    else
        updateShelfFilter (kind, sampleRate, freqHz, q, gainDb, slopeDbPerOctave);
}

void SlopeFilterChain::process (juce::dsp::ProcessContextReplacing<float>& context)
{
    for (int i = 0; i < numActiveStages; ++i)
        stages[(size_t) i].process (context);
}

void SlopeFilterChain::updatePassFilter (SlopeFilterKind kind, double sampleRate, float freqHz, int slopeDbPerOctave)
{
    const int order = juce::jlimit (2, maxStages * 2, (slopeDbPerOctave / 12) * 2);

    const auto coeffsArray = (kind == SlopeFilterKind::HighPass)
        ? juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod (freqHz, sampleRate, order)
        : juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod (freqHz, sampleRate, order);

    const int newNumStages = juce::jmin ((int) coeffsArray.size(), maxStages);

    for (int i = 0; i < newNumStages; ++i)
    {
        stages[(size_t) i].coefficients = coeffsArray[(size_t) i];

        if (i >= numActiveStages)
            stages[(size_t) i].reset();
    }

    numActiveStages = newNumStages;
}

void SlopeFilterChain::updateShelfFilter (SlopeFilterKind kind, double sampleRate, float freqHz, float q,
                                           float gainDb, int slopeDbPerOctave)
{
    const int newNumStages = juce::jlimit (1, maxStages, slopeDbPerOctave / 12);
    const float perStageGainFactor = juce::Decibels::decibelsToGain (gainDb / (float) newNumStages);

    for (int i = 0; i < newNumStages; ++i)
    {
        stages[(size_t) i].coefficients = (kind == SlopeFilterKind::LowShelf)
            ? Coefficients::makeLowShelf (sampleRate, freqHz, q, perStageGainFactor)
            : Coefficients::makeHighShelf (sampleRate, freqHz, q, perStageGainFactor);

        if (i >= numActiveStages)
            stages[(size_t) i].reset();
    }

    numActiveStages = newNumStages;
}
