#include "SpectrumAnalyzerComponent.h"

SpectrumAnalyzerComponent::SpectrumAnalyzerComponent (SpectrumAnalyzerFifo& fifoToUse) : fifo (fifoToUse)
{
    startTimerHz (30);
}

void SpectrumAnalyzerComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);

    const float width = (float) getWidth();
    const float height = (float) getHeight();
    juce::Path path;

    for (int i = 0; i < scopeSize; ++i)
    {
        const float x = juce::jmap ((float) i, 0.0f, (float) (scopeSize - 1), 0.0f, width);
        const float y = juce::jmap (scopeData[(size_t) i], 0.0f, 1.0f, height, 0.0f);

        if (i == 0)
            path.startNewSubPath (x, y);
        else
            path.lineTo (x, y);
    }

    g.setColour (juce::Colours::lightgreen);
    g.strokePath (path, juce::PathStrokeType (1.5f));
}

void SpectrumAnalyzerComponent::timerCallback()
{
    if (fifo.popFftBlockWhenReady (fftData))
    {
        drawNextFrameOfSpectrum();
        repaint();
    }
}

void SpectrumAnalyzerComponent::drawNextFrameOfSpectrum()
{
    fft.performFrequencyOnlyForwardTransform (fftData.data());

    constexpr float minDb = -100.0f;
    constexpr float maxDb = 0.0f;
    constexpr int fftSize = SpectrumAnalyzerFifo::fftSize;

    for (int i = 0; i < scopeSize; ++i)
    {
        const float skewedProportionX = 1.0f - std::exp (std::log (1.0f - (float) i / (float) scopeSize) * 0.2f);
        const int fftDataIndex = juce::jlimit (0, fftSize / 2, (int) (skewedProportionX * (float) fftSize * 0.5f));
        const float levelDb = juce::Decibels::gainToDecibels (fftData[(size_t) fftDataIndex])
                               - juce::Decibels::gainToDecibels ((float) fftSize);

        scopeData[(size_t) i] = juce::jmap (juce::jlimit (minDb, maxDb, levelDb), minDb, maxDb, 0.0f, 1.0f);
    }
}
