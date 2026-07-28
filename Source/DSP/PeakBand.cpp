#include "PeakBand.h"

namespace
{
    bool nearlyEqual (float a, float b) { return std::abs (a - b) < 1.0e-6f; }
}

void PeakBand::prepare (const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    filter.prepare (spec);

    freqSmoothed.reset (spec.sampleRate, 0.02);
    gainSmoothed.reset (spec.sampleRate, 0.02);
    qSmoothed.reset (spec.sampleRate, 0.02);
}

void PeakBand::reset()
{
    filter.reset();
    firstUpdate = true;
}

void PeakBand::setParameters (float freqHz, float gainDb, float q, int numSamples)
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
        || ! nearlyEqual (effectiveFreq, lastFreqHz)
        || ! nearlyEqual (effectiveGain, lastGainDb)
        || ! nearlyEqual (effectiveQ, lastQ);

    if (! changed)
        return;

    filter.coefficients = Coefficients::makePeakFilter (sampleRate, effectiveFreq, effectiveQ,
                                                          juce::Decibels::decibelsToGain (effectiveGain));

    lastFreqHz = effectiveFreq;
    lastGainDb = effectiveGain;
    lastQ = effectiveQ;
    firstUpdate = false;
}

void PeakBand::process (juce::dsp::ProcessContextReplacing<float>& context)
{
    filter.process (context);
}
