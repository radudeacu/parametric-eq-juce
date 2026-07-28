#pragma once

#include <juce_dsp/juce_dsp.h>
#include <array>
#include <atomic>

// Lock-free, alloc-free hand-off of the pre-EQ signal from the audio thread
// to the UI thread. Audio thread mixes to mono and writes into a ring buffer;
// UI thread drains it, accumulates a windowed FFT-sized block, and reads it out.
class SpectrumAnalyzerFifo
{
public:
    static constexpr int fftOrder = 11;
    static constexpr int fftSize = 1 << fftOrder; // 2048

    void prepare (double sampleRateToUse, int maxSamplesPerBlock);
    void reset();

    double getSampleRate() const noexcept { return sampleRate.load(); }

    // AUDIO THREAD.
    void pushSamples (const juce::AudioBuffer<float>& buffer);

    // UI THREAD. Returns true once fftData holds a full windowed, zero-padded block.
    bool popFftBlockWhenReady (std::array<float, 2 * (size_t) fftSize>& fftData);

private:
    std::atomic<double> sampleRate { 44100.0 };
    juce::AbstractFifo fifo { 1 << 13 };
    juce::AudioBuffer<float> ringBuffer;
    juce::AudioBuffer<float> monoScratch;

    std::array<float, (size_t) fftSize> accumulator {};
    int accumulatorIndex = 0;

    juce::dsp::WindowingFunction<float> window { (size_t) fftSize, juce::dsp::WindowingFunction<float>::hann };
};
