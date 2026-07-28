#include "TestToneGenerator.h"

void TestToneGenerator::prepare (const juce::dsp::ProcessSpec& spec)
{
    oscillator.prepare (spec);
    gainSmoothed.reset (spec.sampleRate, 0.02);
}

void TestToneGenerator::reset()
{
    oscillator.reset();
    isEnabled = false;
    lastFreqHz = -1.0f;
}

void TestToneGenerator::setParameters (bool enabled, float freqHz, float gainDb)
{
    isEnabled = enabled;
    gainSmoothed.setTargetValue (gainDb);

    if (std::abs (freqHz - lastFreqHz) > 1.0e-3f)
    {
        oscillator.setFrequency (freqHz);
        lastFreqHz = freqHz;
    }
}

void TestToneGenerator::process (juce::AudioBuffer<float>& buffer)
{
    if (! isEnabled)
        return;

    // Oscillator::process() adds to the existing buffer content rather than
    // overwriting it, so the buffer must be silenced first for the tone to
    // truly replace whatever audio was already there.
    buffer.clear();

    const float startGain = juce::Decibels::decibelsToGain (gainSmoothed.getCurrentValue());
    const float endGain = juce::Decibels::decibelsToGain (gainSmoothed.skip (buffer.getNumSamples()));

    juce::dsp::AudioBlock<float> block (buffer);
    juce::dsp::ProcessContextReplacing<float> context (block);
    oscillator.process (context);

    buffer.applyGainRamp (0, buffer.getNumSamples(), startGain, endGain);
}
