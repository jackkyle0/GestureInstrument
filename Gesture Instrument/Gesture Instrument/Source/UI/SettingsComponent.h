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
    juce::Label leftHandLabel{ "Left Hand", "Left Hand" };
    juce::Label rightHandLabel{ "Right Hand", "Right Hand" };

    // Lefthand rows
    MappingRow leftXRow{ "X Axis", 1 };
    MappingRow leftYRow{ "Y Axis", 2 };
    MappingRow leftZRow{ "Z Axis", 8 };
    MappingRow leftWristRow{ "Wrist Roll", 8 };
    MappingRow leftGrabRow{ "Grab (Fist)", 8 };
    MappingRow leftPinchRow{ "Pinch", 8 };
    // Fingers
    MappingRow leftThumbRow{ "Thumb", 8 };
    MappingRow leftIndexRow{ "Index", 8 };
    MappingRow leftMiddleRow{ "Middle", 8 };
    MappingRow leftRingRow{ "Ring", 8 };
    MappingRow leftPinkyRow{ "Pinky", 8 };

    // Right hand rows
    MappingRow rightXRow{ "X Axis", 1 };
    MappingRow rightYRow{ "Y Axis", 2 };
    MappingRow rightZRow{ "Z Axis", 8 };
    MappingRow rightWristRow{ "Wrist Roll", 8 };
    MappingRow rightGrabRow{ "Grab (Fist)", 8 };
    MappingRow rightPinchRow{ "Pinch", 8 };
    // Fingers
    MappingRow rightThumbRow{ "Thumb", 8 };
    MappingRow rightIndexRow{ "Index", 8 };
    MappingRow rightMiddleRow{ "Middle", 8 };
    MappingRow rightRingRow{ "Ring", 8 };
    MappingRow rightPinkyRow{ "Pinky", 8 };

    // Sliders
    LabeledSlider sensitivityControl;
    LabeledSlider minHeightControl;
    LabeledSlider maxHeightControl;

    juce::ComboBox instrumentSelector;
    juce::Label instrumentLabel;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsComponent)
};