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

    leapThread.getLatestData(leftHand, rightHand, isSensorConnected);
    if (muteOutput.load()) {
        return;
    }

    if (instrumentChanged) {
        midiManager.sendProgramChange(midiMessages, currentInstrument);
        instrumentChanged = false;
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
            currentRangeMode, startNote, endNote 
        );
    }
}

bool GestureInstrumentAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* GestureInstrumentAudioProcessor::createEditor() {
    return new GestureInstrumentAudioProcessorEditor(*this);
}

void GestureInstrumentAudioProcessor::getStateInformation(juce::MemoryBlock& destData) {}
void GestureInstrumentAudioProcessor::setStateInformation(const void* data, int sizeInBytes) {}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new GestureInstrumentAudioProcessor();
}