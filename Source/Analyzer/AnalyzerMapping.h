#pragma once

#include <juce_dsp/juce_dsp.h>
#include <cmath>

// Shared frequency/gain <-> pixel mapping used by the spectrum trace, the
// axis gridlines, and the composite EQ curve so all three can never
// visually disagree about where a given Hz or dB value sits on screen.
namespace AnalyzerMapping
{
    constexpr float minFrequencyHz = 20.0f;
    constexpr float maxFrequencyHz = 20000.0f;
    constexpr float minGainDb = -24.0f;
    constexpr float maxGainDb = 24.0f;

    inline float frequencyToProportion (float freqHz) noexcept
    {
        const float clamped = juce::jlimit (minFrequencyHz, maxFrequencyHz, freqHz);
        return std::log10 (clamped / minFrequencyHz) / std::log10 (maxFrequencyHz / minFrequencyHz);
    }

    inline float proportionToFrequency (float proportion) noexcept
    {
        return minFrequencyHz * std::pow (maxFrequencyHz / minFrequencyHz, juce::jlimit (0.0f, 1.0f, proportion));
    }

    inline float frequencyToX (float freqHz, float width) noexcept
    {
        return frequencyToProportion (freqHz) * width;
    }

    inline float xToFrequency (float x, float width) noexcept
    {
        return proportionToFrequency (width > 0.0f ? x / width : 0.0f);
    }

    // maxGainDb maps to y=0 (top edge); minGainDb maps to y=height (bottom edge).
    inline float dbToY (float db, float height) noexcept
    {
        return juce::jmap (juce::jlimit (minGainDb, maxGainDb, db), maxGainDb, minGainDb, 0.0f, height);
    }
}
