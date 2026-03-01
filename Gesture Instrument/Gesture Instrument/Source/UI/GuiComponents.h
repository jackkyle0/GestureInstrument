#pragma once
#include <JuceHeader.h>

struct MappingRow : public juce::Component {
    juce::Label label;
    juce::ComboBox comboBox;

    MappingRow(juce::String labelText, int defaultId) {
        addAndMakeVisible(label);
        label.setText(labelText, juce::dontSendNotification);
        label.setColour(juce::Label::textColourId, juce::Colours::white);

        addAndMakeVisible(comboBox);

        comboBox.addItem("None", 99);
        comboBox.addSeparator();

        comboBox.addItem("Volume", 1);
        comboBox.addItem("Pitch", 2);
        comboBox.addItem("Note Off", 3);
        comboBox.addItem("Modulation", 4);
        comboBox.addItem("Expression", 5);
        comboBox.addItem("Breath", 6);
        comboBox.addItem("Cutoff", 7);
        comboBox.addItem("Resonance", 8);
        comboBox.addItem("Attack", 9);
        comboBox.addItem("Release", 10);
        comboBox.addItem("Vibrato", 11);
        comboBox.addItem("Pan", 12);
        comboBox.addItem("Reverb", 13);
        comboBox.addItem("Chorus", 14);
        comboBox.addItem("Sustain", 15);
        comboBox.addItem("Portamento", 16);
        comboBox.addItem("Waveform", 17);

        comboBox.setSelectedId(defaultId, juce::dontSendNotification);
    }

    void resized() override {
        auto bounds = getLocalBounds();
        label.setBounds(bounds.removeFromLeft(100));
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