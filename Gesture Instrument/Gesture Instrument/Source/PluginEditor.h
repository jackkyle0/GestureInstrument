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
    const float fov = 350.0f;      // Field of view
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

    juce::ComboBox octaveSelector;
    juce::Label octaveLabel;

    LabeledSlider xMinControl;
    LabeledSlider xMaxControl; 
    LabeledSlider yMinControl;
    LabeledSlider yMaxControl;
    LabeledSlider zMinControl;
    LabeledSlider zMaxControl;

    void drawPitchBlocks(juce::Graphics& g, juce::Rectangle<int> bounds, float pitchVal, juce::String handName, juce::Colour color);

    juce::ToggleButton showNoteNamesButton{ "Show Note Names" };

    std::vector<int> getScaleIntervals(int scaleType);

    void drawParameterHUD(juce::Graphics& g, juce::Rectangle<int> bounds,
        GestureTarget target, float value,
        bool isVertical, juce::String label, juce::Colour color);

    void drawScaleBlocks(juce::Graphics& g, juce::Rectangle<int> bounds, float value, bool isVertical, juce::Colour color);
    void drawFaderBar(juce::Graphics& g, juce::Rectangle<int> bounds, float value, bool isVertical, juce::Colour color);
    void drawBooleanBox(juce::Graphics& g, juce::Rectangle<int> bounds, float value, juce::Colour color);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GestureInstrumentAudioProcessorEditor)
};