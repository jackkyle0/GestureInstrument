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

    // Left hand
    MappingRow leftXRow{ "X Axis", 99 };       
    MappingRow leftYRow{ "Y Axis", 2 };       
    MappingRow leftZRow{ "Z Axis", 99 };       
    MappingRow leftWristRow{ "Wrist Roll", 99 };
    MappingRow leftGrabRow{ "Grab (Fist)", 99 }; 
    MappingRow leftPinchRow{ "Pinch", 99 };     

    // Fingers
    MappingRow leftThumbRow{ "Thumb", 99 };
    MappingRow leftIndexRow{ "Index", 99 };
    MappingRow leftMiddleRow{ "Middle", 99 };
    MappingRow leftRingRow{ "Ring", 99 };
    MappingRow leftPinkyRow{ "Pinky", 99 };

    // Right hand
    MappingRow rightXRow{ "X Axis", 99 };     
    MappingRow rightYRow{ "Y Axis", 2 };      
    MappingRow rightZRow{ "Z Axis", 10 };    
    MappingRow rightWristRow{ "Wrist Roll", 99 }; 
    MappingRow rightGrabRow{ "Grab (Fist)", 99 }; 
    MappingRow rightPinchRow{ "Pinch", 3 };     

    // Fingers
    MappingRow rightThumbRow{ "Thumb", 99 };
    MappingRow rightIndexRow{ "Index", 99 };
    MappingRow rightMiddleRow{ "Middle", 99 };
    MappingRow rightRingRow{ "Ring", 99 };
    MappingRow rightPinkyRow{ "Pinky", 99 };

    // Sliders
    LabeledSlider sensitivityControl;
    LabeledSlider minHeightControl;
    LabeledSlider maxHeightControl;

    juce::ComboBox instrumentSelector;
    juce::Label instrumentLabel;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsComponent)
};