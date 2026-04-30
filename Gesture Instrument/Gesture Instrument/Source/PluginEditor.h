#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "UI/SettingsComponent.h"
#include "UI/HUDComponents.h"
#include "UI/VirtualCursor.h"
#include "UI/StaticDialsComponent.h"
#include "UI/ChordBuilder.h"

struct Point3D {
    float x, y, z;
};

struct CustomScaleEditor : public juce::Component {
    juce::TextButton noteButtons[12];
    std::function<void(std::vector<int>)> onScaleChanged;

    CustomScaleEditor() {
        juce::StringArray labels = { "1", "b2", "2", "b3", "3", "4", "b5", "5", "b6", "6", "b7", "7" };

        for (int i = 0; i < 12; ++i) {
            addAndMakeVisible(noteButtons[i]);
            noteButtons[i].setButtonText(labels[i]);
            noteButtons[i].setClickingTogglesState(true);
            noteButtons[i].setColour(juce::TextButton::buttonOnColourId, juce::Colours::orange);
            noteButtons[i].setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);

            noteButtons[i].onClick = [this] { updateScale(); };
        }
    }

    void setCustomScale(const std::vector<int>& intervals) {
        for (int i = 0; i < 12; ++i) noteButtons[i].setToggleState(false, juce::dontSendNotification);
        for (int interval : intervals) {
            if (interval >= 0 && interval < 12) {
                noteButtons[interval].setToggleState(true, juce::dontSendNotification);
            }
        }
    }

    void updateScale() {
        std::vector<int> newScale;
        for (int i = 0; i < 12; ++i) {
            if (noteButtons[i].getToggleState()) newScale.push_back(i);
        }

        if (newScale.empty()) {
            newScale.push_back(0);
            noteButtons[0].setToggleState(true, juce::dontSendNotification);
        }

        if (onScaleChanged) onScaleChanged(newScale);
    }

    void resized() override {
        auto bounds = getLocalBounds();
        int w = bounds.getWidth() / 12;
        for (int i = 0; i < 12; ++i) {
            noteButtons[i].setBounds(bounds.removeFromLeft(w).reduced(1));
        }
    }
};

class GestureInstrumentAudioProcessorEditor : public juce::AudioProcessorEditor, public juce::Timer {
public:
    GestureInstrumentAudioProcessorEditor(GestureInstrumentAudioProcessor&);
    ~GestureInstrumentAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;
    bool keyPressed(const juce::KeyPress& key) override;

    class CalibrationOverlay : public juce::Component {
    public:
        CalibrationOverlay() {
            addAndMakeVisible(cancelButton);
            cancelButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkred);
            cancelButton.onClick = [this] { if (onCancel) onCancel(); };
        }

        void setProgress(float p);
        void paint(juce::Graphics& g) override;
        void resized() override {
            auto bounds = getLocalBounds();
            cancelButton.setBounds(bounds.getCentreX() - 60, bounds.getBottom() - 100, 120, 35);
        }

        std::function<void()> onCancel;
    private:
        float progress = 0.0f;
        juce::TextButton cancelButton{ "Cancel (Esc)" };
    };

private:
    GestureInstrumentAudioProcessor& audioProcessor;

    // Pages and overlays
    juce::OpenGLContext openGLContext;
    SettingsComponent settingsPage;
    juce::Viewport settingsViewport;
    StaticDialsComponent staticDialsPage;
    ChordBuilder chordBuilderPage;
    HUDComponents hud;
    VirtualCursor virtualCursor;
    CalibrationOverlay calibrationOverlay;
    CustomScaleEditor customScaleUI;

    // Buttons and labels
    juce::TextButton settingsButton{ "Settings" };
    juce::TextButton calibrateButton{ "Calibrate Area" };
    juce::TextButton editModeButton{ "Virtual Mouse" };
    juce::TextButton staticDialsButton{ "Dials" };
    juce::TextButton chordBuilderButton{ "Chord Builder" };
    juce::TextButton maximizeButton{ "Maximize" };
    juce::ToggleButton showNoteNamesButton{ "Show Note Names" };
    juce::ToggleButton muteButton{ "Mute (Space)" };

    juce::Label connectionStatusLabel;
    juce::Label scaleLabel;
    juce::Label keyLabel;
    juce::Label baseOctaveLabel;
    juce::Label octaveLabel;

    // Selectors
    juce::ComboBox rootSelector;
    juce::ComboBox scaleSelector;
    juce::ComboBox rangeModeSelector;
    juce::ComboBox octaveSelector;
    juce::ComboBox startNoteSelector;
    juce::ComboBox endNoteSelector;
    juce::ComboBox baseOctaveSelector;

    // Sliders
    LabeledSlider xMinControl;
    LabeledSlider xMaxControl;
    LabeledSlider yMinControl;
    LabeledSlider yMaxControl;
    LabeledSlider zMinControl;
    LabeledSlider zMaxControl;

    // State tracking
    bool isEditMode = false;
    bool isMaximized = false;
    bool menuGestureFired = false;
    float menuGestureTimer = 0.0f;
    int lastKnownOutputMode = -1;

    juce::Rectangle<int> previousSize{ 1500, 700 };
    juce::Point<int> previousPosition;

    // Calibration
    bool isCalibrating = false;
    float calibrationTimer = 0.0f;
    const float calibrationDuration = 15.0f;
    float tempMinX = 1000.0f, tempMaxX = -1000.0f;
    float tempMinY = 1000.0f, tempMaxY = -1000.0f;
    float tempMinZ = 1000.0f, tempMaxZ = -1000.0f;

    void startCalibration();
    void stopCalibration(bool success);

    // 3D graphics
    const float fov = 350.0f;
    const float camDist = 600.0f;
    juce::Point<float> centerScreen;

    void updateScaleDropdown();
    void updateConnectionStatus();
    juce::Point<float> projectPoint(Point3D p);
    void draw3DGrid(juce::Graphics& g);
    void draw3DHand(juce::Graphics& g, const HandData& hand, juce::Colour baseColour);
    void drawCalibrationBox3D(juce::Graphics& g);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GestureInstrumentAudioProcessorEditor)
};