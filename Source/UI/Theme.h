#pragma once

#include <juce_graphics/juce_graphics.h>

// Central colour/style palette for the "modern glass" theme. One shared
// header, included wherever a themed constant is needed, following the
// same pattern as Analyzer/AnalyzerMapping.h.
namespace Theme
{
    // PluginEditor background gradient
    inline const juce::Colour backgroundTop { 0xff1a1c22 };
    inline const juce::Colour backgroundBottom { 0xff0d0e12 };

    // Glass card panel chrome (Band/Global/TestTone components)
    inline const juce::Colour panelFill { juce::Colour (0xff2b2f3a).withAlpha (0.45f) };
    inline const juce::Colour panelBorder { juce::Colours::white.withAlpha (0.12f) };
    inline const juce::Colour panelSheen { juce::Colours::white.withAlpha (0.10f) };
    inline const juce::Colour panelShadow { juce::Colours::black.withAlpha (0.5f) };
    constexpr float panelCornerRadius = 10.0f;
    constexpr int panelShadowRadius = 12;

    // Controls
    inline const juce::Colour accent { 0xffffa500 }; // matches the analyzer's existing curve colour
    inline const juce::Colour knobTrack { juce::Colours::white.withAlpha (0.14f) };
    inline const juce::Colour knobFace { juce::Colour (0xff22252c) };
    inline const juce::Colour textPrimary { juce::Colours::white.withAlpha (0.92f) };
    inline const juce::Colour textSecondary { juce::Colours::white.withAlpha (0.6f) };

    // Analyzer aliases (kept for future consistency; Analyzer/*.cpp keep
    // their own literals in this pass, except one optional cohesion swap).
    inline const juce::Colour analyzerBackground { 0xff0a0b0d };
}
