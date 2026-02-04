#include "PluginProcessor.h"
#include "PluginEditor.h"

GestureInstrumentAudioProcessor::GestureInstrumentAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    oscManager.connect("127.0.0.1", 9000);
}
    
GestureInstrumentAudioProcessor::~GestureInstrumentAudioProcessor()
{
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
int GestureInstrumentAudioProcessor::getCurrentProgram(){ return 0; }
void GestureInstrumentAudioProcessor::setCurrentProgram (int index){
}
const juce::String GestureInstrumentAudioProcessor::getProgramName (int index){ return {}; }
void GestureInstrumentAudioProcessor::changeProgramName (int index, const juce::String& newName){
}

//==============================================================================
void GestureInstrumentAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock){
}
void GestureInstrumentAudioProcessor::releaseResources(){
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool GestureInstrumentAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

//==============================================================================
void GestureInstrumentAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    buffer.clear();
    midiMessages.clear();

    if (instrumentChanged) {
        midiManager.sendProgramChange(midiMessages, currentInstrument);
        instrumentChanged = false; // Only send once
    }

    leapService.pollHandData(leftHand, rightHand, isSensorConnected);

   
    // OSC
    if (currentOutputMode == OutputMode::OSC_Only) {
        oscManager.processHandData(leftHand, rightHand,
            sensitivityLevel, minHeightThreshold, maxHeightThreshold,
            leftXTarget, leftYTarget, leftZTarget, leftRollTarget, leftGrabTarget, leftPinchTarget,
            leftThumbTarget, leftIndexTarget, leftMiddleTarget, leftRingTarget, leftPinkyTarget,

            rightXTarget, rightYTarget, rightZTarget, rightRollTarget, rightGrabTarget, rightPinchTarget,
            rightThumbTarget, rightIndexTarget, rightMiddleTarget, rightRingTarget, rightPinkyTarget,
            rootNote, scaleType, octaveRange
        );
    }

    // MIDI
    if (currentOutputMode == OutputMode::MIDI_Only) {
        midiManager.processHandData(
            midiMessages, leftHand, rightHand,
            sensitivityLevel, minHeightThreshold, maxHeightThreshold,
            leftXTarget, leftYTarget, leftZTarget, leftRollTarget, leftGrabTarget, leftPinchTarget,
            leftThumbTarget, leftIndexTarget, leftMiddleTarget, leftRingTarget, leftPinkyTarget,

            rightXTarget, rightYTarget, rightZTarget, rightRollTarget, rightGrabTarget, rightPinchTarget,
            rightThumbTarget, rightIndexTarget, rightMiddleTarget, rightRingTarget, rightPinkyTarget,
            rootNote, scaleType, octaveRange
        );
    }
}


bool GestureInstrumentAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* GestureInstrumentAudioProcessor::createEditor() {
    return new GestureInstrumentAudioProcessorEditor (*this);
}

//==============================================================================
void GestureInstrumentAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void GestureInstrumentAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

// Create Instance of plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new GestureInstrumentAudioProcessor();
}
