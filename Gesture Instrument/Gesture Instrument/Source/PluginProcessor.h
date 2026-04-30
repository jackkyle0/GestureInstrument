#pragma once

#include <JuceHeader.h>
#include "Helpers/HandData.h"
#include "Helpers/LeapService.h"
#include "OSC/OscManager.h"
#include "MIDI/MidiManager.h"
#include "MIDI/GestureTarget.h"
#include "Helpers/MusicalRangeMode.h" 
#include "Helpers/LeapThread.h"

enum class OutputMode {
    OSC_Only,
    MIDI_Only
};

class GestureInstrumentAudioProcessor : public juce::AudioProcessor {
public:
    static inline bool isRunningInUnitTest = false;

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

    // Global state / Routing
    OutputMode currentOutputMode = OutputMode::MIDI_Only;
    std::atomic<bool> globalMute{ false };
    std::atomic<bool> isCalibrating{ false };
    std::atomic<bool> isWindowMaximized{ false };

    int currentInstrument = 90;
    bool instrumentChanged = true;

    // Musicial settings
    int rootNote = 0;
    int scaleType = 0;
    std::vector<int> customScaleIntervals{ 0, 2, 4, 7, 9 };

    MusicalRangeMode currentRangeMode = MusicalRangeMode::OctaveRange;
    int octaveRange = 2;
    int startNote = 48;
    int endNote = 72;
    bool invertNoteTrigger = false;
    bool showNoteNames = false;

    // Mpe settings
    bool isMpeEnabled = false;
    std::atomic<int> mpePitchBendAxis{ 1 }; // x-axis
    std::atomic<int> mpeTimbreAxis{ 2 };    // y-axis
    std::atomic<int> mpePressureAxis{ 3 };  // z-axis

	// Virtual Mouse settings
    std::atomic<bool> isVirtualMouse{ false };
    std::atomic<bool> isGestureToMouseEnabled{ true };
    int virtualMouseGestureType = 1; 
    float virtualMouseHoldTime = 1.5f;

    // Chord builder
    std::atomic<bool> chordEngineEnabled{ true };

    // Left Hand Chords
    std::atomic<bool> leftChordDegree1{ true }, leftChordDegree2{ false }, leftChordDegree3{ true }, leftChordDegree4{ false }, leftChordDegree5{ true }, leftChordDegree6{ false }, leftChordDegree7{ false };
    std::atomic<bool> leftRootI{ true }, leftRootII{ true }, leftRootIII{ true }, leftRootIV{ true }, leftRootV{ true }, leftRootVI{ true }, leftRootVII{ true };
    std::atomic<int> leftChordInversionMode{ 0 };
    std::atomic<bool> leftDropBass{ false };

    // Right Hand Chords
    std::atomic<bool> rightChordDegree1{ true }, rightChordDegree2{ false }, rightChordDegree3{ true }, rightChordDegree4{ false }, rightChordDegree5{ true }, rightChordDegree6{ false }, rightChordDegree7{ false };
    std::atomic<bool> rightRootI{ true }, rightRootII{ true }, rightRootIII{ true }, rightRootIV{ true }, rightRootV{ true }, rightRootVI{ true }, rightRootVII{ true };
    std::atomic<int> rightChordInversionMode{ 0 };
    std::atomic<bool> rightDropBass{ false };

    // HUD tracking arrays
    std::atomic<int> activeLeftNotes[8];
    std::atomic<int> activeRightNotes[8];

    // Gesture Mapping targets
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

    // Spatial and hardware thresholds
    HandData leftHand;
    HandData rightHand;
    bool isSensorConnected = false;
    float sensitivityLevel = 1.0f;

    std::atomic<bool> enableSplitXAxis{ false };
    bool showFloorShadow = true;
    bool showWallShadow = true;

    float minWidthThreshold = -200.0f;
    float maxWidthThreshold = 200.0f;
    float minHeightThreshold = 150.0f;
    float maxHeightThreshold = 450.0f;
    float minDepthThreshold = -150.0f;
    float maxDepthThreshold = 150.0f;

    std::atomic<float> wristMultiplier{ 1.0f };
    std::atomic<float> grabMultiplier{ 1.0f };
    std::atomic<float> pinchMultiplier{ 1.0f };

    // Static parameters
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

    // Managers
    OscManager oscManager;
    MidiManager midiManager;

    std::unique_ptr<juce::XmlElement> createPresetXml();
    void loadPresetXml(juce::XmlElement* xml);

private:
    LeapThread leapThread;

    // Smoothing state
    bool wasMutedLastFrame = false;
    float savedPreMuteVolume = 0.8f;
    int lastOutputModeInt = -1;

    bool leftHandWasPresent = false;
    float smoothLeftX = 0.0f, smoothLeftY = 0.0f, smoothLeftZ = 0.0f;
    float smoothLeftRoll = 0.0f, smoothLeftGrab = 0.0f, smoothLeftPinch = 0.0f;

    bool rightHandWasPresent = false;
    float smoothRightX = 0.0f, smoothRightY = 0.0f, smoothRightZ = 0.0f;
    float smoothRightRoll = 0.0f, smoothRightGrab = 0.0f, smoothRightPinch = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GestureInstrumentAudioProcessor)
};