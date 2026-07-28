#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

// Enable toggle + frequency/level sliders for the built-in sine test tone,
// letting the EQ and analyzer be exercised without an external audio source.
class TestToneControlsComponent : public juce::Component
{
public:
    explicit TestToneControlsComponent (juce::AudioProcessorValueTreeState& apvts);

    void resized() override;

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    juce::ToggleButton enableButton { "Test Tone" };
    juce::Label freqLabel { {}, "Freq" }, levelLabel { {}, "Level" };
    juce::Slider freqSlider, levelSlider;

    std::unique_ptr<ButtonAttachment> enableAttachment;
    std::unique_ptr<SliderAttachment> freqAttachment, levelAttachment;
};
