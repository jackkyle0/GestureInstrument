#include "SettingsComponent.h"

SettingsComponent::SettingsComponent(GestureInstrumentAudioProcessor& p)
    : audioProcessor(p),
    sensitivityControl("Sensitivity", 0.1f, 5.0f, p.sensitivityLevel),
    minHeightControl("Min Height", 0.0f, 200.0f, p.minHeightThreshold),
    maxHeightControl("Max Height", 200.0f, 600.0f, p.maxHeightThreshold)
{
    setOpaque(true);

    auto getTargetFromId = [](int id) -> GestureTarget {
        switch (id) {
            // Essentials
        case 1: return GestureTarget::Volume;
        case 2: return GestureTarget::Pitch;
        case 13: return GestureTarget::NoteTrigger;
        case 3: return GestureTarget::Modulation;
        case 4: return GestureTarget::Expression;

            //  Shaping
        case 5: return GestureTarget::Cutoff;
        case 6: return GestureTarget::Resonance;
        case 10: return GestureTarget::Vibrato;

            // Spatial
        case 7: return GestureTarget::Pan;
        case 8: return GestureTarget::Reverb;

            // Switches
        case 9: return GestureTarget::Sustain;

            // Macros
        case 11: return GestureTarget::Macro1;
        case 12: return GestureTarget::Macro2;

        case 99: return GestureTarget::None;
        default: return GestureTarget::None;
        }
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
    instrumentSelector.addItem("Sci-Fi (97)", 97);
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

    auto bottomArea = bounds.removeFromBottom(100);

    auto mainArea = bounds;

-    int colWidth = 320;
    int colGap = 40;

    // Center the two columns
    int totalWidth = (colWidth * 2) + colGap;
    int startX = (mainArea.getWidth() - totalWidth) / 2;

    auto leftCol = mainArea.removeFromLeft(startX + colWidth).removeFromRight(colWidth);
    auto rightCol = mainArea.removeFromRight(startX + colWidth).removeFromLeft(colWidth);

    leftHandLabel.setBounds(leftCol.removeFromTop(30));
    rightHandLabel.setBounds(rightCol.removeFromTop(30));

    auto stackRow = [](juce::Rectangle<int>& area, juce::Component& c) {
        c.setBounds(area.removeFromTop(22)); // Reduced height slightly
        area.removeFromTop(4);               // Small gap
        };

    // Left hand rows
    stackRow(leftCol, leftXRow);
    stackRow(leftCol, leftYRow);
    stackRow(leftCol, leftZRow);
    stackRow(leftCol, leftWristRow);
    stackRow(leftCol, leftGrabRow);
    stackRow(leftCol, leftPinchRow);
    leftCol.removeFromTop(10); // Section Gap
    stackRow(leftCol, leftThumbRow);
    stackRow(leftCol, leftIndexRow);
    stackRow(leftCol, leftMiddleRow);
    stackRow(leftCol, leftRingRow);
    stackRow(leftCol, leftPinkyRow);

    // Right hand rows
    stackRow(rightCol, rightXRow);
    stackRow(rightCol, rightYRow);
    stackRow(rightCol, rightZRow);
    stackRow(rightCol, rightWristRow);
    stackRow(rightCol, rightGrabRow);
    stackRow(rightCol, rightPinchRow);
    rightCol.removeFromTop(10); // Section Gap
    stackRow(rightCol, rightThumbRow);
    stackRow(rightCol, rightIndexRow);
    stackRow(rightCol, rightMiddleRow);
    stackRow(rightCol, rightRingRow);
    stackRow(rightCol, rightPinkyRow);

    bool isMidi = (audioProcessor.currentOutputMode == OutputMode::MIDI_Only);
    instrumentSelector.setVisible(isMidi);
    instrumentLabel.setVisible(isMidi);

    if (isMidi) {
        instrumentSelector.setBounds(mainArea.getCentreX() - 100, mainArea.getBottom() - 40, 200, 25);
    }

    int sliderWidth = bottomArea.getWidth() / 3;

    sensitivityControl.setBounds(bottomArea.removeFromLeft(sliderWidth).reduced(10, 0));
    minHeightControl.setBounds(bottomArea.removeFromLeft(sliderWidth).reduced(10, 0));
    maxHeightControl.setBounds(bottomArea.removeFromLeft(sliderWidth).reduced(10, 0));

    closeButton.setBounds(getLocalBounds().getCentreX() - 50, getHeight() - 30, 100, 25);
}