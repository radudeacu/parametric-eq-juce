#include "LowBand.h"

namespace
{
    bool nearlyEqual (float a, float b) { return std::abs (a - b) < 1.0e-6f; }
}

void LowBand::prepare (const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    filterChain.prepare (spec);

    freqSmoothed.reset (spec.sampleRate, 0.02);
    gainSmoothed.reset (spec.sampleRate, 0.02);
    qSmoothed.reset (spec.sampleRate, 0.02);
}

void LowBand::reset()
{
    filterChain.reset();
    firstUpdate = true;
}

void LowBand::setParameters (LowBandType type, float freqHz, float gainDb, float q, int slopeDbPerOctave,
                              int numSamples)
{
    if (firstUpdate)
    {
        freqSmoothed.setCurrentAndTargetValue (freqHz);
        gainSmoothed.setCurrentAndTargetValue (gainDb);
        qSmoothed.setCurrentAndTargetValue (q);
    }
    else
    {
        freqSmoothed.setTargetValue (freqHz);
        gainSmoothed.setTargetValue (gainDb);
        qSmoothed.setTargetValue (q);
    }

    const int skipSamples = juce::jmax (0, numSamples - 1);
    const float effectiveFreq = freqSmoothed.skip (skipSamples);
    const float effectiveGain = gainSmoothed.skip (skipSamples);
    const float effectiveQ = qSmoothed.skip (skipSamples);

    const bool changed = firstUpdate
        || type != lastType
        || ! nearlyEqual (effectiveFreq, lastFreqHz)
        || ! nearlyEqual (effectiveGain, lastGainDb)
        || ! nearlyEqual (effectiveQ, lastQ)
        || slopeDbPerOctave != lastSlope;

    if (! changed)
        return;

    const auto kind = (type == LowBandType::HighPass) ? SlopeFilterKind::HighPass : SlopeFilterKind::LowShelf;
    filterChain.update (kind, sampleRate, effectiveFreq, effectiveQ, effectiveGain, slopeDbPerOctave);

    lastType = type;
    lastFreqHz = effectiveFreq;
    lastGainDb = effectiveGain;
    lastQ = effectiveQ;
    lastSlope = slopeDbPerOctave;
    firstUpdate = false;
}

void LowBand::process (juce::dsp::ProcessContextReplacing<float>& context)
{
    filterChain.process (context);
}
