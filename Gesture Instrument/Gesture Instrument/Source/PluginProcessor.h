#pragma once

#include <JuceHeader.h>
#include "Helpers/LeapService.h"
#include "HandData.h"
#include "OscManager.h"
#include "MidiManager.h"
#include "GestureTarget.h"


enum class OutputMode {

    OSC_Only,
    MIDI_Only
};

class GestureInstrumentAudioProcessor  : public juce::AudioProcessor 
{
public:
    GestureInstrumentAudioProcessor();
    ~GestureInstrumentAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    void updateOscSettings(juce::String newIp, int newPort) {
        oscManager.connect(newIp, newPort);
    }

    //==============================================================================
    
    OutputMode currentOutputMode = OutputMode::OSC_Only;
    
    float sensitivityLevel = 1.0f;   
    float minHeightThreshold = 50.0f; 
    float maxHeightThreshold = 300.0f;
    
    int leftHandTargetCC = 1;   
    int rightHandTargetCC = 7;  
    
    GestureTarget leftXTarget = GestureTarget::None;
    GestureTarget leftYTarget = GestureTarget::Volume; // Default
    GestureTarget leftZTarget = GestureTarget::None;
    GestureTarget leftRollTarget = GestureTarget::None;

    GestureTarget rightXTarget = GestureTarget::Pitch; // Default
    GestureTarget rightYTarget = GestureTarget::None;
    GestureTarget rightZTarget = GestureTarget::None;
    GestureTarget rightRollTarget = GestureTarget::None;

    HandData leftHand;
    HandData rightHand;
    bool isSensorConnected = false;

private:
    LeapService leapService;
    OscManager oscManager;
    MidiManager midiManager;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GestureInstrumentAudioProcessor)
};
