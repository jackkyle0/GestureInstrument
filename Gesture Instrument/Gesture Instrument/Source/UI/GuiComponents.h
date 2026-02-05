#pragma once
#include <JuceHeader.h>

struct MappingRow : public juce::Component
{
    juce::Label label;
    juce::ComboBox comboBox;

    MappingRow(juce::String labelText, int defaultId)
    {
        // setup label
        addAndMakeVisible(label);
        label.setText(labelText, juce::dontSendNotification);
        label.setColour(juce::Label::textColourId, juce::Colours::white);

        // setup comboBox
        addAndMakeVisible(comboBox);

        // Options
        comboBox.addItem("None", 99); 
        comboBox.addSeparator();

        // Essentials
        comboBox.addItem("Volume (CC 7)", 1);
        comboBox.addItem("Pitch (Bend)", 2);
        comboBox.addItem("Note Trigger (No Bend)", 13);
        comboBox.addItem("Modulation (CC 1)", 3);
        comboBox.addItem("Expression (CC 11)", 4);

        // Shaping
        comboBox.addItem("Cutoff (CC 74)", 5);
        comboBox.addItem("Resonance (CC 71)", 6);
        comboBox.addItem("Vibrato (CC 76)", 10);

        // Spatial
        comboBox.addItem("Pan (CC 10)", 7);
        comboBox.addItem("Reverb (CC 91)", 8);

        // Switches
        comboBox.addItem("Sustain (CC 64)", 9);

        // Macros
        comboBox.addSeparator();
        comboBox.addItem("Macro 1 (CC 20)", 11);
        comboBox.addItem("Macro 2 (CC 21)", 12);

        // Set Default
        comboBox.setSelectedId(defaultId, juce::dontSendNotification);
    }

    void resized() override
    {
        auto bounds = getLocalBounds();
        label.setBounds(bounds.removeFromLeft(100)); 
        comboBox.setBounds(bounds.reduced(0, 2));  
    }
};

struct LabeledSlider : public juce::Component
{
    juce::Label label;
    juce::Slider slider;

    LabeledSlider(juce::String name, float min, float max, float initial)
    {
        addAndMakeVisible(label);
        label.setText(name, juce::dontSendNotification);

        addAndMakeVisible(slider);
        slider.setRange(min, max);
        slider.setValue(initial);
        slider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 50, 20);
    }

    void resized() override
    {
        auto bounds = getLocalBounds();
        label.setBounds(bounds.removeFromLeft(80));
        slider.setBounds(bounds);
    }
};