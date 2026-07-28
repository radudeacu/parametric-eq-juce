#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class ParametricEQAudioProcessor;

// Frequency/dB axis gridlines, the composite EQ curve (all 5 bands combined),
// and a marker dot per band, layered on top of the raw spectrum trace.
// Purely a UI-thread visual: never reads live audio-thread DSP state,
// only APVTS parameter values and ParametricEQAudioProcessor's pure
// magnitude-query methods.
class EqCurveOverlayComponent : public juce::Component, private juce::Timer
{
public:
    explicit EqCurveOverlayComponent (ParametricEQAudioProcessor& processorToUse);

    void paint (juce::Graphics& g) override;

private:
    void timerCallback() override;

    void drawFrequencyGrid (juce::Graphics& g) const;
    void drawGainGrid (juce::Graphics& g) const;
    juce::Path buildCompositeCurvePath() const;
    void drawBandMarkers (juce::Graphics& g) const;

    static constexpr int numCurvePoints = 300;

    ParametricEQAudioProcessor& processor;
};
