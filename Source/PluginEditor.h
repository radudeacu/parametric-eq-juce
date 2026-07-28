#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "UI/BandControlComponent.h"
#include "UI/GlobalControlsComponent.h"
#include "UI/TestToneControlsComponent.h"
#include "UI/ModernLookAndFeel.h"
#include "Analyzer/AnalyzerPanelComponent.h"

class ParametricEQAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit ParametricEQAudioProcessorEditor (ParametricEQAudioProcessor& processor);
    ~ParametricEQAudioProcessorEditor() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    ParametricEQAudioProcessor& audioProcessor;

    // Declared first so it's constructed before, and destroyed after, every
    // child component below that depends on it via getLookAndFeel().
    ModernLookAndFeel lookAndFeel;

    AnalyzerPanelComponent analyzerPanel;
    TestToneControlsComponent testToneControls;
    BandControlComponent band1Controls, band2Controls, band3Controls, band4Controls, band5Controls;
    GlobalControlsComponent globalControls;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParametricEQAudioProcessorEditor)
};
