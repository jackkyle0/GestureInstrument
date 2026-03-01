#pragma once

#include <JuceHeader.h>
#include "../PluginProcessor.h"
#include "GuiComponents.h"

class SettingsComponent : public juce::Component {
public:
    SettingsComponent(GestureInstrumentAudioProcessor& p);
    ~SettingsComponent() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    juce::TextButton closeButton{ "Close" };

    juce::ToggleButton enableGestureSwitchButton{ "Enable Gesture to Virtual Mouse" };
    juce::Slider gestureTimerSlider{ juce::Slider::LinearHorizontal, juce::Slider::TextBoxLeft };
    juce::ComboBox gestureTypeSelector;

    std::function<void()> onPresetLoaded;

private:
    GestureInstrumentAudioProcessor& audioProcessor;

    juce::Label titleLabel{ "Settings", "Gesture Mapping" };
    juce::Label leftHandLabel{ "Left Hand", "Left Hand" };
    juce::Label rightHandLabel{ "Right Hand", "Right Hand" };

    MappingRow leftXRow{ "X Axis n/(Side-to-side)", 99 };
    MappingRow leftYRow{ "Y Axis (Height)", 2 };
    MappingRow leftZRow{ "Z Axis (Depth)", 99 };
    MappingRow leftWristRow{ "Wrist Rotation", 99 };
    MappingRow leftGrabRow{ "Grab", 99 };
    MappingRow leftPinchRow{ "Pinch", 99 };

    MappingRow leftThumbRow{ "Thumb", 99 };
    MappingRow leftIndexRow{ "Index", 99 };
    MappingRow leftMiddleRow{ "Middle", 99 };
    MappingRow leftRingRow{ "Ring", 99 };
    MappingRow leftPinkyRow{ "Pinky", 99 };

    MappingRow rightXRow{ "X Axis (Side-to-side)", 99 };
    MappingRow rightYRow{ "Y Axis (Height)", 2 };
    MappingRow rightZRow{ "Z Axis (Depth)", 10 };
    MappingRow rightWristRow{ "Wrist Rotation", 99 };
    MappingRow rightGrabRow{ "Grab", 99 };
    MappingRow rightPinchRow{ "Pinch", 3 };

    MappingRow rightThumbRow{ "Thumb", 99 };
    MappingRow rightIndexRow{ "Index", 99 };
    MappingRow rightMiddleRow{ "Middle", 99 };
    MappingRow rightRingRow{ "Ring", 99 };
    MappingRow rightPinkyRow{ "Pinky", 99 };

    juce::ComboBox instrumentSelector;
    juce::Label instrumentLabel;
    juce::ToggleButton invertTriggerButton{ "Invert Note Off (Selected Gesture to Mute)" };

    juce::ComboBox modeSelector;
    juce::Label modeLabel;

    juce::TextButton savePresetButton{ "Save Preset" };
    juce::TextButton loadPresetButton{ "Load Preset" };
    std::unique_ptr<juce::FileChooser> fileChooser;
    juce::File lastPresetDirectory{ juce::File::getSpecialLocation(juce::File::userDocumentsDirectory) };

    juce::ToggleButton mpeButton{ "Enable MPE" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsComponent)
};