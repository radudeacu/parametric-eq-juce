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
    juce::dsp::FFT fft { SpectrumAnalyzerFifo::fftOrder };

    std::array<float, 2 * (size_t) SpectrumAnalyzerFifo::fftSize> fftData {};

    static constexpr int scopeSize = 512;
    std::array<float, (size_t) scopeSize> scopeData {};
};
