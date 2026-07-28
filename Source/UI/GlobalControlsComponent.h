#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

// Output trim slider + bypass toggle, shared across the whole plugin.
class GlobalControlsComponent : public juce::Component
{
public:
    explicit GlobalControlsComponent (juce::AudioProcessorValueTreeState& apvts);

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    juce::Label trimLabel;
    juce::Slider trimSlider;
    juce::ToggleButton bypassButton { "Bypass" };

    std::unique_ptr<SliderAttachment> trimAttachment;
    std::unique_ptr<ButtonAttachment> bypassAttachment;
};
