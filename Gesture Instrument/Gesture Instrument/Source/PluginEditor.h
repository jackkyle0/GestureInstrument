#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "UI/SettingsComponent.h"

struct Point3D {
    float x, y, z;
};

class GestureInstrumentAudioProcessorEditor : public juce::AudioProcessorEditor,
    public juce::ComboBox::Listener,
    public juce::Timer
{
public:
    GestureInstrumentAudioProcessorEditor(GestureInstrumentAudioProcessor&);
    ~GestureInstrumentAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    void timerCallback() override;
    void comboBoxChanged(juce::ComboBox* box) override;

private:
    void updateConnectionStatus();

    // 3D Render
    juce::Point<float> projectPoint(Point3D p);
    void draw3DGrid(juce::Graphics& g);
    void draw3DHand(juce::Graphics& g, const HandData& hand, juce::Colour baseColour);

    // 3D Camera 
    const float fov = 350.0f;      // Field of View (Zoom)
    const float camDist = 600.0f;  // Distance from camera
    juce::Point<float> centerScreen;

    GestureInstrumentAudioProcessor& audioProcessor;

    // Components
    SettingsComponent settingsPage;

    juce::TextButton settingsButton{ "Settings" };
    juce::Label connectionStatusLabel;

    juce::ComboBox modeSelector;
    juce::Label modeLabel;

    // Scale 
    juce::ComboBox rootSelector;
    juce::ComboBox scaleSelector;
    juce::Label scaleLabel;

    juce::OpenGLContext openGLContext;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GestureInstrumentAudioProcessorEditor)
};