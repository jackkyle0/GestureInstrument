#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "UI/GuiComponents.h" 

class SettingsComponent : public juce::Component {
public:
    SettingsComponent(GestureInstrumentAudioProcessor& p);
    ~SettingsComponent() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    juce::TextButton closeButton{ "Close" };

private:
    GestureInstrumentAudioProcessor& audioProcessor;

    juce::Label titleLabel{ {}, "Parameter Selection" };


    // Left Hand Controls
    MappingRow leftXRow{ "Left Hand X (Side)", 3 };     
    MappingRow leftYRow{ "Left Hand Y (Height)", 1 };     // Default Volume
    MappingRow leftZRow{ "Left Hand Z (Depth)", 3 };
    MappingRow leftWristRow{ "Left Hand Rotation", 3 };

    // Right Hand Controls
    MappingRow rightXRow{ "Right Hand X (Side)", 2 };      // Default Pitch
    MappingRow rightYRow{ "Right Hand Y (Height)", 3 };
    MappingRow rightZRow{ "Right Hand Z (Depth)", 3 };
    MappingRow rightWristRow{ "Right Hand Rotation", 3 };

    // Sliders
    LabeledSlider sensitivityControl;
    LabeledSlider minHeightControl;
    LabeledSlider maxHeightControl;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsComponent)
};