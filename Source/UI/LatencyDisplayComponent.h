#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

// Shows the current audio device's round-trip latency (input + output, as
// reported by the driver) so the user can tune buffer size/driver choice
// while watching the number update live. Standalone-only: in a DAW host
// there is no device for the plugin to query, and the host itself is
// responsible for reporting/compensating I/O latency.
class LatencyDisplayComponent : public juce::Component, private juce::Timer
{
public:
    LatencyDisplayComponent();

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    void timerCallback() override;
    void refreshLatencyText();

    juce::Label latencyLabel;
};
