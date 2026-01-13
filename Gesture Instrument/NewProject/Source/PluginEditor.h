#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "HandData.h"

class GestureInstrumentAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    GestureInstrumentAudioProcessorEditor (GestureInstrumentAudioProcessor&);
    ~GestureInstrumentAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    
    void timerCallback() override;
    void updateConnectionStatus();

private:
    GestureInstrumentAudioProcessor& audioProcessor;
    
    juce::Slider sensitivitySlider;
    juce::Label sensitivityLabel;
    
    juce::Slider minThresholdSlider;
    juce::Label minThresholdLabel;

    juce::Slider maxThresholdSlider;
    juce::Label maxThresholdLabel;
    
    juce::Label connectionStatusLabel;
    void drawHand(juce::Graphics& g, const HandData& hand, juce::Colour colour, juce::String label);
    void drawGrid(juce::Graphics& g);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GestureInstrumentAudioProcessorEditor)
};
