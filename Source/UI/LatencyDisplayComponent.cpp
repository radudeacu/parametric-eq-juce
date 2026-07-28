#include "LatencyDisplayComponent.h"
#include "GlassPanel.h"
#include "Theme.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_audio_plugin_client/Standalone/juce_StandaloneFilterWindow.h>

LatencyDisplayComponent::LatencyDisplayComponent()
{
    latencyLabel.setJustificationType (juce::Justification::centred);
    latencyLabel.setFont (juce::Font (juce::FontOptions (14.0f)));
    latencyLabel.setColour (juce::Label::textColourId, Theme::textPrimary);
    addAndMakeVisible (latencyLabel);

    refreshLatencyText();
    startTimerHz (2);
}

void LatencyDisplayComponent::paint (juce::Graphics& g)
{
    GlassPanel::draw (g, getLocalBounds().toFloat());
}

void LatencyDisplayComponent::resized()
{
    latencyLabel.setBounds (getLocalBounds());
}

void LatencyDisplayComponent::timerCallback()
{
    refreshLatencyText();
}

void LatencyDisplayComponent::refreshLatencyText()
{
    auto* holder = juce::StandalonePluginHolder::getInstance();
    auto* device = (holder != nullptr) ? holder->deviceManager.getCurrentAudioDevice() : nullptr;

    if (device == nullptr)
    {
        latencyLabel.setText ("Latency: host-controlled", juce::dontSendNotification);
        return;
    }

    const int inputLatencySamples = device->getInputLatencyInSamples();
    const int outputLatencySamples = device->getOutputLatencyInSamples();
    const double sampleRate = device->getCurrentSampleRate();
    const int bufferSize = device->getCurrentBufferSizeSamples();

    const double roundTripMs = sampleRate > 0.0
        ? (double) (inputLatencySamples + outputLatencySamples) * 1000.0 / sampleRate
        : 0.0;

    latencyLabel.setText ("Round-trip latency: " + juce::String (roundTripMs, 1) + " ms  (buffer "
                               + juce::String (bufferSize) + " @ " + juce::String (sampleRate, 0) + " Hz, "
                               + device->getName() + ")",
                           juce::dontSendNotification);
}
