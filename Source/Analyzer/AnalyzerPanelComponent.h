#pragma once

#include "SpectrumAnalyzerComponent.h"
#include "EqCurveOverlayComponent.h"

class ParametricEQAudioProcessor;

// Thin z-order container: the raw spectrum trace underneath, the EQ curve
// overlay (axes, composite curve, band markers) on top, both at the same
// bounds. Kept separate rather than merged since they have different data
// sources (audio-thread-fed FFT ring buffer vs. UI-thread parameter reads).
class AnalyzerPanelComponent : public juce::Component
{
public:
    explicit AnalyzerPanelComponent (ParametricEQAudioProcessor& processor);

    void resized() override;

private:
    SpectrumAnalyzerComponent traceComponent;
    EqCurveOverlayComponent overlayComponent;
};
