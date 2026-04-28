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
//#include "../Testing/Unit Tests/SensorTests.h"


enum class OutputMode {
    OSC_Only,
    MIDI_Only
};

class GestureInstrumentAudioProcessor : public juce::AudioProcessor {
public:

    static inline bool isRunningInUnitTest = false; // <-- Add this!

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

    // --- State Variables ---
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

    // --- Routing Targets ---
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

    // --- Hand Data & Physical Thresholds ---
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

    // --- Static Dial Values ---
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
    float staticDelay = 0.0f;
    float staticDistortion = 0.0f;

    int waveformGestureSource = 0;

    // --- Live Variables ---
    std::atomic<float> liveVolume{ -1.0f };
    std::atomic<float> liveCutoff{ -1.0f };
    std::atomic<float> liveResonance{ -1.0f };
    std::atomic<float> liveReverb{ -1.0f };
    std::atomic<float> liveWaveform{ -1.0f };
    std::atomic<float> liveSustain{ -1.0f };
    std::atomic<float> livePortamento{ -1.0f };

    float tempMinY = 1000.0f;
    float tempMaxY = 0.0f;
    float calibrationProgress = 0.0f;

    std::unique_ptr<juce::XmlElement> createPresetXml();
    void loadPresetXml(juce::XmlElement* xml);

    OscManager oscManager;
    MidiManager midiManager;
    std::atomic<double> timeLastFrameReceived{ 0.0 };
    std::atomic<bool> isAdaptiveEnabled{ false };

    std::atomic<bool> globalMute{ false };
    std::atomic<bool> isVirtualMouse{ false };

    std::atomic<bool> isGestureToMouseEnabled{ true }; // Tracks SETTING

    int virtualMouseGestureType = 1; // 1 = Both, 2 = Right, 3 = Left
    float virtualMouseHoldTime = 1.5f;

    std::atomic<bool> isCalibrating{ false };

    std::vector<int> customScaleIntervals{ 0, 2, 4, 7, 9 };

    bool showFloorShadow = true;
    bool showWallShadow = true;

    std::atomic<bool> enableSplitXAxis{ false };

    std::atomic<int> mpePitchBendAxis{ 1 }; // Default: X-Axis
    std::atomic<int> mpeTimbreAxis{ 2 };    // Default: Y-Axis
    std::atomic<int> mpePressureAxis{ 3 };  // Default: Z-Axis


    // --- LEFT HAND CHORD ENGINE ---
    std::atomic<bool> leftChordDegree1{ true }, leftChordDegree2{ false }, leftChordDegree3{ true }, leftChordDegree4{ false }, leftChordDegree5{ true }, leftChordDegree6{ false }, leftChordDegree7{ false };
    std::atomic<bool> leftRootI{ true }, leftRootII{ true }, leftRootIII{ true }, leftRootIV{ true }, leftRootV{ true }, leftRootVI{ true }, leftRootVII{ true };
    std::atomic<int> leftChordInversionMode{ 0 };
    std::atomic<bool> leftDropBass{ false };

    // --- RIGHT HAND CHORD ENGINE ---
    std::atomic<bool> rightChordDegree1{ true }, rightChordDegree2{ false }, rightChordDegree3{ true }, rightChordDegree4{ false }, rightChordDegree5{ true }, rightChordDegree6{ false }, rightChordDegree7{ false };
    std::atomic<bool> rightRootI{ true }, rightRootII{ true }, rightRootIII{ true }, rightRootIV{ true }, rightRootV{ true }, rightRootVI{ true }, rightRootVII{ true };
    std::atomic<int> rightChordInversionMode{ 0 };
    std::atomic<bool> rightDropBass{ false };

    // --- CHORD ENGINE MASTER & HUD TRACKING ---
    std::atomic<bool> chordEngineEnabled{ true };
    std::atomic<int> activeLeftNotes[8];  // Stores up to 8 notes for the HUD
    std::atomic<int> activeRightNotes[8]; // Stores up to 8 notes for the HUD

    std::atomic<bool> isWindowMaximized{ false };





private:
    LeapThread leapThread;

    // --- Global Smoothing Trackers ---
    bool wasMutedLastFrame = false;
    float savedPreMuteVolume = 0.8f;

    bool leftHandWasPresent = false;
    float smoothLeftX = 0.0f, smoothLeftY = 0.0f, smoothLeftZ = 0.0f;
    float smoothLeftRoll = 0.0f, smoothLeftGrab = 0.0f, smoothLeftPinch = 0.0f;

    bool rightHandWasPresent = false;
    float smoothRightX = 0.0f, smoothRightY = 0.0f, smoothRightZ = 0.0f;

    float smoothRightRoll = 0.0f, smoothRightGrab = 0.0f, smoothRightPinch = 0.0f;

    int lastOutputModeInt = -1;
    bool invertFirstOctave = false;


    // --- Biometric Trackers ---
    BiometricSpatialTracker xTracker{ 72000, -350.0f, 350.0f, 150.0f };
    BiometricSpatialTracker yTracker{ 72000, 50.0f, 500.0f, 100.0f };
    BiometricSpatialTracker zTracker{ 72000, -225.0f, 225.0f, 100.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GestureInstrumentAudioProcessor)
};