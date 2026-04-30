#pragma once

#include <JuceHeader.h>
#include "../PluginProcessor.h"

class HUDComponents : public juce::Component {
public:
    HUDComponents(GestureInstrumentAudioProcessor& p);

    void paint(juce::Graphics& g) override;

    bool isEditMode = false;
    float menuGestureTimer = 0.0f;
    float requiredHoldTime = 1.0f;
    bool isCalibrating = false;
    bool menuGestureFired = false;

private:
    GestureInstrumentAudioProcessor& audioProcessor;

    void drawParameterHUD(juce::Graphics& g, juce::Rectangle<int> bounds, GestureTarget target, float value, bool isVertical, juce::String label, juce::Colour color, bool isLeftHand);
    void drawScaleBlocks(juce::Graphics& g, juce::Rectangle<int> bounds, float value, bool isVertical, juce::Colour color, bool isLeftHand);
    void drawFaderBar(juce::Graphics& g, juce::Rectangle<int> bounds, float value, bool isVertical, juce::Colour color);
    void drawBooleanBox(juce::Graphics& g, juce::Rectangle<int> bounds, float value, juce::Colour color);
    void drawDial(juce::Graphics& g, juce::Rectangle<int> bounds, GestureTarget target, float value, juce::String label, juce::Colour color);
    void drawUnquantisedPitch(juce::Graphics& g, juce::Rectangle<int> bounds, float value, bool isVertical, juce::Colour color);
};