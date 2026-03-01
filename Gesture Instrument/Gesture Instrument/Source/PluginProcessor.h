#pragma once

#include <JuceHeader.h>
#include "Helpers/HandData.h"
#include "Helpers/LeapService.h"
#include "OSC/OscManager.h"
#include "MIDI/MidiManager.h"
#include "MIDI/GestureTarget.h"
#include "Helpers/MusicalRangeMode.h" 
#include "Helpers/LeapThread.h"
#include "Helpers/BiometricSpatialTracker.h"

enum class OutputMode {
    OSC_Only,
    MIDI_Only
};

class GestureInstrumentAudioProcessor : public juce::AudioProcessor {
public:
    GestureInstrumentAudioProcessor();
    ~GestureInstrumentAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    void updateOscSettings(juce::String newIp, int newPort) {
        oscManager.connectSender(newIp, newPort);
    }

    OutputMode currentOutputMode = OutputMode::MIDI_Only;
    bool isMpeEnabled = false; 
    std::atomic<bool> muteOutput{ false };

    MusicalRangeMode currentRangeMode = MusicalRangeMode::OctaveRange;
    int octaveRange = 2;
    int startNote = 48; 
    int endNote = 72;   

    bool showNoteNames = false;
    bool invertNoteTrigger = false;
    int rootNote = 0;
    int scaleType = 0;
    int currentScale = 1;
    int targetScale = 1;
    int currentInstrument = 90;
    bool instrumentChanged = true;

    int leftHandTargetCC = 1;
    int rightHandTargetCC = 7;

    GestureTarget leftXTarget = GestureTarget::None;
    GestureTarget leftYTarget = GestureTarget::Pitch;
    GestureTarget leftZTarget = GestureTarget::None;
    GestureTarget leftRollTarget = GestureTarget::None;
    GestureTarget leftGrabTarget = GestureTarget::None;
    GestureTarget leftPinchTarget = GestureTarget::None;
    GestureTarget leftThumbTarget = GestureTarget::None;
    GestureTarget leftIndexTarget = GestureTarget::None;
    GestureTarget leftMiddleTarget = GestureTarget::None;
    GestureTarget leftRingTarget = GestureTarget::None;
    GestureTarget leftPinkyTarget = GestureTarget::None;

    GestureTarget rightXTarget = GestureTarget::None;
    GestureTarget rightYTarget = GestureTarget::Pitch;
    GestureTarget rightZTarget = GestureTarget::None;
    GestureTarget rightRollTarget = GestureTarget::None;
    GestureTarget rightGrabTarget = GestureTarget::None;
    GestureTarget rightPinchTarget = GestureTarget::Modulation;
    GestureTarget rightThumbTarget = GestureTarget::None;
    GestureTarget rightIndexTarget = GestureTarget::Vibrato;
    GestureTarget rightMiddleTarget = GestureTarget::None;
    GestureTarget rightRingTarget = GestureTarget::None;
    GestureTarget rightPinkyTarget = GestureTarget::None;

    HandData leftHand;
    HandData rightHand;
    bool isSensorConnected = false;

    float sensitivityLevel = 1.0f;

    float minWidthThreshold = -200.0f;
    float maxWidthThreshold = 200.0f;
    float minHeightThreshold = 150.0f;
    float maxHeightThreshold = 450.0f;
    float minDepthThreshold = -150.0f;
    float maxDepthThreshold = 150.0f;

    float staticVolume = 0.8f;
    float staticPan = 0.5f;
    float staticModulation = 0.0f;
    float staticExpression = 1.0f;

    float staticCutoff = 1.0f;
    float staticResonance = 0.0f;
    float staticAttack = 0.1f;
    float staticRelease = 0.1f;

    float staticReverb = 0.1f;
    float staticChorus = 0.0f;
    float staticVibrato = 0.0f;
    float staticWaveform = 0.0f;
    int waveformGestureSource = 0;

    std::atomic<float> liveVolume{ -1.0f };
    std::atomic<float> liveCutoff{ -1.0f };
    std::atomic<float> liveResonance{ -1.0f };
    std::atomic<float> liveReverb{ -1.0f };
    std::atomic<float> liveWaveform{ -1.0f };

    bool isCalibrating = false;
    float tempMinY = 1000.0f;
    float tempMaxY = 0.0f;
    float calibrationProgress = 0.0f;

    std::unique_ptr<juce::XmlElement> createPresetXml();
    void loadPresetXml(juce::XmlElement* xml);

    OscManager oscManager;
    MidiManager midiManager;


private:
    LeapThread leapThread; 

    BiometricSpatialTracker xTracker{ 72000, -350.0f, 350.0f, 150.0f };
    BiometricSpatialTracker yTracker{ 72000, 50.0f, 500.0f, 100.0f };
    BiometricSpatialTracker zTracker{ 72000, -225.0f, 225.0f, 100.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GestureInstrumentAudioProcessor)
};