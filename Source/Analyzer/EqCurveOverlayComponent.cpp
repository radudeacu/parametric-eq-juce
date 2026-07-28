#include "EqCurveOverlayComponent.h"
#include "AnalyzerMapping.h"
#include "../PluginProcessor.h"
#include "../Parameters/ParameterIDs.h"
#include <array>

namespace
{
    constexpr std::array<float, 10> frequencyTicks { 20.0f, 50.0f, 100.0f, 200.0f, 500.0f,
                                                       1000.0f, 2000.0f, 5000.0f, 10000.0f, 20000.0f };
    constexpr std::array<float, 5> gainTicks { -24.0f, -12.0f, 0.0f, 12.0f, 24.0f };

    juce::String formatFrequencyLabel (float freqHz)
    {
        if (freqHz >= 1000.0f)
            return juce::String (juce::roundToInt (freqHz / 1000.0f)) + "k";

        return juce::String (juce::roundToInt (freqHz));
    }
}

EqCurveOverlayComponent::EqCurveOverlayComponent (ParametricEQAudioProcessor& processorToUse)
    : processor (processorToUse)
{
    setInterceptsMouseClicks (false, false);
    startTimerHz (30);
}

void EqCurveOverlayComponent::paint (juce::Graphics& g)
{
    drawFrequencyGrid (g);
    drawGainGrid (g);

    const auto curvePath = buildCompositeCurvePath();

    auto fillPath = curvePath;
    fillPath.lineTo ((float) getWidth(), (float) getHeight());
    fillPath.lineTo (0.0f, (float) getHeight());
    fillPath.closeSubPath();

    g.setColour (juce::Colours::orange.withAlpha (0.15f));
    g.fillPath (fillPath);

    g.setColour (juce::Colours::orange);
    g.strokePath (curvePath, juce::PathStrokeType (2.0f));

    drawBandMarkers (g);
}

void EqCurveOverlayComponent::timerCallback()
{
    repaint();
}

void EqCurveOverlayComponent::drawFrequencyGrid (juce::Graphics& g) const
{
    const float width = (float) getWidth();
    const float height = (float) getHeight();

    g.setFont (juce::Font (juce::FontOptions (11.0f)));

    for (float freqHz : frequencyTicks)
    {
        const float x = AnalyzerMapping::frequencyToX (freqHz, width);

        g.setColour (juce::Colours::white.withAlpha (0.15f));
        g.drawVerticalLine (juce::roundToInt (x), 0.0f, height);

        g.setColour (juce::Colours::white.withAlpha (0.6f));
        g.drawText (formatFrequencyLabel (freqHz), juce::roundToInt (x) + 2, juce::roundToInt (height) - 16,
                    40, 14, juce::Justification::centredLeft);
    }
}

void EqCurveOverlayComponent::drawGainGrid (juce::Graphics& g) const
{
    const float width = (float) getWidth();
    const float height = (float) getHeight();

    g.setFont (juce::Font (juce::FontOptions (11.0f)));

    for (float gainDb : gainTicks)
    {
        const float y = AnalyzerMapping::dbToY (gainDb, height);

        g.setColour (juce::Colours::white.withAlpha (gainDb == 0.0f ? 0.3f : 0.1f));
        g.drawHorizontalLine (juce::roundToInt (y), 0.0f, width);

        g.setColour (juce::Colours::white.withAlpha (0.6f));
        g.drawText ((gainDb > 0.0f ? "+" : "") + juce::String (juce::roundToInt (gainDb)),
                    juce::roundToInt (width) - 34, juce::roundToInt (y) - 7, 30, 14,
                    juce::Justification::centredRight);
    }
}

juce::Path EqCurveOverlayComponent::buildCompositeCurvePath() const
{
    const float width = (float) getWidth();
    const float height = (float) getHeight();

    std::array<double, numCurvePoints> frequencies {};
    std::array<float, numCurvePoints> magnitudesDb {};

    for (int i = 0; i < numCurvePoints; ++i)
    {
        const float proportion = (float) i / (float) (numCurvePoints - 1);
        frequencies[(size_t) i] = AnalyzerMapping::proportionToFrequency (proportion);
    }

    processor.getMagnitudeResponseDb (frequencies.data(), magnitudesDb.data(), numCurvePoints);

    juce::Path path;

    for (int i = 0; i < numCurvePoints; ++i)
    {
        const float x = juce::jmap ((float) i, 0.0f, (float) (numCurvePoints - 1), 0.0f, width);
        const float y = AnalyzerMapping::dbToY (magnitudesDb[(size_t) i], height);

        if (i == 0)
            path.startNewSubPath (x, y);
        else
            path.lineTo (x, y);
    }

    return path;
}

void EqCurveOverlayComponent::drawBandMarkers (juce::Graphics& g) const
{
    const float width = (float) getWidth();
    const float height = (float) getHeight();

    const std::array<const char*, 5> freqParamIds { ParamIDs::band1FreqHz, ParamIDs::band2FreqHz,
                                                      ParamIDs::band3FreqHz, ParamIDs::band4FreqHz,
                                                      ParamIDs::band5FreqHz };

    for (auto* freqParamId : freqParamIds)
    {
        const float freqHz = processor.apvts.getRawParameterValue (freqParamId)->load();
        const float magnitudeDb = processor.getMagnitudeForFrequencyDb (freqHz);

        const float x = AnalyzerMapping::frequencyToX (freqHz, width);
        const float y = AnalyzerMapping::dbToY (magnitudeDb, height);

        g.setColour (juce::Colours::orange);
        g.fillEllipse (x - 4.0f, y - 4.0f, 8.0f, 8.0f);

        g.setColour (juce::Colours::black.withAlpha (0.6f));
        g.drawEllipse (x - 4.0f, y - 4.0f, 8.0f, 8.0f, 1.0f);
    }
}
