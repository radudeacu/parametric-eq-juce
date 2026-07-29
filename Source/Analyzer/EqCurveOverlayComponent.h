#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class ParametricEQAudioProcessor;

// Frequency/dB axis gridlines, the composite EQ curve (all 5 bands combined),
// and a draggable marker dot per band, layered on top of the raw spectrum
// trace. Dragging writes to APVTS parameters via the standard host-automation
// -safe gesture path (same as SliderAttachment uses internally) so the
// rotary knobs elsewhere track a drag live and host automation still works.
// Otherwise purely a UI-thread visual: never reads live audio-thread DSP
// state, only APVTS parameter values and ParametricEQAudioProcessor's pure
// magnitude-query methods.
class EqCurveOverlayComponent : public juce::Component, private juce::Timer
{
public:
    explicit EqCurveOverlayComponent (ParametricEQAudioProcessor& processorToUse);

    void paint (juce::Graphics& g) override;

    void mouseDown (const juce::MouseEvent& event) override;
    void mouseDrag (const juce::MouseEvent& event) override;
    void mouseUp (const juce::MouseEvent& event) override;
    void mouseMove (const juce::MouseEvent& event) override;
    void mouseExit (const juce::MouseEvent& event) override;

private:
    void timerCallback() override;

    void drawFrequencyGrid (juce::Graphics& g) const;
    void drawGainGrid (juce::Graphics& g) const;
    juce::Path buildCompositeCurvePath() const;
    void drawBandMarkers (juce::Graphics& g) const;

    juce::Point<float> bandMarkerPosition (int bandIndex) const;
    int findNearestBandMarker (juce::Point<float> position) const;
    bool bandGainIsDraggable (int bandIndex) const;
    void updateParameterFromDrag (int bandIndex, juce::Point<float> position);

    static constexpr int numCurvePoints = 300;
    static constexpr float hitRadiusPixels = 12.0f;

    ParametricEQAudioProcessor& processor;

    int draggedBandIndex = -1;
    int hoveredBandIndex = -1;
};
