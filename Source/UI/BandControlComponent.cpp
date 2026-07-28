#include "BandControlComponent.h"

namespace
{
    void configureRotarySlider (juce::Slider& slider)
    {
        slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 20);
    }

    void populateFromChoiceParameter (juce::ComboBox& box, juce::AudioProcessorValueTreeState& apvts,
                                       const juce::String& paramId)
    {
        if (auto* param = dynamic_cast<juce::AudioParameterChoice*> (apvts.getParameter (paramId)))
            box.addItemList (param->choices, 1);
    }

    void configureCaptionLabel (juce::Label& label, const juce::String& text)
    {
        label.setText (text, juce::dontSendNotification);
        label.setJustificationType (juce::Justification::centred);
        label.setFont (juce::Font (juce::FontOptions (12.0f)));
    }
}

BandControlComponent::BandControlComponent (juce::AudioProcessorValueTreeState& apvts, const juce::String& bandLabel,
                                             const juce::String& freqParamId, const juce::String& gainParamId,
                                             const juce::String& qParamId,
                                             const juce::String& typeParamId, const juce::String& slopeParamId)
{
    bandNameLabel.setText (bandLabel, juce::dontSendNotification);
    bandNameLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (bandNameLabel);

    for (auto* slider : { &freqSlider, &gainSlider, &qSlider })
    {
        configureRotarySlider (*slider);
        addAndMakeVisible (slider);
    }

    configureCaptionLabel (freqLabel, "Frequency");
    configureCaptionLabel (gainLabel, "Gain");
    configureCaptionLabel (qLabel, "Q");

    for (auto* label : { &freqLabel, &gainLabel, &qLabel })
        addAndMakeVisible (label);

    freqAttachment = std::make_unique<SliderAttachment> (apvts, freqParamId, freqSlider);
    gainAttachment = std::make_unique<SliderAttachment> (apvts, gainParamId, gainSlider);
    qAttachment = std::make_unique<SliderAttachment> (apvts, qParamId, qSlider);

    hasTypeAndSlope = typeParamId.isNotEmpty() && slopeParamId.isNotEmpty();

    if (hasTypeAndSlope)
    {
        populateFromChoiceParameter (typeBox, apvts, typeParamId);
        populateFromChoiceParameter (slopeBox, apvts, slopeParamId);
        addAndMakeVisible (typeBox);
        addAndMakeVisible (slopeBox);

        typeAttachment = std::make_unique<ComboBoxAttachment> (apvts, typeParamId, typeBox);
        slopeAttachment = std::make_unique<ComboBoxAttachment> (apvts, slopeParamId, slopeBox);
    }
}

void BandControlComponent::resized()
{
    auto area = getLocalBounds();
    bandNameLabel.setBounds (area.removeFromTop (20));

    if (hasTypeAndSlope)
    {
        auto comboArea = area.removeFromTop (24);
        typeBox.setBounds (comboArea.removeFromLeft (comboArea.getWidth() / 2).reduced (2));
        slopeBox.setBounds (comboArea.reduced (2));
    }

    auto labelArea = area.removeFromBottom (16);

    const int knobWidth = area.getWidth() / 3;
    freqSlider.setBounds (area.removeFromLeft (knobWidth).reduced (4));
    gainSlider.setBounds (area.removeFromLeft (knobWidth).reduced (4));
    qSlider.setBounds (area.reduced (4));

    const int labelWidth = labelArea.getWidth() / 3;
    freqLabel.setBounds (labelArea.removeFromLeft (labelWidth));
    gainLabel.setBounds (labelArea.removeFromLeft (labelWidth));
    qLabel.setBounds (labelArea);
}
