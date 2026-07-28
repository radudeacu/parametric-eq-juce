#include "ModernLookAndFeel.h"
#include "Theme.h"
#include <cmath>

ModernLookAndFeel::ModernLookAndFeel()
{
    setColour (juce::Slider::textBoxTextColourId, Theme::textPrimary);
    setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    setColour (juce::Label::textColourId, Theme::textPrimary);
    setColour (juce::ComboBox::backgroundColourId, Theme::panelFill);
    setColour (juce::ComboBox::outlineColourId, Theme::panelBorder);
    setColour (juce::ComboBox::textColourId, Theme::textPrimary);
    setColour (juce::ComboBox::arrowColourId, Theme::accent);
    setColour (juce::PopupMenu::backgroundColourId, Theme::panelFill.withAlpha (0.95f));
    setColour (juce::PopupMenu::textColourId, Theme::textPrimary);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, Theme::accent.withAlpha (0.35f));
}

void ModernLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                                           float rotaryStartAngle, float rotaryEndAngle, juce::Slider&)
{
    auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (4.0f);
    const float radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f - 2.0f;
    const auto centre = bounds.getCentre();
    const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    constexpr float lineWidth = 3.0f;

    juce::Path track;
    track.addCentredArc (centre.x, centre.y, radius, radius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour (Theme::knobTrack);
    g.strokePath (track, juce::PathStrokeType (lineWidth, juce::PathStrokeType::curved,
                                                juce::PathStrokeType::rounded));

    juce::Path value;
    value.addCentredArc (centre.x, centre.y, radius, radius, 0.0f, rotaryStartAngle, angle, true);
    g.setColour (Theme::accent);
    g.strokePath (value, juce::PathStrokeType (lineWidth, juce::PathStrokeType::curved,
                                                juce::PathStrokeType::rounded));

    g.setColour (Theme::knobFace);
    g.fillEllipse (juce::Rectangle<float> (centre, centre).expanded (radius - lineWidth * 1.5f));

    const float indicatorLen = radius - lineWidth * 1.5f - 4.0f;
    juce::Point<float> tip (centre.x + indicatorLen * std::cos (angle - juce::MathConstants<float>::halfPi),
                             centre.y + indicatorLen * std::sin (angle - juce::MathConstants<float>::halfPi));
    g.setColour (Theme::textPrimary);
    g.drawLine ({ centre, tip }, 2.0f);
}

void ModernLookAndFeel::drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                                           float, float, juce::Slider::SliderStyle style, juce::Slider& slider)
{
    if (style != juce::Slider::LinearHorizontal)
    {
        LookAndFeel_V4::drawLinearSlider (g, x, y, width, height, sliderPos, 0.0f, 0.0f, style, slider);
        return;
    }

    auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat();
    constexpr float trackHeight = 6.0f;
    auto track = bounds.withSizeKeepingCentre (bounds.getWidth(), trackHeight);

    g.setColour (Theme::knobTrack);
    g.fillRoundedRectangle (track, trackHeight * 0.5f);

    auto filled = track.withRight (juce::jlimit (track.getX(), track.getRight(), sliderPos));
    g.setColour (Theme::accent);
    g.fillRoundedRectangle (filled, trackHeight * 0.5f);

    constexpr float thumbRadius = 7.0f;
    g.setColour (Theme::textPrimary);
    g.fillEllipse (sliderPos - thumbRadius, bounds.getCentreY() - thumbRadius, thumbRadius * 2.0f,
                    thumbRadius * 2.0f);
}

void ModernLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& button, bool, bool)
{
    auto bounds = button.getLocalBounds().toFloat();
    constexpr float pillWidth = 36.0f;
    constexpr float pillHeight = 18.0f;
    auto pillBounds = bounds.withSizeKeepingCentre (pillWidth, pillHeight).withX (bounds.getX());

    const bool isOn = button.getToggleState();
    g.setColour (isOn ? Theme::accent : Theme::knobTrack);
    g.fillRoundedRectangle (pillBounds, pillHeight * 0.5f);

    constexpr float thumbDiameter = pillHeight - 4.0f;
    const float thumbX = isOn ? pillBounds.getRight() - thumbDiameter - 2.0f : pillBounds.getX() + 2.0f;
    g.setColour (Theme::textPrimary);
    g.fillEllipse (thumbX, pillBounds.getCentreY() - thumbDiameter * 0.5f, thumbDiameter, thumbDiameter);

    g.setFont (juce::Font (juce::FontOptions (13.0f)));
    g.drawText (button.getButtonText(), bounds.withTrimmedLeft (pillWidth + 8.0f),
                juce::Justification::centredLeft);
}
