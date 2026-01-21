#include "PluginProcessor.h"
#include "PluginEditor.h"

GestureInstrumentAudioProcessorEditor::GestureInstrumentAudioProcessorEditor (GestureInstrumentAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Output Box MIDI/OSC
    addAndMakeVisible(modeSelector);
    modeSelector.addItem("OSC", 1);
    modeSelector.addItem("MIDI", 2);

    switch (audioProcessor.currentOutputMode) {
        case OutputMode::OSC_Only:  modeSelector.setSelectedId(1); break;
        case OutputMode::MIDI_Only: modeSelector.setSelectedId(2); break;
    }

    modeSelector.addListener(this);

    addAndMakeVisible(modeLabel);
    modeLabel.setText("Output Mode:", juce::dontSendNotification);
    modeLabel.attachToComponent(&modeSelector, true);


    // Sensitivity
    addAndMakeVisible(sensitivitySlider);
    sensitivitySlider.setRange(0.1, 5.0, 0.1);
    sensitivitySlider.setValue(audioProcessor.sensitivityLevel);
    sensitivitySlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 50, 20);
    sensitivitySlider.onValueChange = [this] { audioProcessor.sensitivityLevel = (int)sensitivitySlider.getValue(); };

    addAndMakeVisible(sensitivityLabel);
    sensitivityLabel.setText("Sensitivity", juce::dontSendNotification);

    // Min Height
    addAndMakeVisible(minThresholdSlider);
    minThresholdSlider.setRange(0.0, 200.0, 1.0);
    minThresholdSlider.setValue(audioProcessor.minHeightThreshold);
    minThresholdSlider.onValueChange = [this] { audioProcessor.minHeightThreshold = minThresholdSlider.getValue(); };

    addAndMakeVisible(minThresholdLabel);
    minThresholdLabel.setText("Min Height", juce::dontSendNotification);

    // Max Height
    addAndMakeVisible(maxThresholdSlider);
    maxThresholdSlider.setRange(200.0, 600.0, 1.0);
    maxThresholdSlider.setValue(audioProcessor.maxHeightThreshold);
    maxThresholdSlider.onValueChange = [this] { audioProcessor.maxHeightThreshold = maxThresholdSlider.getValue(); };

    addAndMakeVisible(maxThresholdLabel);
    maxThresholdLabel.setText("Max Height", juce::dontSendNotification);

    addAndMakeVisible(connectionStatusLabel);
    connectionStatusLabel.setText("Checking Sensor...", juce::dontSendNotification);
    connectionStatusLabel.setJustificationType(juce::Justification::centredRight);
    connectionStatusLabel.setColour(juce::Label::textColourId, juce::Colours::grey);

    setSize(800, 600);

    startTimerHz(60);
    }

GestureInstrumentAudioProcessorEditor::~GestureInstrumentAudioProcessorEditor(){
    stopTimer();
}

//==============================================================================
void GestureInstrumentAudioProcessorEditor::paint (juce::Graphics& g) {
    g.fillAll(juce::Colours::black);

    drawGrid(g);
    drawHand(g, audioProcessor.leftHand, juce::Colours::cyan, "L");
    drawHand(g, audioProcessor.rightHand, juce::Colours::orange, "R");
    
}



void GestureInstrumentAudioProcessorEditor::drawHand(juce::Graphics& g, const HandData& hand, juce::Colour colour, juce::String label) {
    if (!hand.isPresent) return;

    g.setColour(colour);

    float mapX = juce::jmap(hand.currentHandPositionX, -300.0f, 300.0f, 0.0f, (float)getWidth());
    float mapY = juce::jmap(hand.currentHandPositionY, 0.0f, 600.0f, (float)getHeight(), 0.0f);

    bool isActive = (hand.currentHandPositionY > audioProcessor.minHeightThreshold);

    float palmSize = 30.0f;
    if (isActive)
        g.fillEllipse(mapX - palmSize / 2, mapY - palmSize / 2, palmSize, palmSize);
    else
        g.drawEllipse(mapX - palmSize / 2, mapY - palmSize / 2, palmSize, palmSize, 2.0f);

    g.setColour(juce::Colours::white);
    g.drawText(label, (int)mapX - 10, (int)mapY - 10, 20, 20, juce::Justification::centred);
    g.setColour(colour);

    // Draw Fingers
    for (const auto& finger : hand.fingers)
    {
        float tipX = juce::jmap(finger.fingerPositionX, -300.0f, 300.0f, 0.0f, (float)getWidth());
        float tipY = juce::jmap(finger.fingerPositionY, 0.0f, 600.0f, (float)getHeight(), 0.0f);

        g.drawLine(mapX, mapY, tipX, tipY, 2.0f);
        g.fillEllipse(tipX - 5.0f, tipY - 5.0f, 10.0f, 10.0f);
    }
}

void GestureInstrumentAudioProcessorEditor::drawGrid(juce::Graphics& g) {
    // Vertical Lines
    g.setColour(juce::Colours::darkgrey.withAlpha(0.5f));
    const int numVerticalLines = 12;
    for (int i = 0; i < numVerticalLines; ++i) {
        float x = (float)getWidth() / numVerticalLines * i;
        g.drawVerticalLine((int)x, 0.0f, (float)getHeight());
    }

    // Horizontal Lines
    float minY = juce::jmap(audioProcessor.minHeightThreshold, 0.0f, 600.0f, (float)getHeight(), 0.0f);
    float maxY = juce::jmap(audioProcessor.maxHeightThreshold, 0.0f, 600.0f, (float)getHeight(), 0.0f);

    // Draw Min Height 
    g.setColour(juce::Colours::red);
    g.drawHorizontalLine((int)minY, 0.0f, (float)getWidth());
    g.drawText("Min Threshold", 5, (int)minY + 2, 200, 20, juce::Justification::left);

    // Draw Max Height
    g.setColour(juce::Colours::green);
    g.drawHorizontalLine((int)maxY, 0.0f, (float)getWidth());
    g.drawText("Max Threshold", 5, (int)maxY - 20, 200, 20, juce::Justification::left);
}

void GestureInstrumentAudioProcessorEditor::resized() {
    // Sliders 
    int margin = 10;
    int w = 200;
    int h = 20;

    connectionStatusLabel.setBounds(getWidth() - 200 - margin, margin, 200, h);

    sensitivityLabel.setBounds(margin, margin, w, h);
    sensitivitySlider.setBounds(margin, margin + h, w, h);

    minThresholdLabel.setBounds(margin, margin + h * 3, w, h);
    minThresholdSlider.setBounds(margin, margin + h * 4, w, h);

    maxThresholdLabel.setBounds(margin, margin + h * 6, w, h);
    maxThresholdSlider.setBounds(margin, margin + h * 7, w, h);

    modeSelector.setBounds(getWidth() - 160 - margin, margin + 30, 150, 30);
}


void GestureInstrumentAudioProcessorEditor::timerCallback() {
    updateConnectionStatus();
    repaint();
}

void GestureInstrumentAudioProcessorEditor::updateConnectionStatus() {
    bool connected = audioProcessor.isSensorConnected;

    if (connected) {
        connectionStatusLabel.setText("Sensor Connected", juce::dontSendNotification);
        connectionStatusLabel.setColour(juce::Label::textColourId, juce::Colours::green);
    }
    else {
        connectionStatusLabel.setText("Sensor Disconnected", juce::dontSendNotification);
        connectionStatusLabel.setColour(juce::Label::textColourId, juce::Colours::red);
    }
}

void GestureInstrumentAudioProcessorEditor::comboBoxChanged(juce::ComboBox * box)
{
    if (box == &modeSelector) {
        int id = modeSelector.getSelectedId();

        if (id == 1) audioProcessor.currentOutputMode = OutputMode::OSC_Only;
        if (id == 2) audioProcessor.currentOutputMode = OutputMode::MIDI_Only;
    }
}

