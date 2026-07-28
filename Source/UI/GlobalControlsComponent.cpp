#include "GlobalControlsComponent.h"
#include "GlassPanel.h"
#include "../Parameters/ParameterIDs.h"

GlobalControlsComponent::GlobalControlsComponent (juce::AudioProcessorValueTreeState& apvts)
{
    trimLabel.setText ("Output Trim", juce::dontSendNotification);
    trimLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (trimLabel);

    trimSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    trimSlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 60, 20);
    addAndMakeVisible (trimSlider);

    addAndMakeVisible (bypassButton);

    trimAttachment = std::make_unique<SliderAttachment> (apvts, ParamIDs::outputGainDb, trimSlider);
    bypassAttachment = std::make_unique<ButtonAttachment> (apvts, ParamIDs::bypass, bypassButton);
}

void GlobalControlsComponent::paint (juce::Graphics& g)
{
    GlassPanel::draw (g, getLocalBounds().toFloat());
}

void GlobalControlsComponent::resized()
{
    auto area = getLocalBounds();
    bypassButton.setBounds (area.removeFromLeft (100).reduced (4));
    trimLabel.setBounds (area.removeFromLeft (100).reduced (4));
    trimSlider.setBounds (area.reduced (4));
}
