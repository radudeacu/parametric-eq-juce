#pragma once

#include <juce_graphics/juce_graphics.h>
#include "Theme.h"

// Shared "frosted glass card" background renderer used by every control
// panel's paint() override. One shared header, mirrors AnalyzerMapping.h.
namespace GlassPanel
{
    inline void draw (juce::Graphics& g, juce::Rectangle<float> bounds)
    {
        auto cardBounds = bounds.reduced (1.0f);
        juce::Path shape;
        shape.addRoundedRectangle (cardBounds, Theme::panelCornerRadius);

        juce::DropShadow shadow (Theme::panelShadow, Theme::panelShadowRadius, { 0, 2 });
        shadow.drawForPath (g, shape);

        g.setColour (Theme::panelFill);
        g.fillPath (shape);

        {
            juce::Graphics::ScopedSaveState saved (g);
            g.reduceClipRegion (shape);

            auto sheenBounds = cardBounds.withHeight (cardBounds.getHeight() * 0.4f);
            juce::ColourGradient sheen (Theme::panelSheen, sheenBounds.getX(), sheenBounds.getY(),
                                        juce::Colours::transparentWhite, sheenBounds.getX(),
                                        sheenBounds.getBottom(), false);
            g.setGradientFill (sheen);
            g.fillRect (sheenBounds);
        }

        g.setColour (Theme::panelBorder);
        g.strokePath (shape, juce::PathStrokeType (1.0f));
    }
}
