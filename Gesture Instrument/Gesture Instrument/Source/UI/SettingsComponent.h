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
    void refreshUI();

    juce::TextButton closeButton{ "Close" };
    std::function<void()> onPresetLoaded;

private:
    GestureInstrumentAudioProcessor& audioProcessor;

    // UI labels
    juce::Label titleLabel{ "Settings", "Gesture Mapping" };
    juce::Label leftHandLabel{ "Left Hand", "Left Hand" };
    juce::Label rightHandLabel{ "Right Hand", "Right Hand" };
    juce::Label visualsLabel{ "VISUALS", "VISUALS" };
    juce::Label standaloneLabel{ "STANDALONE SYNTH", "STANDALONE SYNTH" };
    juce::Label modeLabel{ "Output Mode:", "OUTPUT MODE:" };
    juce::Label midiLabel{ "Midi", "MIDI" };
    juce::Label advCalibLabel{ "Advanced", "ADVANCED CALIBRATION" };
    juce::Label presetLabel{ "Save/Load Preset", "SAVE/LOAD PRESET" };

    // System toggles
    juce::ComboBox modeSelector;
    juce::ComboBox instrumentSelector;
    juce::Label instrumentLabel;

    juce::ToggleButton floorShadowToggle{ "Floor Shadow" };
    juce::ToggleButton wallShadowToggle{ "Wall Shadow" };
    juce::ToggleButton splitXAxisToggle{ "Split X-Axis" };
    juce::ToggleButton invertTriggerButton{ "Invert Mute" };

    // Virtual mouse
    juce::Label virtualMouseLabel{ "Virtual Mouse", "VIRTUAL MOUSE" };
    juce::ToggleButton enableGestureSwitchButton{ "Enable Gesture to Virtual Mouse" };
    juce::Slider gestureTimerSlider{ juce::Slider::LinearHorizontal, juce::Slider::TextBoxLeft };
    juce::ComboBox gestureTypeSelector;

    // Preset management
    juce::TextButton savePresetButton{ "Save Preset" };
    juce::TextButton loadPresetButton{ "Load Preset" };
    std::unique_ptr<juce::FileChooser> fileChooser;
    juce::File lastPresetDirectory{ juce::File::getSpecialLocation(juce::File::userDocumentsDirectory) };

    // MPE 
    juce::ToggleButton mpeButton{ "Enable MPE" };
    juce::Label mpeRoutingLabel{ "MPE", "MPE ROUTING:" };

    juce::Label mpePitchLabel{ "Pitch", "Pitch Bend (Glide):" };
    juce::ComboBox mpePitchSelector;

    juce::Label mpeTimbreLabel{ "Timbre", "Timbre (Slide):" };
    juce::ComboBox mpeTimbreSelector;

    juce::Label mpePressureLabel{ "Pressure", "Pressure (Press):" };
    juce::ComboBox mpePressureSelector;


    // Left hand
    MappingRow leftXRow{ "X Axis \n(Side-to-side)", 99 };
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

    // Right hand
    MappingRow rightXRow{ "X Axis \n(Side-to-side)", 99 };
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

	// Advanced calibration
    LabeledSlider wristMultControl{ "Wrist Sens", 1.0f, 3.0f, 1.0f };
    LabeledSlider grabMultControl{ "Grab Sens", 1.0f, 3.0f, 1.0f };
    LabeledSlider pinchMultControl{ "Pinch Sens", 1.0f, 3.0f, 1.0f };

   // helpers
    int getIdFromTarget(GestureTarget target);
    GestureTarget getTargetFromId(int id);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsComponent)
};