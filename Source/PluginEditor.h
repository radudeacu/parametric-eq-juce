#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "UI/BandControlComponent.h"
#include "UI/GlobalControlsComponent.h"
#include "UI/TestToneControlsComponent.h"
#include "Analyzer/AnalyzerPanelComponent.h"

class ParametricEQAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit ParametricEQAudioProcessorEditor (ParametricEQAudioProcessor& processor);

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    ParametricEQAudioProcessor& audioProcessor;

    AnalyzerPanelComponent analyzerPanel;
    TestToneControlsComponent testToneControls;
    BandControlComponent band1Controls, band2Controls, band3Controls, band4Controls, band5Controls;
    GlobalControlsComponent globalControls;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParametricEQAudioProcessorEditor)
};
