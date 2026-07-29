#include "SpectrumAnalyzerComponent.h"
#include "AnalyzerMapping.h"
#include "../UI/Theme.h"
#include <algorithm>

SpectrumAnalyzerComponent::SpectrumAnalyzerComponent (SpectrumAnalyzerFifo& fifoToUse) : fifo (fifoToUse)
{
    startTimerHz (30);
}

void SpectrumAnalyzerComponent::paint (juce::Graphics& g)
{
    g.fillAll (Theme::analyzerBackground);

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
    std::fill (paddedFftData.begin(), paddedFftData.end(), 0.0f);
    std::copy_n (fftData.begin(), SpectrumAnalyzerFifo::fftSize, paddedFftData.begin());

    fft.performFrequencyOnlyForwardTransform (paddedFftData.data());

    constexpr float minDb = -100.0f;
    constexpr float maxDb = 0.0f;
    // Normalise against the real (unpadded) sample count: zero-padding adds
    // interpolated bins but doesn't change the DFT's magnitude scaling,
    // which is driven by the number of non-zero input samples.
    constexpr int realFftSize = SpectrumAnalyzerFifo::fftSize;
    const float sampleRate = (float) fifo.getSampleRate();

    for (int i = 0; i < scopeSize; ++i)
    {
        const float proportion = (float) i / (float) (scopeSize - 1);
        const float freqHz = AnalyzerMapping::proportionToFrequency (proportion);
        const int fftDataIndex = juce::jlimit (0, paddedFftSize / 2,
                                                juce::roundToInt (freqHz * (float) paddedFftSize / sampleRate));
        const float levelDb = juce::Decibels::gainToDecibels (paddedFftData[(size_t) fftDataIndex])
                               - juce::Decibels::gainToDecibels ((float) realFftSize);

        scopeData[(size_t) i] = juce::jmap (juce::jlimit (minDb, maxDb, levelDb), minDb, maxDb, 0.0f, 1.0f);
    }
}
