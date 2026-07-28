#include "SpectrumAnalyzerFifo.h"

void SpectrumAnalyzerFifo::prepare (double sampleRateToUse, int maxSamplesPerBlock)
{
    sampleRate.store (sampleRateToUse);
    ringBuffer.setSize (1, fifo.getTotalSize());
    monoScratch.setSize (1, maxSamplesPerBlock);
    reset();
}

void SpectrumAnalyzerFifo::reset()
{
    fifo.reset();
    ringBuffer.clear();
    monoScratch.clear();
    accumulator.fill (0.0f);
    accumulatorIndex = 0;
}

void SpectrumAnalyzerFifo::pushSamples (const juce::AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    if (numSamples <= 0 || numChannels <= 0 || numSamples > monoScratch.getNumSamples())
        return;

    monoScratch.copyFrom (0, 0, buffer, 0, 0, numSamples);
    for (int ch = 1; ch < numChannels; ++ch)
        monoScratch.addFrom (0, 0, buffer, ch, 0, numSamples);
    monoScratch.applyGain (0, 0, numSamples, 1.0f / (float) numChannels);

    int start1 = 0, size1 = 0, start2 = 0, size2 = 0;
    fifo.prepareToWrite (numSamples, start1, size1, start2, size2);

    if (size1 > 0)
        ringBuffer.copyFrom (0, start1, monoScratch, 0, 0, size1);
    if (size2 > 0)
        ringBuffer.copyFrom (0, start2, monoScratch, 0, size1, size2);

    fifo.finishedWrite (size1 + size2);
}

bool SpectrumAnalyzerFifo::popFftBlockWhenReady (std::array<float, 2 * (size_t) fftSize>& fftData)
{
    const int wanted = juce::jmin (fifo.getNumReady(), fftSize - accumulatorIndex);

    if (wanted <= 0)
        return false;

    int start1 = 0, size1 = 0, start2 = 0, size2 = 0;
    fifo.prepareToRead (wanted, start1, size1, start2, size2);

    for (int i = 0; i < size1; ++i)
        accumulator[(size_t) accumulatorIndex++] = ringBuffer.getSample (0, start1 + i);
    for (int i = 0; i < size2; ++i)
        accumulator[(size_t) accumulatorIndex++] = ringBuffer.getSample (0, start2 + i);

    fifo.finishedRead (size1 + size2);

    if (accumulatorIndex < fftSize)
        return false;

    std::fill (fftData.begin(), fftData.end(), 0.0f);
    std::copy (accumulator.begin(), accumulator.end(), fftData.begin());
    window.multiplyWithWindowingTable (fftData.data(), fftSize);
    accumulatorIndex = 0;
    return true;
}
