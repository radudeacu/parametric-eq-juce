#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

// A reusable cluster of controls for one EQ band: Freq/Gain/Q rotary sliders,
// plus optional Type and Slope combo boxes for the two switchable outer bands.
class BandControlComponent : public juce::Component
{
public:
    BandControlComponent (juce::AudioProcessorValueTreeState& apvts, const juce::String& bandLabel,
                           const juce::String& freqParamId, const juce::String& gainParamId,
                           const juce::String& qParamId,
                           const juce::String& typeParamId = {}, const juce::String& slopeParamId = {});

    void resized() override;

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    juce::Label bandNameLabel;
    juce::Slider freqSlider, gainSlider, qSlider;
    juce::Label freqLabel, gainLabel, qLabel;
    juce::ComboBox typeBox, slopeBox;

    std::unique_ptr<SliderAttachment> freqAttachment, gainAttachment, qAttachment;
    std::unique_ptr<ComboBoxAttachment> typeAttachment, slopeAttachment;

    bool hasTypeAndSlope = false;
};
