#pragma once
#include <JuceHeader.h>

class LabeledSlider : public juce::Component {
public:
    juce::Label label;
    juce::Slider slider;

    LabeledSlider(juce::String title, float min, float max, float initialValue) {
        addAndMakeVisible(label);
        label.setText(title, juce::dontSendNotification);
        label.setFont(14.0f);
        label.setJustificationType(juce::Justification::centred);

        addAndMakeVisible(slider);
        slider.setRange(min, max, (max - min) / 100.0f); 
        slider.setValue(initialValue);
        slider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 50, 20);
    }

    void resized() override {
        auto area = getLocalBounds();
        label.setBounds(area.removeFromTop(20));
        slider.setBounds(area);
    }
};

class MappingRow : public juce::Component {
public:
    juce::Label nameLabel;
    juce::ComboBox comboBox;

    MappingRow(juce::String rowName, int defaultID) {
        addAndMakeVisible(nameLabel);
        nameLabel.setText(rowName, juce::dontSendNotification);
        nameLabel.setColour(juce::Label::textColourId, juce::Colours::white);

        // Setup Dropdown
        addAndMakeVisible(comboBox);
        comboBox.addItem("Volume (CC 7)", 1);
        comboBox.addItem("Pitch (Note)", 2);
        comboBox.addItem("Modulation (CC 1)", 3);
        comboBox.addItem("Expression (CC 11)", 4);
        comboBox.addItem("Resonance (CC 71)", 5);
        comboBox.addItem("Vibrato (CC 76)", 6);
        comboBox.addItem("Cutoff (CC 74)", 7);
        comboBox.addItem("None", 8);

        comboBox.setSelectedId(defaultID);
    }

    void resized() override {
        auto area = getLocalBounds();
        nameLabel.setBounds(area.removeFromLeft(150)); 
        comboBox.setBounds(area.reduced(0, 5)); 
    }


};