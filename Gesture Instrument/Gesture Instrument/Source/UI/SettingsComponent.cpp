#include "SettingsComponent.h"

SettingsComponent::SettingsComponent(GestureInstrumentAudioProcessor& p)
    : audioProcessor(p),
    sensitivityControl("Sensitivity", 0.1f, 5.0f, p.sensitivityLevel),
    minHeightControl("Min Height", 0.0f, 200.0f, p.minHeightThreshold),
    maxHeightControl("Max Height", 200.0f, 600.0f, p.maxHeightThreshold)
{
    setOpaque(true);

  
    auto getTargetFromId = [](int id) -> GestureTarget {
        if (id == 1) return GestureTarget::Volume;
        if (id == 2) return GestureTarget::Pitch;
        if (id == 3) return GestureTarget::Modulation;
        if (id == 4) return GestureTarget::Expression;
        if (id == 5) return GestureTarget::Resonance;
        if (id == 6) return GestureTarget::Vibrato;
        if (id == 7) return GestureTarget::Cutoff;
        if (id == 8) return GestureTarget::None;
        return GestureTarget::None;
        };

    auto setupRow = [&](MappingRow& row, GestureTarget& target) {
        addAndMakeVisible(row);
        row.comboBox.onChange = [&, getTargetFromId] { target = getTargetFromId(row.comboBox.getSelectedId()); };
        };

    // left hand setup
    setupRow(leftXRow, audioProcessor.leftXTarget);
    setupRow(leftYRow, audioProcessor.leftYTarget);
    setupRow(leftZRow, audioProcessor.leftZTarget);
    setupRow(leftWristRow, audioProcessor.leftRollTarget);
    setupRow(leftGrabRow, audioProcessor.leftGrabTarget);
    setupRow(leftPinchRow, audioProcessor.leftPinchTarget);
    setupRow(leftThumbRow, audioProcessor.leftThumbTarget);
    setupRow(leftIndexRow, audioProcessor.leftIndexTarget);
    setupRow(leftMiddleRow, audioProcessor.leftMiddleTarget);
    setupRow(leftRingRow, audioProcessor.leftRingTarget);
    setupRow(leftPinkyRow, audioProcessor.leftPinkyTarget);

    // Right hand setup
    setupRow(rightXRow, audioProcessor.rightXTarget);
    setupRow(rightYRow, audioProcessor.rightYTarget);
    setupRow(rightZRow, audioProcessor.rightZTarget);
    setupRow(rightWristRow, audioProcessor.rightRollTarget);
    setupRow(rightGrabRow, audioProcessor.rightGrabTarget);
    setupRow(rightPinchRow, audioProcessor.rightPinchTarget);
    setupRow(rightThumbRow, audioProcessor.rightThumbTarget);
    setupRow(rightIndexRow, audioProcessor.rightIndexTarget);
    setupRow(rightMiddleRow, audioProcessor.rightMiddleTarget);
    setupRow(rightRingRow, audioProcessor.rightRingTarget);
    setupRow(rightPinkyRow, audioProcessor.rightPinkyTarget);

    // Labels
    addAndMakeVisible(titleLabel);
    titleLabel.setFont(juce::Font(24.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(leftHandLabel);
    leftHandLabel.setFont(juce::Font(18.0f, juce::Font::bold));
    leftHandLabel.setJustificationType(juce::Justification::centred);
    leftHandLabel.setColour(juce::Label::textColourId, juce::Colours::cyan);

    addAndMakeVisible(rightHandLabel);
    rightHandLabel.setFont(juce::Font(18.0f, juce::Font::bold));
    rightHandLabel.setJustificationType(juce::Justification::centred);
    rightHandLabel.setColour(juce::Label::textColourId, juce::Colours::orange);

    // Sliders
    addAndMakeVisible(sensitivityControl);
    addAndMakeVisible(minHeightControl);
    addAndMakeVisible(maxHeightControl);

   
    sensitivityControl.slider.onValueChange = [this] { audioProcessor.sensitivityLevel = (float)sensitivityControl.slider.getValue(); };
    minHeightControl.slider.onValueChange = [this] { audioProcessor.minHeightThreshold = (float)minHeightControl.slider.getValue(); };
    maxHeightControl.slider.onValueChange = [this] { audioProcessor.maxHeightThreshold = (float)maxHeightControl.slider.getValue(); };

    addAndMakeVisible(closeButton);


    //  Instrument Selector
    addAndMakeVisible(instrumentSelector);
    instrumentSelector.addItem("Piano (1)", 1);
    instrumentSelector.addItem("Warm Pad (90)", 90);
    instrumentSelector.addItem("Choir Pad (92)", 92);
    instrumentSelector.addItem("Sci-Fi (97)", 97); // "Soundtrack"
    instrumentSelector.addItem("Atmosphere (100)", 100);

    instrumentSelector.setSelectedId(p.currentInstrument);

    instrumentSelector.onChange = [this] {
        audioProcessor.currentInstrument = instrumentSelector.getSelectedId();
        audioProcessor.instrumentChanged = true;
        };

    addAndMakeVisible(instrumentLabel);
    instrumentLabel.setText("Instrument:", juce::dontSendNotification);
    instrumentLabel.attachToComponent(&instrumentSelector, true);
}

SettingsComponent::~SettingsComponent() {}

void SettingsComponent::paint(juce::Graphics& g) {
    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::grey);
    g.drawVerticalLine(getWidth() / 2, 60, getHeight() - 150);
}

void SettingsComponent::resized() {
    auto bounds = getLocalBounds().reduced(20);

    titleLabel.setBounds(bounds.removeFromTop(40));

    auto bottomArea = bounds.removeFromBottom(180);


    auto leftCol = bounds.removeFromLeft(bounds.getWidth() / 2).reduced(10, 0);
    auto rightCol = bounds.reduced(10, 0); 
    leftHandLabel.setBounds(leftCol.removeFromTop(30));
    rightHandLabel.setBounds(rightCol.removeFromTop(30));

    auto stackRow = [](juce::Rectangle<int>& area, juce::Component& c) {
        c.setBounds(area.removeFromTop(25)); // Give row 25px height
        area.removeFromTop(5);               // Burn 5px for spacing
        };

    stackRow(leftCol, leftXRow);
    stackRow(leftCol, leftYRow);
    stackRow(leftCol, leftZRow);
    stackRow(leftCol, leftWristRow);
    stackRow(leftCol, leftGrabRow);
    stackRow(leftCol, leftPinchRow);
    leftCol.removeFromTop(10); // Gap
    stackRow(leftCol, leftThumbRow);
    stackRow(leftCol, leftIndexRow);
    stackRow(leftCol, leftMiddleRow);
    stackRow(leftCol, leftRingRow);
    stackRow(leftCol, leftPinkyRow);

    stackRow(rightCol, rightXRow);
    stackRow(rightCol, rightYRow);
    stackRow(rightCol, rightZRow);
    stackRow(rightCol, rightWristRow);
    stackRow(rightCol, rightGrabRow);
    stackRow(rightCol, rightPinchRow);
    rightCol.removeFromTop(10); // Gap
    stackRow(rightCol, rightThumbRow);
    stackRow(rightCol, rightIndexRow);
    stackRow(rightCol, rightMiddleRow);
    stackRow(rightCol, rightRingRow);
    stackRow(rightCol, rightPinkyRow);


    instrumentSelector.setBounds(bottomArea.removeFromTop(30).reduced(80, 0));

    bottomArea.removeFromTop(10); // Spacer

    sensitivityControl.setBounds(bottomArea.removeFromTop(40));
    bottomArea.removeFromTop(5); // Spacer
    minHeightControl.setBounds(bottomArea.removeFromTop(40));
    bottomArea.removeFromTop(5); // Spacer
    maxHeightControl.setBounds(bottomArea.removeFromTop(40));

    closeButton.setBounds(getLocalBounds().getCentreX() - 50, getHeight() - 40, 100, 30);
}