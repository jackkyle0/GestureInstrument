#include "PluginProcessor.h"
#include "PluginEditor.h"

GestureInstrumentAudioProcessorEditor::GestureInstrumentAudioProcessorEditor(GestureInstrumentAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), settingsPage(p)
{
    // Output Mode
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

    // Connection Status
    addAndMakeVisible(connectionStatusLabel);
    connectionStatusLabel.setText("Checking Sensor...", juce::dontSendNotification);
    connectionStatusLabel.setJustificationType(juce::Justification::centredRight);
    connectionStatusLabel.setColour(juce::Label::textColourId, juce::Colours::grey);

    // Settings Page+Button
    addAndMakeVisible(settingsButton);
    settingsButton.onClick = [this] {
        settingsPage.setVisible(true);
        settingsButton.setVisible(false);

        // Hide Scale controls when settings are open
        rootSelector.setVisible(false);
        scaleSelector.setVisible(false);
        };

    addChildComponent(settingsPage);
    settingsPage.setVisible(false);

    settingsPage.closeButton.onClick = [this] {
        settingsPage.setVisible(false);
        settingsButton.setVisible(true);

        // Show Scale controls again
        rootSelector.setVisible(true);
        scaleSelector.setVisible(true);
        };


    // Root Note Selector
    addAndMakeVisible(rootSelector);
    rootSelector.addItemList({ "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" }, 1);
    rootSelector.setSelectedId(p.rootNote + 1);
    rootSelector.onChange = [this] { audioProcessor.rootNote = rootSelector.getSelectedId() - 1; };

    // Scale Type Selector
    addAndMakeVisible(scaleSelector);
    scaleSelector.addItem("Chromatic", 1);
    scaleSelector.addItem("Major", 2);
    scaleSelector.addItem("Minor", 3);
    scaleSelector.addItem("Pentatonic", 4);
    scaleSelector.setSelectedId(p.scaleType + 1);
    scaleSelector.onChange = [this] { audioProcessor.scaleType = scaleSelector.getSelectedId() - 1; };

    // Label
    addAndMakeVisible(scaleLabel);
    scaleLabel.setText("Key / Scale:", juce::dontSendNotification);
    scaleLabel.attachToComponent(&rootSelector, true);
    scaleLabel.setColour(juce::Label::textColourId, juce::Colours::white);

    // Graphics Setup
    setResizable(true, true);
    setResizeLimits(800, 600, 3000, 2000);
    setOpaque(true);

    openGLContext.attachTo(*this);
    openGLContext.setContinuousRepainting(false); // control the paint timing


    setSize(1200, 800);

    startTimerHz(60);
    }


GestureInstrumentAudioProcessorEditor::~GestureInstrumentAudioProcessorEditor() {
    openGLContext.detach();
    stopTimer();
}

//==============================================================================
void GestureInstrumentAudioProcessorEditor::paint(juce::Graphics& g) {
    g.fillAll(juce::Colours::black);

    drawGrid(g);

    static const juce::String leftLabel("L");
    static const juce::String rightLabel("R");

    drawHand(g, audioProcessor.leftHand, juce::Colours::cyan, leftLabel);
    drawHand(g, audioProcessor.rightHand, juce::Colours::orange, rightLabel);
}

void GestureInstrumentAudioProcessorEditor::drawHand(juce::Graphics& g, const HandData& hand, juce::Colour colour, const juce::String& label) {
    if (!hand.isPresent) return;

    g.setColour(colour);

    float w = (float)getWidth();
    float h = (float)getHeight();

    float mapX = juce::jmap(hand.currentHandPositionX, -300.0f, 300.0f, 0.0f, w);
    float mapY = juce::jmap(hand.currentHandPositionY, 0.0f, 600.0f, h, 0.0f);

    bool isActive = (hand.currentHandPositionY > audioProcessor.minHeightThreshold);
    float palmSize = 30.0f;

    if (isActive)
        g.fillEllipse(mapX - palmSize / 2, mapY - palmSize / 2, palmSize, palmSize);
    else
        g.drawEllipse(mapX - palmSize / 2, mapY - palmSize / 2, palmSize, palmSize, 2.0f);

    g.setColour(juce::Colours::white);
    g.drawText(label, (int)mapX - 10, (int)mapY - 10, 20, 20, juce::Justification::centred);

    g.setColour(colour);

    for (const auto& finger : hand.fingers)
    {
        float tipX = juce::jmap(finger.fingerPositionX, -300.0f, 300.0f, 0.0f, w);
        float tipY = juce::jmap(finger.fingerPositionY, 0.0f, 600.0f, h, 0.0f);

        g.drawLine(mapX, mapY, tipX, tipY, 2.0f);
        g.fillEllipse(tipX - 5.0f, tipY - 5.0f, 10.0f, 10.0f);
    }
}

void GestureInstrumentAudioProcessorEditor::drawGrid(juce::Graphics& g) {
    g.setColour(juce::Colours::darkgrey.withAlpha(0.5f));

    float w = (float)getWidth();
    float h = (float)getHeight();

    // Vertical Lines
    const int numVerticalLines = 12;
    float spacing = w / numVerticalLines;

    for (int i = 0; i < numVerticalLines; ++i) {
        g.drawVerticalLine((int)(spacing * i), 0.0f, h);
    }

    // Horizontal Lines (Thresholds)
    float minY = juce::jmap(audioProcessor.minHeightThreshold, 0.0f, 600.0f, h, 0.0f);
    float maxY = juce::jmap(audioProcessor.maxHeightThreshold, 0.0f, 600.0f, h, 0.0f);

    g.setColour(juce::Colours::red);
    g.drawHorizontalLine((int)minY, 0.0f, w);
    g.drawText("Min Threshold", 5, (int)minY + 2, 200, 20, juce::Justification::left);

    g.setColour(juce::Colours::green);
    g.drawHorizontalLine((int)maxY, 0.0f, w);
    g.drawText("Max Threshold", 5, (int)maxY - 20, 200, 20, juce::Justification::left);
}

void GestureInstrumentAudioProcessorEditor::resized() {
    int margin = 10;
    int topBarY = margin;

    //Settings Button in Bottom Center
    int buttonW = 120;
    settingsButton.setBounds(getLocalBounds().getCentreX() - (buttonW / 2), margin, buttonW, 30);
    settingsPage.setBounds(getLocalBounds());

    // Connection Status in Top Right
    connectionStatusLabel.setBounds(getWidth() - 200 - margin, topBarY, 200, 20);

    // Output Mode in Top Right, below status
    modeSelector.setBounds(getWidth() - 160 - margin, topBarY + 30, 150, 30);

    // Scale Controls in Top Left
    int controlHeight = 30;

    // Root Note Positioned with margin
    rootSelector.setBounds(margin + 80, topBarY + 10, 60, controlHeight);

    // Scale Type Positioned next to Root
    scaleSelector.setBounds(rootSelector.getRight() + 10, topBarY + 10, 120, controlHeight);
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

void GestureInstrumentAudioProcessorEditor::comboBoxChanged(juce::ComboBox* box)
{
    if (box == &modeSelector) {
        int id = modeSelector.getSelectedId();

        if (id == 1) audioProcessor.currentOutputMode = OutputMode::OSC_Only;
        if (id == 2) audioProcessor.currentOutputMode = OutputMode::MIDI_Only;
    }
}