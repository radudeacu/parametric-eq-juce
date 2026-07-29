#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_dsp/juce_dsp.h>
#include "SpectrumAnalyzerFifo.h"
#include <array>

// Timer-driven FFT display of the pre-EQ signal fed by SpectrumAnalyzerFifo.
class SpectrumAnalyzerComponent : public juce::Component, private juce::Timer
{
public:
    explicit SpectrumAnalyzerComponent (SpectrumAnalyzerFifo& fifoToUse);

    void paint (juce::Graphics& g) override;

private:
    void timerCallback() override;
    void drawNextFrameOfSpectrum();

    SpectrumAnalyzerFifo& fifo;

    // The real accumulation window stays at SpectrumAnalyzerFifo::fftSize (no
    // added latency), but the transform itself runs zero-padded 8x larger.
    // Zero-padding doesn't add real frequency information, but it does
    // sinc-interpolate many more display bins between the real ones,
    // smoothing out the staircase blockiness that's otherwise most visible
    // at the log-compressed low end (2048 real samples @ 48kHz is only
    // ~23Hz/bin; 20-50Hz spans barely one bin without padding).
    static constexpr int paddedFftOrder = SpectrumAnalyzerFifo::fftOrder + 3; // 8x zero-padding
    static constexpr int paddedFftSize = 1 << paddedFftOrder;

    juce::dsp::FFT fft { paddedFftOrder };
    std::array<float, 2 * (size_t) SpectrumAnalyzerFifo::fftSize> fftData {};
    std::array<float, 2 * (size_t) paddedFftSize> paddedFftData {};

    static constexpr int scopeSize = 512;
    std::array<float, (size_t) scopeSize> scopeData {};
};
