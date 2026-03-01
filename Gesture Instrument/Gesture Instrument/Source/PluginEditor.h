#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "UI/SettingsComponent.h"
#include "UI/HUDComponents.h"
#include "UI/VirtualCursor.h"
#include "UI/StaticDialsComponent.h"

struct Point3D {
    float x, y, z;
};

class GestureInstrumentAudioProcessorEditor : public juce::AudioProcessorEditor,
    public juce::ComboBox::Listener,
    public juce::Timer {
public:
    GestureInstrumentAudioProcessorEditor(GestureInstrumentAudioProcessor&);
    ~GestureInstrumentAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;
    void comboBoxChanged(juce::ComboBox* box) override;



    class CalibrationOverlay : public juce::Component {
    public:
        void setProgress(float p);
        void paint(juce::Graphics& g) override;
    private:
        float progress = 0.0f;
    };



private:
    GestureInstrumentAudioProcessor& audioProcessor;

    StaticDialsComponent staticDialsPage;
    juce::TextButton staticDialsButton{ "Dials" };

    // Overlays
    juce::OpenGLContext openGLContext;
    SettingsComponent settingsPage;
    HUDComponents hud;
    VirtualCursor virtualCursor;
    CalibrationOverlay calibrationOverlay;

    // Buttons and labels
    juce::TextButton settingsButton{ "Settings" };
    juce::TextButton calibrateButton{ "Calibrate Area" };
    juce::TextButton editModeButton{ "Virtual Mouse" };
    juce::ToggleButton showNoteNamesButton{ "Show Note Names" };
    juce::Label connectionStatusLabel;

    // Selectors
    juce::ComboBox rootSelector;
    juce::ComboBox scaleSelector;
    juce::Label scaleLabel;

    // Musical Range Selectors
    juce::ComboBox rangeModeSelector;
    juce::ComboBox octaveSelector;
    juce::ComboBox startNoteSelector;
    juce::ComboBox endNoteSelector;
    juce::Label octaveLabel;



    // Sliders
    LabeledSlider xMinControl;
    LabeledSlider xMaxControl;
    LabeledSlider yMinControl;
    LabeledSlider yMaxControl;
    LabeledSlider zMinControl;
    LabeledSlider zMaxControl;


 

    bool isEditMode = false;
    float menuGestureTimer = 0.0f;
    bool menuGestureFired = false;

    // Calibration
    void startCalibration();
    void stopCalibration(bool success);

    bool isCalibrating = false;
    float calibrationTimer = 0.0f;
    const float calibrationDuration = 15.0f;

    float tempMinX = 1000.0f, tempMaxX = -1000.0f;
    float tempMinY = 1000.0f, tempMaxY = -1000.0f;
    float tempMinZ = 1000.0f, tempMaxZ = -1000.0f;

    void updateConnectionStatus();
    juce::Point<float> projectPoint(Point3D p);
    void draw3DGrid(juce::Graphics& g);
    void draw3DHand(juce::Graphics& g, const HandData& hand, juce::Colour baseColour);
    void drawCalibrationBox3D(juce::Graphics& g);

    const float fov = 350.0f;
    const float camDist = 600.0f;
    juce::Point<float> centerScreen;
    float currentStyle = 0.0f;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GestureInstrumentAudioProcessorEditor)
};