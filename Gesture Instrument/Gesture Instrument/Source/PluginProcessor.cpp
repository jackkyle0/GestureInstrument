#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "../Testing/Unit Tests/ScaleQuantiserTests.h"
#include "../Testing/Unit Tests/PluginProcessorTests.h"
#include "../Testing/Unit Tests/MidiManagerTests.h"
#include "../Testing/Unit Tests/OscManagerTests.h"
#include "../Testing/Unit Tests/LeapServiceTests.h"

GestureInstrumentAudioProcessor::GestureInstrumentAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    )
#endif
{
    oscManager.connectSender("127.0.0.1", 9000);

    // Run Units Tests
    if (!isRunningInUnitTest) {
        leapThread.startThread(juce::Thread::Priority::high);
    }

    for (int i = 0; i < 8; ++i) {
        activeLeftNotes[i].store(-1);
        activeRightNotes[i].store(-1);
    }

    static bool testsHaveRun = false;
    if (!testsHaveRun) {
        testsHaveRun = true;
        juce::UnitTestRunner testRunner;
        testRunner.runAllTests();
    }
}

GestureInstrumentAudioProcessor::~GestureInstrumentAudioProcessor() {}

const juce::String GestureInstrumentAudioProcessor::getName() const { return JucePlugin_Name; }

bool GestureInstrumentAudioProcessor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool GestureInstrumentAudioProcessor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool GestureInstrumentAudioProcessor::isMidiEffect() const {
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double GestureInstrumentAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int GestureInstrumentAudioProcessor::getNumPrograms() { return 1; }
int GestureInstrumentAudioProcessor::getCurrentProgram() { return 0; }
void GestureInstrumentAudioProcessor::setCurrentProgram(int index) {}
const juce::String GestureInstrumentAudioProcessor::getProgramName(int index) { return {}; }
void GestureInstrumentAudioProcessor::changeProgramName(int index, const juce::String& newName) {}
void GestureInstrumentAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {}
void GestureInstrumentAudioProcessor::releaseResources() {}

#ifndef JucePlugin_PreferredChannelConfigurations
bool GestureInstrumentAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif
    return true;
#endif
}
#endif

void GestureInstrumentAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    buffer.clear();

    //Get latest sensor data
    leapThread.getLatestData(leftHand, rightHand, isSensorConnected);

    bool didLeftHandJustDisconnect = (!leftHand.isPresent && leftHandWasPresent);
    bool didRightHandJustDisconnect = (!rightHand.isPresent && rightHandWasPresent);

    // output mode switches
    bool modeChanged = false;
    if (static_cast<int>(currentOutputMode) != lastOutputModeInt) {
        modeChanged = true;
        lastOutputModeInt = static_cast<int>(currentOutputMode);
    }

    // Send panic messages is hands disconnected
    if (didLeftHandJustDisconnect || modeChanged) {
        oscManager.panicLeft();
        midiManager.panicLeft(midiMessages);
    }
    if (didRightHandJustDisconnect || modeChanged) {
        oscManager.panicRight();
        midiManager.panicRight(midiMessages);
    }

    // Apply spatital smoothing
    float smoothingFactor = 0.15f;

    auto applySmoothing = [&](HandData& hand, bool& wasPresent, float& sX, float& sY, float& sZ, float& sRoll, float& sGrab, float& sPinch) {
        if (hand.isPresent) {
            if (!wasPresent) {
                sX = hand.currentHandPositionX; sY = hand.currentHandPositionY; sZ = hand.currentHandPositionZ;
                sRoll = hand.currentWristRotation; sGrab = hand.grabStrength; sPinch = hand.pinchStrength;
            }

            sX += (hand.currentHandPositionX - sX) * smoothingFactor;
            sY += (hand.currentHandPositionY - sY) * smoothingFactor;
            sZ += (hand.currentHandPositionZ - sZ) * smoothingFactor;
            sRoll += (hand.currentWristRotation - sRoll) * smoothingFactor;
            sGrab += (hand.grabStrength - sGrab) * smoothingFactor;
            sPinch += (hand.pinchStrength - sPinch) * smoothingFactor;

            hand.currentHandPositionX = sX;
            hand.currentHandPositionY = sY;
            hand.currentHandPositionZ = sZ;
            hand.currentWristRotation = sRoll;
            hand.grabStrength = sGrab;
            hand.pinchStrength = sPinch;
        }
        wasPresent = hand.isPresent;
        };

    applySmoothing(leftHand, leftHandWasPresent, smoothLeftX, smoothLeftY, smoothLeftZ, smoothLeftRoll, smoothLeftGrab, smoothLeftPinch);
    applySmoothing(rightHand, rightHandWasPresent, smoothRightX, smoothRightY, smoothRightZ, smoothRightRoll, smoothRightGrab, smoothRightPinch);

    // Globabl muting
    bool isCurrentlyMuted = globalMute.load() || isCalibrating.load() || isVirtualMouse.load();
    if (isCurrentlyMuted) {
        buffer.clear();
        midiMessages.clear();
        if (!wasMutedLastFrame) {
            savedPreMuteVolume = (currentOutputMode == OutputMode::OSC_Only) ? oscManager.liveVolume.load() : midiManager.liveVolume.load();
            if (savedPreMuteVolume < 0.0f) savedPreMuteVolume = staticVolume;

            if (currentOutputMode == OutputMode::OSC_Only) {
                juce::OSCMessage volMsg("/global/volume");
                volMsg.addFloat32(0.0f);
                oscManager.sender.send(volMsg);
            }
            else {
                midiMessages.addEvent(juce::MidiMessage::controllerEvent(2, 123, 0), 0);
                midiMessages.addEvent(juce::MidiMessage::controllerEvent(2, 7, 0), 0);
                midiMessages.addEvent(juce::MidiMessage::controllerEvent(3, 123, 0), 0);
                midiMessages.addEvent(juce::MidiMessage::controllerEvent(3, 7, 0), 0);
            }
            wasMutedLastFrame = true;
        }
        return;
    }

    if (wasMutedLastFrame) {
        if (currentOutputMode == OutputMode::OSC_Only) {
            juce::OSCMessage volMsg("/global/volume");
            volMsg.addFloat32(savedPreMuteVolume);
            oscManager.sender.send(volMsg);
        }
        else {
            int vol7bit = juce::jlimit(0, 127, (int)(savedPreMuteVolume * 127.0f));
            midiMessages.addEvent(juce::MidiMessage::controllerEvent(2, 7, vol7bit), 0);
            midiMessages.addEvent(juce::MidiMessage::controllerEvent(3, 7, vol7bit), 0);
        }
        wasMutedLastFrame = false;
    }

    // Reset UI trackers
    auto resetTrackers = [](auto& manager) {
        manager.liveVolume.store(-1.0f); manager.livePan.store(-1.0f); manager.liveModulation.store(-1.0f); manager.liveExpression.store(-1.0f);
        manager.liveCutoff.store(-1.0f); manager.liveResonance.store(-1.0f); manager.liveAttack.store(-1.0f); manager.liveRelease.store(-1.0f);
        manager.liveReverb.store(-1.0f); manager.liveChorus.store(-1.0f); manager.liveVibrato.store(-1.0f); manager.liveWaveform.store(-1.0f);
        };

    resetTrackers(oscManager);
    resetTrackers(midiManager);
    oscManager.liveDelay.store(-1.0f);
    oscManager.liveDistortion.store(-1.0f);
    midiManager.liveSustain.store(-1.0f);
    midiManager.livePortamento.store(-1.0f);

    if (instrumentChanged) {
        midiManager.sendProgramChange(midiMessages, currentInstrument);
        instrumentChanged = false;
    }

    // Process core logic
    oscManager.sendRawData(leftHand, rightHand);
    oscManager.sendMidiData(midiMessages);
    midiManager.updateCustomScale(customScaleIntervals);

    std::array<bool, 7> leftDiatonicDegrees = { leftChordDegree1.load(), leftChordDegree2.load(), leftChordDegree3.load(), leftChordDegree4.load(), leftChordDegree5.load(), leftChordDegree6.load(), leftChordDegree7.load() };
    std::array<bool, 7> rightDiatonicDegrees = { rightChordDegree1.load(), rightChordDegree2.load(), rightChordDegree3.load(), rightChordDegree4.load(), rightChordDegree5.load(), rightChordDegree6.load(), rightChordDegree7.load() };

    std::array<bool, 7> leftAllowedRoots = { leftRootI.load(), leftRootII.load(), leftRootIII.load(), leftRootIV.load(), leftRootV.load(), leftRootVI.load(), leftRootVII.load() };
    std::array<bool, 7> rightAllowedRoots = { rightRootI.load(), rightRootII.load(), rightRootIII.load(), rightRootIV.load(), rightRootV.load(), rightRootVI.load(), rightRootVII.load() };

    if (currentOutputMode == OutputMode::OSC_Only) {
        oscManager.processHandData(
            leftHand, rightHand,
            sensitivityLevel,
            minWidthThreshold, maxWidthThreshold, minHeightThreshold, maxHeightThreshold, minDepthThreshold, maxDepthThreshold,
            wristMultiplier.load(), grabMultiplier.load(), pinchMultiplier.load(),
            leftXTarget, leftYTarget, leftZTarget, leftRollTarget, leftGrabTarget, leftPinchTarget,
            leftThumbTarget, leftIndexTarget, leftMiddleTarget, leftRingTarget, leftPinkyTarget,
            rightXTarget, rightYTarget, rightZTarget, rightRollTarget, rightGrabTarget, rightPinchTarget,
            rightThumbTarget, rightIndexTarget, rightMiddleTarget, rightRingTarget, rightPinkyTarget,
            rootNote, scaleType, octaveRange, currentRangeMode, startNote, endNote, enableSplitXAxis.load(), 
            activeLeftNotes, activeRightNotes
        );
    }
    else if (currentOutputMode == OutputMode::MIDI_Only) {
        midiManager.processHandData(
            midiMessages, leftHand, rightHand,
            sensitivityLevel,
            minWidthThreshold, maxWidthThreshold, minHeightThreshold, maxHeightThreshold, minDepthThreshold, maxDepthThreshold,
            wristMultiplier.load(), grabMultiplier.load(), pinchMultiplier.load(),
            leftXTarget, leftYTarget, leftZTarget, leftRollTarget, leftGrabTarget, leftPinchTarget,
            leftThumbTarget, leftIndexTarget, leftMiddleTarget, leftRingTarget, leftPinkyTarget,
            rightXTarget, rightYTarget, rightZTarget, rightRollTarget, rightGrabTarget, rightPinchTarget,
            rightThumbTarget, rightIndexTarget, rightMiddleTarget, rightRingTarget, rightPinkyTarget,
            rootNote, scaleType, octaveRange, invertNoteTrigger,
            currentRangeMode, startNote, endNote, isMpeEnabled, enableSplitXAxis.load(),
            mpePitchBendAxis.load(), mpeTimbreAxis.load(), mpePressureAxis.load(),
            leftDiatonicDegrees, leftAllowedRoots, leftChordInversionMode.load(), leftDropBass.load(),
            rightDiatonicDegrees, rightAllowedRoots, rightChordInversionMode.load(), rightDropBass.load(),
            chordEngineEnabled.load(),
            activeLeftNotes, activeRightNotes
        );
    }
}

bool GestureInstrumentAudioProcessor::hasEditor() const { return true; }
juce::AudioProcessorEditor* GestureInstrumentAudioProcessor::createEditor() { return new GestureInstrumentAudioProcessorEditor(*this); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new GestureInstrumentAudioProcessor(); }

void GestureInstrumentAudioProcessor::getStateInformation(juce::MemoryBlock& destData) {
    auto xml = createPresetXml();
    copyXmlToBinary(*xml, destData);
}

void GestureInstrumentAudioProcessor::setStateInformation(const void* data, int sizeInBytes) {
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    loadPresetXml(xmlState.get());
}

std::unique_ptr<juce::XmlElement> GestureInstrumentAudioProcessor::createPresetXml() {
    auto xml = std::make_unique<juce::XmlElement>("GestureInstrumentState");

    xml->setAttribute("outputMode", static_cast<int>(currentOutputMode));
    xml->setAttribute("currentInstrument", currentInstrument);

    xml->setAttribute("rootNote", rootNote);
    xml->setAttribute("scaleType", scaleType);
    xml->setAttribute("currentRangeMode", static_cast<int>(currentRangeMode));
    xml->setAttribute("startNote", startNote);
    xml->setAttribute("endNote", endNote);
    xml->setAttribute("octaveRange", octaveRange);
    xml->setAttribute("invertNoteTrigger", invertNoteTrigger);
    xml->setAttribute("showNoteNames", showNoteNames);
    xml->setAttribute("globalMute", globalMute.load());
    xml->setAttribute("isWindowMaximized", isWindowMaximized.load());
    xml->setAttribute("showFloorShadow", showFloorShadow);
    xml->setAttribute("showWallShadow", showWallShadow);

    xml->setAttribute("isGestureToMouseEnabled", isGestureToMouseEnabled.load());
    xml->setAttribute("virtualMouseGestureType", virtualMouseGestureType);
    xml->setAttribute("virtualMouseHoldTime", virtualMouseHoldTime);

    xml->setAttribute("leftXTarget", static_cast<int>(leftXTarget));
    xml->setAttribute("leftYTarget", static_cast<int>(leftYTarget));
    xml->setAttribute("leftZTarget", static_cast<int>(leftZTarget));
    xml->setAttribute("leftRollTarget", static_cast<int>(leftRollTarget));
    xml->setAttribute("leftGrabTarget", static_cast<int>(leftGrabTarget));
    xml->setAttribute("leftPinchTarget", static_cast<int>(leftPinchTarget));
    xml->setAttribute("leftThumbTarget", static_cast<int>(leftThumbTarget));
    xml->setAttribute("leftIndexTarget", static_cast<int>(leftIndexTarget));
    xml->setAttribute("leftMiddleTarget", static_cast<int>(leftMiddleTarget));
    xml->setAttribute("leftRingTarget", static_cast<int>(leftRingTarget));
    xml->setAttribute("leftPinkyTarget", static_cast<int>(leftPinkyTarget));

    xml->setAttribute("rightXTarget", static_cast<int>(rightXTarget));
    xml->setAttribute("rightYTarget", static_cast<int>(rightYTarget));
    xml->setAttribute("rightZTarget", static_cast<int>(rightZTarget));
    xml->setAttribute("rightRollTarget", static_cast<int>(rightRollTarget));
    xml->setAttribute("rightGrabTarget", static_cast<int>(rightGrabTarget));
    xml->setAttribute("rightPinchTarget", static_cast<int>(rightPinchTarget));
    xml->setAttribute("rightThumbTarget", static_cast<int>(rightThumbTarget));
    xml->setAttribute("rightIndexTarget", static_cast<int>(rightIndexTarget));
    xml->setAttribute("rightMiddleTarget", static_cast<int>(rightMiddleTarget));
    xml->setAttribute("rightRingTarget", static_cast<int>(rightRingTarget));
    xml->setAttribute("rightPinkyTarget", static_cast<int>(rightPinkyTarget));

    xml->setAttribute("minWidthThreshold", minWidthThreshold);
    xml->setAttribute("maxWidthThreshold", maxWidthThreshold);
    xml->setAttribute("minHeightThreshold", minHeightThreshold);
    xml->setAttribute("maxHeightThreshold", maxHeightThreshold);
    xml->setAttribute("minDepthThreshold", minDepthThreshold);
    xml->setAttribute("maxDepthThreshold", maxDepthThreshold);

    xml->setAttribute("wristMult", wristMultiplier.load());
    xml->setAttribute("grabMult", grabMultiplier.load());
    xml->setAttribute("pinchMult", pinchMultiplier.load());


    xml->setAttribute("staticVolume", staticVolume);
    xml->setAttribute("staticPan", staticPan);
    xml->setAttribute("staticModulation", staticModulation);
    xml->setAttribute("staticExpression", staticExpression);
    xml->setAttribute("staticCutoff", staticCutoff);
    xml->setAttribute("staticResonance", staticResonance);
    xml->setAttribute("staticAttack", staticAttack);
    xml->setAttribute("staticRelease", staticRelease);
    xml->setAttribute("staticReverb", staticReverb);
    xml->setAttribute("staticChorus", staticChorus);
    xml->setAttribute("staticVibrato", staticVibrato);
    xml->setAttribute("staticWaveform", staticWaveform);
    xml->setAttribute("staticDelay", staticDelay);
    xml->setAttribute("staticDistortion", staticDistortion);

    juce::String scaleStr;
    for (int i = 0; i < customScaleIntervals.size(); ++i) {
        scaleStr += juce::String(customScaleIntervals[i]);
        if (i < customScaleIntervals.size() - 1) scaleStr += ",";
    }

    xml->setAttribute("isMpeEnabled", isMpeEnabled);
    xml->setAttribute("customScaleIntervals", scaleStr);
    xml->setAttribute("enableSplitXAxis", enableSplitXAxis.load());

    xml->setAttribute("mpePitchBendAxis", mpePitchBendAxis.load());
    xml->setAttribute("mpeTimbreAxis", mpeTimbreAxis.load());
    xml->setAttribute("mpePressureAxis", mpePressureAxis.load());

    // Left hand chord data
    xml->setAttribute("l_deg1", leftChordDegree1.load()); xml->setAttribute("l_deg2", leftChordDegree2.load());
    xml->setAttribute("l_deg3", leftChordDegree3.load()); xml->setAttribute("l_deg4", leftChordDegree4.load());
    xml->setAttribute("l_deg5", leftChordDegree5.load()); xml->setAttribute("l_deg6", leftChordDegree6.load());
    xml->setAttribute("l_deg7", leftChordDegree7.load());

    xml->setAttribute("l_r1", leftRootI.load()); xml->setAttribute("l_r2", leftRootII.load());
    xml->setAttribute("l_r3", leftRootIII.load()); xml->setAttribute("l_r4", leftRootIV.load());
    xml->setAttribute("l_r5", leftRootV.load()); xml->setAttribute("l_r6", leftRootVI.load());
    xml->setAttribute("l_r7", leftRootVII.load());

    xml->setAttribute("l_invMode", leftChordInversionMode.load());
    xml->setAttribute("l_dropB", leftDropBass.load());

    // right hand chord data
    xml->setAttribute("r_deg1", rightChordDegree1.load()); xml->setAttribute("r_deg2", rightChordDegree2.load());
    xml->setAttribute("r_deg3", rightChordDegree3.load()); xml->setAttribute("r_deg4", rightChordDegree4.load());
    xml->setAttribute("r_deg5", rightChordDegree5.load()); xml->setAttribute("r_deg6", rightChordDegree6.load());
    xml->setAttribute("r_deg7", rightChordDegree7.load());

    xml->setAttribute("r_r1", rightRootI.load()); xml->setAttribute("r_r2", rightRootII.load());
    xml->setAttribute("r_r3", rightRootIII.load()); xml->setAttribute("r_r4", rightRootIV.load());
    xml->setAttribute("r_r5", rightRootV.load()); xml->setAttribute("r_r6", rightRootVI.load());
    xml->setAttribute("r_r7", rightRootVII.load());

    xml->setAttribute("r_invMode", rightChordInversionMode.load());
    xml->setAttribute("r_dropB", rightDropBass.load());

    xml->setAttribute("chEng", chordEngineEnabled.load());

    return xml;
}

void GestureInstrumentAudioProcessor::loadPresetXml(juce::XmlElement* xml) {
    if (xml == nullptr || !xml->hasTagName("GestureInstrumentState")) {
        return;
    }

    currentOutputMode = static_cast<OutputMode>(xml->getIntAttribute("outputMode", 1));
    currentInstrument = xml->getIntAttribute("currentInstrument", 1);

    rootNote = xml->getIntAttribute("rootNote", 0);
    scaleType = xml->getIntAttribute("scaleType", 0);
    currentRangeMode = static_cast<MusicalRangeMode>(xml->getIntAttribute("currentRangeMode", 1));
    startNote = xml->getIntAttribute("startNote", 48);
    endNote = xml->getIntAttribute("endNote", 72);
    octaveRange = xml->getIntAttribute("octaveRange", 2);
    invertNoteTrigger = xml->getBoolAttribute("invertNoteTrigger", false);

    showFloorShadow = xml->getBoolAttribute("showFloorShadow", true);
    showWallShadow = xml->getBoolAttribute("showWallShadow", true);
    showNoteNames = xml->getBoolAttribute("showNoteNames", false);
    globalMute.store(xml->getBoolAttribute("globalMute", false));
    isWindowMaximized.store(xml->getBoolAttribute("isWindowMaximized", false));

    isGestureToMouseEnabled.store(xml->getBoolAttribute("isGestureToMouseEnabled", true));
    virtualMouseGestureType = xml->getIntAttribute("virtualMouseGestureType", 1);
    virtualMouseHoldTime = (float)xml->getDoubleAttribute("virtualMouseHoldTime", 1.5);

    leftXTarget = static_cast<GestureTarget>(xml->getIntAttribute("leftXTarget", 99));
    leftYTarget = static_cast<GestureTarget>(xml->getIntAttribute("leftYTarget", 99));
    leftZTarget = static_cast<GestureTarget>(xml->getIntAttribute("leftZTarget", 99));
    leftRollTarget = static_cast<GestureTarget>(xml->getIntAttribute("leftRollTarget", 99));
    leftGrabTarget = static_cast<GestureTarget>(xml->getIntAttribute("leftGrabTarget", 99));
    leftPinchTarget = static_cast<GestureTarget>(xml->getIntAttribute("leftPinchTarget", 99));
    leftThumbTarget = static_cast<GestureTarget>(xml->getIntAttribute("leftThumbTarget", 99));
    leftIndexTarget = static_cast<GestureTarget>(xml->getIntAttribute("leftIndexTarget", 99));
    leftMiddleTarget = static_cast<GestureTarget>(xml->getIntAttribute("leftMiddleTarget", 99));
    leftRingTarget = static_cast<GestureTarget>(xml->getIntAttribute("leftRingTarget", 99));
    leftPinkyTarget = static_cast<GestureTarget>(xml->getIntAttribute("leftPinkyTarget", 99));

    rightXTarget = static_cast<GestureTarget>(xml->getIntAttribute("rightXTarget", 99));
    rightYTarget = static_cast<GestureTarget>(xml->getIntAttribute("rightYTarget", 99));
    rightZTarget = static_cast<GestureTarget>(xml->getIntAttribute("rightZTarget", 99));
    rightRollTarget = static_cast<GestureTarget>(xml->getIntAttribute("rightRollTarget", 99));
    rightGrabTarget = static_cast<GestureTarget>(xml->getIntAttribute("rightGrabTarget", 99));
    rightPinchTarget = static_cast<GestureTarget>(xml->getIntAttribute("rightPinchTarget", 99));
    rightThumbTarget = static_cast<GestureTarget>(xml->getIntAttribute("rightThumbTarget", 99));
    rightIndexTarget = static_cast<GestureTarget>(xml->getIntAttribute("rightIndexTarget", 99));
    rightMiddleTarget = static_cast<GestureTarget>(xml->getIntAttribute("rightMiddleTarget", 99));
    rightRingTarget = static_cast<GestureTarget>(xml->getIntAttribute("rightRingTarget", 99));
    rightPinkyTarget = static_cast<GestureTarget>(xml->getIntAttribute("rightPinkyTarget", 99));

    minWidthThreshold = (float)xml->getDoubleAttribute("minWidthThreshold", -200.0);
    maxWidthThreshold = (float)xml->getDoubleAttribute("maxWidthThreshold", 200.0);
    minHeightThreshold = (float)xml->getDoubleAttribute("minHeightThreshold", 150.0);
    maxHeightThreshold = (float)xml->getDoubleAttribute("maxHeightThreshold", 450.0);
    minDepthThreshold = (float)xml->getDoubleAttribute("minDepthThreshold", -150.0);
    maxDepthThreshold = (float)xml->getDoubleAttribute("maxDepthThreshold", 150.0);

    wristMultiplier.store((float)xml->getDoubleAttribute("wristMult", 1.0));
    grabMultiplier.store((float)xml->getDoubleAttribute("grabMult", 1.0));
    pinchMultiplier.store((float)xml->getDoubleAttribute("pinchMult", 1.0));

    staticVolume = (float)xml->getDoubleAttribute("staticVolume", 1.0);
    staticPan = (float)xml->getDoubleAttribute("staticPan", 0.5);
    staticModulation = (float)xml->getDoubleAttribute("staticModulation", 0.0);
    staticExpression = (float)xml->getDoubleAttribute("staticExpression", 0.0);
    staticCutoff = (float)xml->getDoubleAttribute("staticCutoff", 1.0);
    staticResonance = (float)xml->getDoubleAttribute("staticResonance", 0.1);
    staticAttack = (float)xml->getDoubleAttribute("staticAttack", 0.1);
    staticRelease = (float)xml->getDoubleAttribute("staticRelease", 0.1);
    staticReverb = (float)xml->getDoubleAttribute("staticReverb", 0.0);
    staticChorus = (float)xml->getDoubleAttribute("staticChorus", 0.0);
    staticVibrato = (float)xml->getDoubleAttribute("staticVibrato", 0.0);
    staticWaveform = (float)xml->getDoubleAttribute("staticWaveform", 0.0);
    staticDelay = (float)xml->getDoubleAttribute("staticDelay", 0.0);
    staticDistortion = (float)xml->getDoubleAttribute("staticDistortion", 0.0);

    juce::String scaleStr = xml->getStringAttribute("customScaleIntervals", "");
    if (scaleStr.isNotEmpty()) {
        customScaleIntervals.clear();
        juce::StringArray tokens;
        tokens.addTokens(scaleStr, ",", "");
        for (auto t : tokens) {
            customScaleIntervals.push_back(t.getIntValue());
        }
        oscManager.updateCustomScale(customScaleIntervals);
    }

    isMpeEnabled = xml->getBoolAttribute("isMpeEnabled", false);
    enableSplitXAxis.store(xml->getBoolAttribute("enableSplitXAxis", false));
    mpePitchBendAxis.store(xml->getIntAttribute("mpePitchBendAxis", 1));
    mpeTimbreAxis.store(xml->getIntAttribute("mpeTimbreAxis", 2));
    mpePressureAxis.store(xml->getIntAttribute("mpePressureAxis", 3));

    // Load left hand chord data
    leftChordDegree1.store(xml->getBoolAttribute("l_deg1", true));  leftChordDegree2.store(xml->getBoolAttribute("l_deg2", false));
    leftChordDegree3.store(xml->getBoolAttribute("l_deg3", true));  leftChordDegree4.store(xml->getBoolAttribute("l_deg4", false));
    leftChordDegree5.store(xml->getBoolAttribute("l_deg5", true));  leftChordDegree6.store(xml->getBoolAttribute("l_deg6", false));
    leftChordDegree7.store(xml->getBoolAttribute("l_deg7", false));

    leftRootI.store(xml->getBoolAttribute("l_r1", true));  leftRootII.store(xml->getBoolAttribute("l_r2", true));
    leftRootIII.store(xml->getBoolAttribute("l_r3", true));  leftRootIV.store(xml->getBoolAttribute("l_r4", true));
    leftRootV.store(xml->getBoolAttribute("l_r5", true));  leftRootVI.store(xml->getBoolAttribute("l_r6", true));
    leftRootVII.store(xml->getBoolAttribute("l_r7", true));

    leftChordInversionMode.store(xml->getIntAttribute("l_invMode", 0));
    leftDropBass.store(xml->getBoolAttribute("l_dropB", false));

    // load right hand chord data
    rightChordDegree1.store(xml->getBoolAttribute("r_deg1", true));  rightChordDegree2.store(xml->getBoolAttribute("r_deg2", false));
    rightChordDegree3.store(xml->getBoolAttribute("r_deg3", true));  rightChordDegree4.store(xml->getBoolAttribute("r_deg4", false));
    rightChordDegree5.store(xml->getBoolAttribute("r_deg5", true));  rightChordDegree6.store(xml->getBoolAttribute("r_deg6", false));
    rightChordDegree7.store(xml->getBoolAttribute("r_deg7", false));

    rightRootI.store(xml->getBoolAttribute("r_r1", true)); rightRootII.store(xml->getBoolAttribute("r_r2", true));
    rightRootIII.store(xml->getBoolAttribute("r_r3", true)); rightRootIV.store(xml->getBoolAttribute("r_r4", true));
    rightRootV.store(xml->getBoolAttribute("r_r5", true)); rightRootVI.store(xml->getBoolAttribute("r_r6", true));
    rightRootVII.store(xml->getBoolAttribute("r_r7", true));

    rightChordInversionMode.store(xml->getIntAttribute("r_invMode", 0));
    rightDropBass.store(xml->getBoolAttribute("r_dropB", false));

    chordEngineEnabled.store(xml->getBoolAttribute("chEng", true));
}