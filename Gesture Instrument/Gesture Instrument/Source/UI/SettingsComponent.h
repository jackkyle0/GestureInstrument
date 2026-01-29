#pragma once

#include <JuceHeader.h>
#include "../PluginProcessor.h"
#include "GuiComponents.h"

class SettingsComponent : public juce::Component
{
public:
    SettingsComponent(GestureInstrumentAudioProcessor& p);
    ~SettingsComponent() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    juce::TextButton closeButton{ "Close" };

private:
    GestureInstrumentAudioProcessor& audioProcessor;

    juce::Label titleLabel{ "Settings", "Gesture Mapping" };

    // Left Hand Rows
    MappingRow leftXRow{ "Left hand X Axis (Side-to-side)", 1 };
    MappingRow leftYRow{ "Left hand Y Axis (Height)", 2 };
    MappingRow leftZRow{ "Left hand Z Axis (Depth)", 3 };
    MappingRow leftWristRow{ "Left hand Wrist Rotation", 3 };

    // Right Hand Rows
    MappingRow rightXRow{ "Right hand X Axis (Side-to-side)", 1 };
    MappingRow rightYRow{ "Right hand Y Axis (Height)", 2 };
    MappingRow rightZRow{ "Right hand Z Axis (Depth)", 3 };
    MappingRow rightWristRow{ "Right hand Wrist Rotation", 3 };

    // Sliders
    LabeledSlider sensitivityControl;
    LabeledSlider minHeightControl;
    LabeledSlider maxHeightControl;

    // Scale controls
    juce::Label scaleLabel;    
    juce::ComboBox rootSelector;
    juce::ComboBox scaleSelector;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsComponent)
};