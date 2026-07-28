#include "PluginEditor.h"
#include "Parameters/ParameterIDs.h"

ParametricEQAudioProcessorEditor::ParametricEQAudioProcessorEditor (ParametricEQAudioProcessor& processor)
    : AudioProcessorEditor (&processor), audioProcessor (processor),
      analyzerComponent (processor.analyzerFifo),
      testToneControls (processor.apvts),
      band1Controls (processor.apvts, "Low", ParamIDs::band1FreqHz, ParamIDs::band1GainDb, ParamIDs::band1Q,
                     ParamIDs::band1Type, ParamIDs::band1Slope),
      band2Controls (processor.apvts, "Low-Mid", ParamIDs::band2FreqHz, ParamIDs::band2GainDb, ParamIDs::band2Q),
      band3Controls (processor.apvts, "Mid", ParamIDs::band3FreqHz, ParamIDs::band3GainDb, ParamIDs::band3Q),
      band4Controls (processor.apvts, "High-Mid", ParamIDs::band4FreqHz, ParamIDs::band4GainDb, ParamIDs::band4Q),
      band5Controls (processor.apvts, "High", ParamIDs::band5FreqHz, ParamIDs::band5GainDb, ParamIDs::band5Q,
                     ParamIDs::band5Type, ParamIDs::band5Slope),
      globalControls (processor.apvts)
{
    addAndMakeVisible (analyzerComponent);
    addAndMakeVisible (testToneControls);

    for (auto* band : { &band1Controls, &band2Controls, &band3Controls, &band4Controls, &band5Controls })
        addAndMakeVisible (band);

    addAndMakeVisible (globalControls);

    setSize (900, 600);
}

void ParametricEQAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void ParametricEQAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();

    analyzerComponent.setBounds (area.removeFromTop (250));
    testToneControls.setBounds (area.removeFromTop (40));
    globalControls.setBounds (area.removeFromBottom (60));

    const int bandWidth = area.getWidth() / 5;
    for (auto* band : { &band1Controls, &band2Controls, &band3Controls, &band4Controls, &band5Controls })
        band->setBounds (area.removeFromLeft (bandWidth));
}
