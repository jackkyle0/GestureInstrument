#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class SettingsComponent : public juce::Component {
public:
    SettingsComponent(GestureInstrumentAudioProcessor& p);
    ~SettingsComponent() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    juce::TextButton closeButton{ "Close" };

private:
    GestureInstrumentAudioProcessor& audioProcessor;

    // Parameter Mapping
    juce::Label titleLabel{ {}, "Parameter Selection" };
    juce::Label handAxisTitle{ {}, "Hand / Axis" };
    juce::Label mappedParamTitle{ {}, "Mapped Parameter" };

    // Row Labels
    juce::Label leftXLabel{ {}, "Left Hand X (Side)" };
    juce::Label leftYLabel{ {}, "Left Hand Y (Height)" };
    juce::Label rightXLabel{ {}, "Right Hand X (Side)" };
    juce::Label rightYLabel{ {}, "Right Hand Y (Height)" };

    // Dropdowns
    juce::ComboBox leftXBox;
    juce::ComboBox leftYBox;
    juce::ComboBox rightXBox;
    juce::ComboBox rightYBox;

   // Sensivity and sliders
    juce::Slider sensitivitySlider;
    juce::Label  sensitivityLabel{ {}, "Sensitivity" };

    juce::Slider minHeightSlider;
    juce::Label  minHeightLabel{ {}, "Min Height Threshold" };

    juce::Slider maxHeightSlider;
    juce::Label  maxHeightLabel{ {}, "Max Height Threshold" };

    void setupComboBox(juce::ComboBox& box, int defaultID);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsComponent)
};