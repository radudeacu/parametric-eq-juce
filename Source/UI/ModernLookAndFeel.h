#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

// Modern glass-themed widget chrome: arc-style rotary knobs, pill-style
// linear sliders and toggle switches. ComboBox/PopupMenu are left to the
// stock LookAndFeel_V4 painters, themed purely via colour IDs in the
// constructor (a bespoke combobox painter isn't justified for 2 instances).
class ModernLookAndFeel : public juce::LookAndFeel_V4
{
public:
    ModernLookAndFeel();

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                            float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider) override;

    void drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                            float minSliderPos, float maxSliderPos, juce::Slider::SliderStyle style,
                            juce::Slider& slider) override;

    void drawToggleButton (juce::Graphics& g, juce::ToggleButton& button, bool shouldDrawButtonAsHighlighted,
                            bool shouldDrawButtonAsDown) override;
};
