#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "UI/SettingsComponent.h"
#include "UI/HUDComponents.h"
#include "UI/VirtualCursor.h"
#include "UI/StaticDialsComponent.h"
#include "UI/WaveformComponent.h"
#include "UI/ChordBuilder.h"


struct Point3D {
    float x, y, z;
};

struct CustomScaleEditor : public juce::Component {
    juce::TextButton noteButtons[12];
    std::function<void(std::vector<int>)> onScaleChanged;

    CustomScaleEditor() {
        // Labels for the 12 intervals (Root, minor 2nd, Major 2nd, etc.)
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

        // Failsafe: A scale must have at least 1 note, so we force the Root note if they turn everything off
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

class GestureInstrumentAudioProcessorEditor : public juce::AudioProcessorEditor,
    public juce::Timer {
public:
    GestureInstrumentAudioProcessorEditor(GestureInstrumentAudioProcessor&);
    ~GestureInstrumentAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;



    class CalibrationOverlay : public juce::Component {
    public:
        CalibrationOverlay() {
            addAndMakeVisible(cancelButton);
            cancelButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkred);
            cancelButton.onClick = [this] {
                if (onCancel) onCancel();
                };
        }

        void setProgress(float p);
        void paint(juce::Graphics& g) override;
        void resized() override {
            // Place the cancel button nicely at the bottom center of the screen
            auto bounds = getLocalBounds();
            cancelButton.setBounds(bounds.getCentreX() - 60, bounds.getBottom() - 100, 120, 35);
        }

        std::function<void()> onCancel;
    private:
        float progress = 0.0f;
        juce::TextButton cancelButton{ "Cancel (Esc)" };
    };

    bool keyPressed(const juce::KeyPress& key) override;


private:
    GestureInstrumentAudioProcessor& audioProcessor;

    StaticDialsComponent staticDialsPage;
    juce::TextButton staticDialsButton{ "Dials" };

    // Overlays
    juce::OpenGLContext openGLContext;
    SettingsComponent settingsPage;
    juce::Viewport settingsViewport;
    HUDComponents hud;
    VirtualCursor virtualCursor;
    CalibrationOverlay calibrationOverlay;

    // Buttons and labels
    juce::TextButton settingsButton{ "Settings" };
    juce::TextButton calibrateButton{ "Calibrate Area" };
    juce::TextButton editModeButton{ "Virtual Mouse" };
    juce::ToggleButton showNoteNamesButton{ "Show Note Names" };
    juce::ToggleButton muteButton{ "Mute (Space)" };
    juce::Label connectionStatusLabel;

    // Selectors
    juce::ComboBox rootSelector;
    juce::ComboBox scaleSelector;
    juce::Label scaleLabel;
    juce::Label keyLabel;   // <-- Add this right next to your scaleLabel

    juce::Label baseOctaveLabel;


    // Musical Range Selectors
    juce::ComboBox rangeModeSelector;
    juce::ComboBox octaveSelector;
    juce::ComboBox startNoteSelector;
    juce::ComboBox endNoteSelector;
    juce::Label octaveLabel;

    juce::ComboBox baseOctaveSelector;



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

    //std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> muteAttachment;

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

    CustomScaleEditor customScaleUI;

    juce::TextButton maximizeButton{ "Maximize" };
    bool isMaximized = false;
    juce::Rectangle<int> previousSize{ 1500, 700 }; // Default starting size
    juce::Point<int> previousPosition;

    ChordBuilder chordBuilderPage;
    juce::TextButton chordBuilderButton{ "Chord Builder" };


    //WaveformComponent waveformComponent;



    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GestureInstrumentAudioProcessorEditor)
};