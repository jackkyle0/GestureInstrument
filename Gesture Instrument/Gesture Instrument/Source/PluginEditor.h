#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Helpers/HandData.h"
#include "UI/SettingsComponent.h"
#include "MIDI/GestureTarget.h"

class GestureInstrumentAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Timer, public juce::ComboBox::Listener
{
public:
    GestureInstrumentAudioProcessorEditor (GestureInstrumentAudioProcessor&);
    ~GestureInstrumentAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    
    void timerCallback() override;
    void updateConnectionStatus();

    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;

private:
    GestureInstrumentAudioProcessor& audioProcessor;
    
    juce::Label connectionStatusLabel;
    void drawHand(juce::Graphics& g, const HandData& hand, juce::Colour colour, const juce::String& label);
    void drawGrid(juce::Graphics& g);

    juce::ComboBox modeSelector;
    juce::Label modeLabel;

    juce::TextButton settingsButton{ "Settings" };
    SettingsComponent settingsPage;

    juce::OpenGLContext openGLContext;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GestureInstrumentAudioProcessorEditor)
};
