#pragma once

#include <JuceHeader.h>
#include "LeapService.h"
#include "HandData.h"

class GestureInstrumentAudioProcessor  : public juce::AudioProcessor 
{
public:
    GestureInstrumentAudioProcessor();
    ~GestureInstrumentAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
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

    //==============================================================================
    
    int currentNote = 1;
    
    float sensitivityLevel = 1.0f;   
    float minHeightThreshold = 50.0f; 
    float maxHeightThreshold = 300.0f;
    
    int leftHandTargetCC = 1;   
    int rightHandTargetCC = 7;  
    
    bool isSensorConnected = false;

    HandData leftHand;
    HandData rightHand;

private:
    LeapService leapService;

    int lastVolume = -1;
    int lastVibrato = -1;

    int lastNoteTriggered = -1;
    bool instrumentSetup = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GestureInstrumentAudioProcessor)
};
