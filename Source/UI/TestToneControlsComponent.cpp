#include "TestToneControlsComponent.h"
#include "GlassPanel.h"
#include "../Parameters/ParameterIDs.h"

TestToneControlsComponent::TestToneControlsComponent (juce::AudioProcessorValueTreeState& apvts)
{
    addAndMakeVisible (enableButton);

    for (auto* label : { &freqLabel, &levelLabel })
    {
        label->setJustificationType (juce::Justification::centredLeft);
        addAndMakeVisible (label);
    }

    for (auto* slider : { &freqSlider, &levelSlider })
    {
        slider->setSliderStyle (juce::Slider::LinearHorizontal);
        slider->setTextBoxStyle (juce::Slider::TextBoxRight, false, 60, 20);
        addAndMakeVisible (slider);
    }

    enableAttachment = std::make_unique<ButtonAttachment> (apvts, ParamIDs::testToneEnabled, enableButton);
    freqAttachment = std::make_unique<SliderAttachment> (apvts, ParamIDs::testToneFreqHz, freqSlider);
    levelAttachment = std::make_unique<SliderAttachment> (apvts, ParamIDs::testToneGainDb, levelSlider);
}

void TestToneControlsComponent::paint (juce::Graphics& g)
{
    GlassPanel::draw (g, getLocalBounds().toFloat());
}

void TestToneControlsComponent::resized()
{
    auto area = getLocalBounds();
    enableButton.setBounds (area.removeFromLeft (110).reduced (4));

    auto freqArea = area.removeFromLeft (area.getWidth() / 2);
    freqLabel.setBounds (freqArea.removeFromLeft (40));
    freqSlider.setBounds (freqArea.reduced (4));

    levelLabel.setBounds (area.removeFromLeft (40));
    levelSlider.setBounds (area.reduced (4));
}
