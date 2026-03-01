#include "PluginProcessor.h"
#include "PluginEditor.h"

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
    //  hardware polling thread at high priority
    leapThread.startThread(juce::Thread::Priority::high);
}

GestureInstrumentAudioProcessor::~GestureInstrumentAudioProcessor() {
    // Ensure the thread stops safely before the plugin is destroyed
    leapThread.stopThread(1000);
}

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

void GestureInstrumentAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
}

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
    midiMessages.clear();


    auto resetTrackers = [](auto& manager) {
        manager.liveVolume.store(-1.0f); manager.livePan.store(-1.0f); manager.liveModulation.store(-1.0f); manager.liveExpression.store(-1.0f);
        manager.liveCutoff.store(-1.0f); manager.liveResonance.store(-1.0f); manager.liveAttack.store(-1.0f); manager.liveRelease.store(-1.0f);
        manager.liveReverb.store(-1.0f); manager.liveChorus.store(-1.0f); manager.liveVibrato.store(-1.0f); manager.liveWaveform.store(-1.0f);
        };
    resetTrackers(oscManager);
    resetTrackers(midiManager);

    leapThread.getLatestData(leftHand, rightHand, isSensorConnected);
    if (muteOutput.load()) {
        return;
    }


    if (leftHand.isPresent) {
        xTracker.addDataPoint(leftHand.currentHandPositionX);
        yTracker.addDataPoint(leftHand.currentHandPositionY);
        zTracker.addDataPoint(leftHand.currentHandPositionZ);
    }
    if (rightHand.isPresent) {
        xTracker.addDataPoint(rightHand.currentHandPositionX);
        yTracker.addDataPoint(rightHand.currentHandPositionY);
        zTracker.addDataPoint(rightHand.currentHandPositionZ);
    }

    auto xLimits = xTracker.getLearnedBoundaries(2.0f);
    auto yLimits = yTracker.getLearnedBoundaries(2.0f);
    auto zLimits = zTracker.getLearnedBoundaries(2.0f);

    float smooth = 0.999f;
    float lerp = 1.0f - smooth;

    minWidthThreshold = (minWidthThreshold * smooth) + (xLimits.minBound * lerp);
    maxWidthThreshold = (maxWidthThreshold * smooth) + (xLimits.maxBound * lerp);

    minHeightThreshold = (minHeightThreshold * smooth) + (yLimits.minBound * lerp);
    maxHeightThreshold = (maxHeightThreshold * smooth) + (yLimits.maxBound * lerp);

    minDepthThreshold = (minDepthThreshold * smooth) + (zLimits.minBound * lerp);
    maxDepthThreshold = (maxDepthThreshold * smooth) + (zLimits.maxBound * lerp);

    if (instrumentChanged) {
        midiManager.sendProgramChange(midiMessages, currentInstrument);
        instrumentChanged = false;
    }

    oscManager.updateAI(leftHand, rightHand);

    float leftStyle = oscManager.leftAIStyle;
    float rightStyle = oscManager.rightAIStyle;

    float aggressiveRole = std::max(leftStyle, rightStyle);
    float gentleRole = std::min(leftStyle, rightStyle);

 
    float filterBite = aggressiveRole;


    float atmosphereShape = 1.0f - gentleRole;

    if (currentOutputMode == OutputMode::MIDI_Only) {
        midiManager.sendCC(midiMessages, 1, GestureTarget::Cutoff, filterBite);
        midiManager.sendCC(midiMessages, 1, GestureTarget::Resonance, filterBite * 0.5f);
        midiManager.sendCC(midiMessages, 1, GestureTarget::Reverb, atmosphereShape);
        midiManager.sendCC(midiMessages, 1, GestureTarget::Chorus, atmosphereShape);
    }
    else if (currentOutputMode == OutputMode::OSC_Only) {
        juce::OSCMessage cutMsg("/global/cutoff"); cutMsg.addFloat32(filterBite); oscManager.sender.send(cutMsg);
        juce::OSCMessage resMsg("/global/res"); resMsg.addFloat32(filterBite * 0.5f); oscManager.sender.send(resMsg);

        juce::OSCMessage revMsg("/global/reverb"); revMsg.addFloat32(atmosphereShape); oscManager.sender.send(revMsg);
        juce::OSCMessage choMsg("/global/chorus"); choMsg.addFloat32(atmosphereShape); oscManager.sender.send(choMsg);

        oscManager.liveCutoff.store(filterBite);
        oscManager.liveResonance.store(filterBite * 0.5f);
        oscManager.liveReverb.store(atmosphereShape);
        oscManager.liveChorus.store(atmosphereShape);
    }





    oscManager.sendRawData(leftHand, rightHand);
    oscManager.sendMidiData(midiMessages);

    if (currentOutputMode == OutputMode::OSC_Only) {
        oscManager.processHandData(
            leftHand, rightHand,
            sensitivityLevel,
            minWidthThreshold, maxWidthThreshold,
            minHeightThreshold, maxHeightThreshold,
            minDepthThreshold, maxDepthThreshold,
            leftXTarget, leftYTarget, leftZTarget, leftRollTarget, leftGrabTarget, leftPinchTarget,
            leftThumbTarget, leftIndexTarget, leftMiddleTarget, leftRingTarget, leftPinkyTarget,
            rightXTarget, rightYTarget, rightZTarget, rightRollTarget, rightGrabTarget, rightPinchTarget,
            rightThumbTarget, rightIndexTarget, rightMiddleTarget, rightRingTarget, rightPinkyTarget,
            rootNote, scaleType, octaveRange
        );
    }

    if (currentOutputMode == OutputMode::MIDI_Only) {
        midiManager.processHandData(
            midiMessages, leftHand, rightHand,
            sensitivityLevel,
            minWidthThreshold, maxWidthThreshold,
            minHeightThreshold, maxHeightThreshold,
            minDepthThreshold, maxDepthThreshold,
            leftXTarget, leftYTarget, leftZTarget, leftRollTarget, leftGrabTarget, leftPinchTarget,
            leftThumbTarget, leftIndexTarget, leftMiddleTarget, leftRingTarget, leftPinkyTarget,
            rightXTarget, rightYTarget, rightZTarget, rightRollTarget, rightGrabTarget, rightPinchTarget,
            rightThumbTarget, rightIndexTarget, rightMiddleTarget, rightRingTarget, rightPinkyTarget,
            rootNote, scaleType, octaveRange, invertNoteTrigger,
            currentRangeMode, startNote, endNote, isMpeEnabled
        );
    }
}

bool GestureInstrumentAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* GestureInstrumentAudioProcessor::createEditor() {
    return new GestureInstrumentAudioProcessorEditor(*this);
}

void GestureInstrumentAudioProcessor::getStateInformation(juce::MemoryBlock& destData) {
    auto xml = createPresetXml();
    copyXmlToBinary(*xml, destData);
}

void GestureInstrumentAudioProcessor::setStateInformation(const void* data, int sizeInBytes) {
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    loadPresetXml(xmlState.get());
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new GestureInstrumentAudioProcessor();
}

std::unique_ptr<juce::XmlElement> GestureInstrumentAudioProcessor::createPresetXml() {
    auto xml = std::make_unique<juce::XmlElement>("GestureInstrumentState");

    xml->setAttribute("outputMode", static_cast<int>(currentOutputMode));
    xml->setAttribute("currentInstrument", currentInstrument);

    // Save Settings
    xml->setAttribute("rootNote", rootNote);
    xml->setAttribute("scaleType", scaleType);
    xml->setAttribute("currentRangeMode", static_cast<int>(currentRangeMode));
    xml->setAttribute("startNote", startNote);
    xml->setAttribute("endNote", endNote);
    xml->setAttribute("octaveRange", octaveRange);
    xml->setAttribute("invertNoteTrigger", invertNoteTrigger);

    // Save Left Hand Targets
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

    // Save Right Hand Targets
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

    // Save Calibration Thresholds
    xml->setAttribute("minWidthThreshold", minWidthThreshold);
    xml->setAttribute("maxWidthThreshold", maxWidthThreshold);
    xml->setAttribute("minHeightThreshold", minHeightThreshold);
    xml->setAttribute("maxHeightThreshold", maxHeightThreshold);
    xml->setAttribute("minDepthThreshold", minDepthThreshold);
    xml->setAttribute("maxDepthThreshold", maxDepthThreshold);

    return xml;
}

void GestureInstrumentAudioProcessor::loadPresetXml(juce::XmlElement* xml) {
    if (xml == nullptr || !xml->hasTagName("GestureInstrumentState")) {
        return;
    }

    // Load Output
    currentOutputMode = static_cast<OutputMode>(xml->getIntAttribute("outputMode", 1));
    currentInstrument = xml->getIntAttribute("currentInstrument", 1);

    // Load Settings
    rootNote = xml->getIntAttribute("rootNote", 0);
    scaleType = xml->getIntAttribute("scaleType", 0);
    currentRangeMode = static_cast<MusicalRangeMode>(xml->getIntAttribute("currentRangeMode", 1));
    startNote = xml->getIntAttribute("startNote", 48);
    endNote = xml->getIntAttribute("endNote", 72);
    octaveRange = xml->getIntAttribute("octaveRange", 2);
    invertNoteTrigger = xml->getBoolAttribute("invertNoteTrigger", false);

    // Load Left Hand Targets
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

    // Load Right Hand Targets
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

    // Load Calibration Thresholds (Fixed the fallback defaults!)
    minWidthThreshold = (float)xml->getDoubleAttribute("minWidthThreshold", -200.0);
    maxWidthThreshold = (float)xml->getDoubleAttribute("maxWidthThreshold", 200.0);
    minHeightThreshold = (float)xml->getDoubleAttribute("minHeightThreshold", 150.0);
    maxHeightThreshold = (float)xml->getDoubleAttribute("maxHeightThreshold", 450.0);
    minDepthThreshold = (float)xml->getDoubleAttribute("minDepthThreshold", -150.0);
    maxDepthThreshold = (float)xml->getDoubleAttribute("maxDepthThreshold", 150.0);
}