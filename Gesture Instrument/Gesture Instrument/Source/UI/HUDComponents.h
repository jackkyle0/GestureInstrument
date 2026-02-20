#pragma once
#include <JuceHeader.h>
#include "../PluginProcessor.h"

class HUDComponents : public juce::Component {
public:
    HUDComponents(GestureInstrumentAudioProcessor& p);
    void paint(juce::Graphics& g) override;

    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;

private:
    GestureInstrumentAudioProcessor& audioProcessor;

    // Move your helper drawing functions here!
    void drawParameterHUD(juce::Graphics& g, juce::Rectangle<int> bounds, GestureTarget target, float value, bool isVertical, juce::String label, juce::Colour color);
    void drawScaleBlocks(juce::Graphics& g, juce::Rectangle<int> bounds, float value, bool isVertical, juce::Colour color);
    void drawFaderBar(juce::Graphics& g, juce::Rectangle<int> bounds, float value, bool isVertical, juce::Colour color);
    void drawBooleanBox(juce::Graphics& g, juce::Rectangle<int> bounds, float value, juce::Colour color);
    std::vector<int> getScaleIntervals(int scaleType);

    // Inside HUDComponents class:


    juce::ComponentDragger myDragger;
};