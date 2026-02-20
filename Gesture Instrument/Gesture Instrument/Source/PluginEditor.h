#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "UI/SettingsComponent.h"
#include "UI/HUDComponents.h"
#include "UI/VirtualCursor.h"

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

    class CalibrationOverlay : public juce::Component
    {
    public:
        void setExtents(float minX, float maxX, float minY, float maxY);
        void setHandPos(float x, float y); // Real-time dot
        void setProgress(float p);         // 0.0 to 1.0

        void paint(juce::Graphics& g) override;

    private:
        juce::Rectangle<float> currentBounds;
        juce::Point<float> handPos;
        float progress = 0.0f;
    };

private:
    void updateConnectionStatus();

    // 3D Render
    juce::Point<float> projectPoint(Point3D p);
    void draw3DGrid(juce::Graphics& g);
    void draw3DHand(juce::Graphics& g, const HandData& hand, juce::Colour baseColour);
    // Add Z-axis tracking
    float tempMinZ = 1000.0f, tempMaxZ = -1000.0f;

    // Add this new function declaration near draw3DGrid
    void drawCalibrationBox3D(juce::Graphics& g);

    // 3D Camera 
    const float fov = 350.0f;      // Field of view
    const float camDist = 600.0f;  // Distance from camera
    juce::Point<float> centerScreen;

    GestureInstrumentAudioProcessor& audioProcessor;

    // Components
    SettingsComponent settingsPage;

    juce::TextButton settingsButton{ "Settings" };
    juce::TextButton calibrateButton{ "Calibrate Area" }; // Add this line
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

    // Inside GestureInstrumentAudioProcessorEditor class
    CalibrationOverlay calibrationOverlay;
    HUDComponents hud;
    VirtualCursor virtualCursor;

    bool isCalibrating = false;
    float calibrationTimer = 0.0f;
    const float calibrationDuration = 15.0f; // 15 seconds

    // Temporary storage for bounds being "discovered"
    float tempMinX = 1000.0f, tempMaxX = -1000.0f;
    float tempMinY = 1000.0f, tempMaxY = -1000.0f;

    void startCalibration();
    void stopCalibration(bool success);

    juce::TextButton editModeButton;
    bool isEditMode = false;

    // Virtual Cursor Variables
    juce::Point<int> virtualCursorPos;
    bool isCursorActive = false;
    bool isDraggingHUD = false;
    juce::Point<int> dragOffset;

    // Gesture Menu Toggle
    float menuGestureTimer = 0.0f;
    bool menuGestureFired = false;

    // Gesture Switch Configuration
    juce::ToggleButton enableGestureSwitchButton{ "Enable Gesture Switch" };
    juce::Slider gestureTimerSlider{ juce::Slider::LinearHorizontal, juce::Slider::TextBoxLeft };
    juce::ComboBox gestureTypeSelector;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GestureInstrumentAudioProcessorEditor)
};