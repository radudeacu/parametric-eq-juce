#include "AnalyzerPanelComponent.h"
#include "../PluginProcessor.h"

AnalyzerPanelComponent::AnalyzerPanelComponent (ParametricEQAudioProcessor& processor)
    : traceComponent (processor.analyzerFifo), overlayComponent (processor)
{
    addAndMakeVisible (traceComponent);
    addAndMakeVisible (overlayComponent);
}

void AnalyzerPanelComponent::resized()
{
    traceComponent.setBounds (getLocalBounds());
    overlayComponent.setBounds (getLocalBounds());
}
