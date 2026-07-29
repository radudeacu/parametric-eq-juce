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

    struct BandMarkerInfo
    {
        const char* freqParamId;
        const char* gainParamId;
        const char* typeParamId; // nullptr for bands 2-4 (always Bell/Peak, gain always draggable)
    };

    constexpr std::array<BandMarkerInfo, 5> bandMarkers { {
        { ParamIDs::band1FreqHz, ParamIDs::band1GainDb, ParamIDs::band1Type },
        { ParamIDs::band2FreqHz, ParamIDs::band2GainDb, nullptr },
        { ParamIDs::band3FreqHz, ParamIDs::band3GainDb, nullptr },
        { ParamIDs::band4FreqHz, ParamIDs::band4GainDb, nullptr },
        { ParamIDs::band5FreqHz, ParamIDs::band5GainDb, ParamIDs::band5Type },
    } };

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
    for (int i = 0; i < (int) bandMarkers.size(); ++i)
    {
        const auto position = bandMarkerPosition (i);
        const bool isDragged = i == draggedBandIndex;
        const bool isHovered = i == hoveredBandIndex;
        const float radius = isDragged ? 6.0f : (isHovered ? 5.0f : 4.0f);

        g.setColour (juce::Colours::orange.withAlpha (isDragged || isHovered ? 1.0f : 0.9f));
        g.fillEllipse (position.x - radius, position.y - radius, radius * 2.0f, radius * 2.0f);

        g.setColour (juce::Colours::black.withAlpha (0.6f));
        g.drawEllipse (position.x - radius, position.y - radius, radius * 2.0f, radius * 2.0f, 1.0f);
    }
}

juce::Point<float> EqCurveOverlayComponent::bandMarkerPosition (int bandIndex) const
{
    const auto& info = bandMarkers[(size_t) bandIndex];
    const float freqHz = processor.apvts.getRawParameterValue (info.freqParamId)->load();
    const float magnitudeDb = processor.getMagnitudeForFrequencyDb (freqHz);

    return { AnalyzerMapping::frequencyToX (freqHz, (float) getWidth()),
             AnalyzerMapping::dbToY (magnitudeDb, (float) getHeight()) };
}

int EqCurveOverlayComponent::findNearestBandMarker (juce::Point<float> position) const
{
    int nearestIndex = -1;
    float nearestDistance = hitRadiusPixels;

    for (int i = 0; i < (int) bandMarkers.size(); ++i)
    {
        const float distance = bandMarkerPosition (i).getDistanceFrom (position);

        if (distance <= nearestDistance)
        {
            nearestDistance = distance;
            nearestIndex = i;
        }
    }

    return nearestIndex;
}

bool EqCurveOverlayComponent::bandGainIsDraggable (int bandIndex) const
{
    const auto& info = bandMarkers[(size_t) bandIndex];

    if (info.typeParamId == nullptr)
        return true; // bands 2-4: always Bell/Peak, gain always meaningful

    return processor.apvts.getRawParameterValue (info.typeParamId)->load() >= 0.5f; // index 1 == Shelf
}

void EqCurveOverlayComponent::updateParameterFromDrag (int bandIndex, juce::Point<float> position)
{
    const auto& info = bandMarkers[(size_t) bandIndex];

    if (auto* freqParam = processor.apvts.getParameter (info.freqParamId))
    {
        const float rawHz = AnalyzerMapping::xToFrequency (position.x, (float) getWidth());
        const auto range = freqParam->getNormalisableRange();
        freqParam->setValueNotifyingHost (freqParam->convertTo0to1 (juce::jlimit (range.start, range.end, rawHz)));
    }

    if (bandGainIsDraggable (bandIndex))
    {
        if (auto* gainParam = processor.apvts.getParameter (info.gainParamId))
        {
            const float rawDb = AnalyzerMapping::yToDb (position.y, (float) getHeight());
            const auto range = gainParam->getNormalisableRange();
            gainParam->setValueNotifyingHost (gainParam->convertTo0to1 (juce::jlimit (range.start, range.end, rawDb)));
        }
    }
}

void EqCurveOverlayComponent::mouseDown (const juce::MouseEvent& event)
{
    const int hitIndex = findNearestBandMarker (event.position);

    if (hitIndex < 0)
        return;

    draggedBandIndex = hitIndex;
    setMouseCursor (juce::MouseCursor::DraggingHandCursor);

    const auto& info = bandMarkers[(size_t) hitIndex];

    if (auto* freqParam = processor.apvts.getParameter (info.freqParamId))
        freqParam->beginChangeGesture();

    if (auto* gainParam = processor.apvts.getParameter (info.gainParamId))
        gainParam->beginChangeGesture();

    updateParameterFromDrag (hitIndex, event.position);
}

void EqCurveOverlayComponent::mouseDrag (const juce::MouseEvent& event)
{
    if (draggedBandIndex >= 0)
        updateParameterFromDrag (draggedBandIndex, event.position);
}

void EqCurveOverlayComponent::mouseUp (const juce::MouseEvent&)
{
    if (draggedBandIndex < 0)
        return;

    const auto& info = bandMarkers[(size_t) draggedBandIndex];

    if (auto* freqParam = processor.apvts.getParameter (info.freqParamId))
        freqParam->endChangeGesture();

    if (auto* gainParam = processor.apvts.getParameter (info.gainParamId))
        gainParam->endChangeGesture();

    draggedBandIndex = -1;
    setMouseCursor (hoveredBandIndex >= 0 ? juce::MouseCursor::PointingHandCursor
                                           : juce::MouseCursor::NormalCursor);
}

void EqCurveOverlayComponent::mouseMove (const juce::MouseEvent& event)
{
    hoveredBandIndex = findNearestBandMarker (event.position);
    setMouseCursor (hoveredBandIndex >= 0 ? juce::MouseCursor::PointingHandCursor
                                           : juce::MouseCursor::NormalCursor);
}

void EqCurveOverlayComponent::mouseExit (const juce::MouseEvent&)
{
    hoveredBandIndex = -1;
}
