#pragma once
#include <JuceHeader.h>

struct MappingRow : public juce::Component {
    juce::Label label;
    juce::ComboBox comboBox;

    MappingRow(juce::String labelText, int defaultId) {
        addAndMakeVisible(label);
        label.setText(labelText, juce::dontSendNotification);
        label.setColour(juce::Label::textColourId, juce::Colours::white);

        label.setMinimumHorizontalScale(1.0f);
        addAndMakeVisible(comboBox);

        comboBox.setSelectedId(defaultId, juce::dontSendNotification);
    }

    void updateList(bool isOSCMode) {
        int currentlySelected = comboBox.getSelectedId();
        comboBox.clear();

        comboBox.addItem("None", 1);
        comboBox.addSeparator();

        comboBox.addItem("Volume", 2);
        comboBox.addItem("Pitch", 3);
        
        comboBox.addItem("Cutoff", 8);
        comboBox.addItem("Resonance", 9);
        comboBox.addItem("Attack", 10);
        comboBox.addItem("Release", 11);
        comboBox.addItem("Vibrato", 12);
        comboBox.addItem("Pan", 13);
        comboBox.addItem("Reverb", 14);
        comboBox.addItem("Chorus", 15);

        if (isOSCMode) {
            comboBox.addItem("Waveform", 18);
            comboBox.addItem("Delay", 19);
            comboBox.addItem("Distortion", 20);
        }
        else {
            comboBox.addItem("Mute", 4);
            comboBox.addItem("Modulation", 5);
            comboBox.addItem("Expression", 6);
            comboBox.addItem("Breath", 7);
            comboBox.addItem("Sustain", 16);
            comboBox.addItem("Portamento", 17);
        }

        if (comboBox.getItemText(comboBox.indexOfItemId(currentlySelected)).isEmpty()) {
            comboBox.setSelectedId(1, juce::dontSendNotification);
        }
        else {
            comboBox.setSelectedId(currentlySelected, juce::dontSendNotification);
        }
    }

    void resized() override {
        auto bounds = getLocalBounds();
        label.setBounds(bounds.removeFromLeft(120));
        comboBox.setBounds(bounds.reduced(0, 2));
    }
};

    

struct LabeledSlider : public juce::Component {
    juce::Label label;
    juce::Slider slider;

    LabeledSlider(juce::String name, float min, float max, float initial) {
        addAndMakeVisible(label);
        label.setText(name, juce::dontSendNotification);

        addAndMakeVisible(slider);
        slider.setRange(min, max);
        slider.setValue(initial);
        slider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 50, 20);
    }

    void resized() override {
        auto bounds = getLocalBounds();
        label.setBounds(bounds.removeFromLeft(80));
        slider.setBounds(bounds);
    }
};

struct LabeledButton : public juce::Component {
    juce::Label label;
    juce::ToggleButton button;

    LabeledButton(juce::String name, bool initialState) {
        addAndMakeVisible(label);
        label.setText(name, juce::dontSendNotification);
        label.setColour(juce::Label::textColourId, juce::Colours::white);

        addAndMakeVisible(button);
        button.setToggleState(initialState, juce::dontSendNotification);

    }

    void resized() override {
        auto bounds = getLocalBounds();

        label.setBounds(bounds.removeFromLeft(80));

        button.setBounds(bounds);
    }

};