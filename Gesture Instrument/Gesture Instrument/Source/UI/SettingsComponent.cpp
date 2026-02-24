#include "SettingsComponent.h"

SettingsComponent::SettingsComponent(GestureInstrumentAudioProcessor& p)
    : audioProcessor(p)
{
    setOpaque(true);

    addAndMakeVisible(invertTriggerButton);
    invertTriggerButton.setToggleState(audioProcessor.invertNoteTrigger, juce::dontSendNotification);
    invertTriggerButton.onClick = [this] {
        audioProcessor.invertNoteTrigger = invertTriggerButton.getToggleState();
        };

    auto getTargetFromId = [](int id) -> GestureTarget {
        switch (id) {
        case 1: return GestureTarget::Volume;
        case 2: return GestureTarget::Pitch;
        case 3: return GestureTarget::NoteTrigger;
        case 4: return GestureTarget::Modulation;
        case 5: return GestureTarget::Expression;
        case 6: return GestureTarget::Breath;
        case 7: return GestureTarget::Cutoff;
        case 8: return GestureTarget::Resonance;
        case 9: return GestureTarget::Attack;
        case 10: return GestureTarget::Release;
        case 11: return GestureTarget::Vibrato;
        case 12: return GestureTarget::Pan;
        case 13: return GestureTarget::Reverb;
        case 14: return GestureTarget::Chorus;
        case 15: return GestureTarget::Sustain;
        case 16: return GestureTarget::Portamento;
        case 99: return GestureTarget::None;
        default: return GestureTarget::None;
        }
        };

    // Needed for syncing purposes
    auto getIdFromTarget = [](GestureTarget target) -> int {
        switch (target) {
        case GestureTarget::Volume: return 1;
        case GestureTarget::Pitch: return 2;
        case GestureTarget::NoteTrigger: return 3;
        case GestureTarget::Modulation: return 4;
        case GestureTarget::Expression: return 5;
        case GestureTarget::Breath: return 6;
        case GestureTarget::Cutoff: return 7;
        case GestureTarget::Resonance: return 8;
        case GestureTarget::Attack: return 9;
        case GestureTarget::Release: return 10;
        case GestureTarget::Vibrato: return 11;
        case GestureTarget::Pan: return 12;
        case GestureTarget::Reverb: return 13;
        case GestureTarget::Chorus: return 14;
        case GestureTarget::Sustain: return 15;
        case GestureTarget::Portamento: return 16;
        case GestureTarget::None: return 99;
        default: return 99;
        }
        };

    auto setupRow = [&](MappingRow& row, GestureTarget& target) {
        addAndMakeVisible(row);

        row.comboBox.setSelectedId(getIdFromTarget(target), juce::dontSendNotification);

        row.comboBox.onChange = [&row, &target, getTargetFromId] {
            target = getTargetFromId(row.comboBox.getSelectedId());
            };
        };

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

    addAndMakeVisible(closeButton);

    addAndMakeVisible(instrumentSelector);
    instrumentSelector.addItem("Piano", 1);
    instrumentSelector.addItem("Warm Pad", 90);
    instrumentSelector.addItem("Choir Pad", 92);
    instrumentSelector.addItem("Sci-Fi", 97);
    instrumentSelector.addItem("Atmosphere", 100);

    instrumentSelector.setSelectedId(p.currentInstrument, juce::dontSendNotification);

    instrumentSelector.onChange = [this] {
        audioProcessor.currentInstrument = instrumentSelector.getSelectedId();
        audioProcessor.instrumentChanged = true;
        };
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

    int colWidth = 320;
    int colGap = 40;
    int totalWidth = (colWidth * 2) + colGap;
    int startX = (mainArea.getWidth() - totalWidth) / 2;

    auto leftCol = mainArea.removeFromLeft(startX + colWidth).removeFromRight(colWidth);
    auto rightCol = mainArea.removeFromRight(startX + colWidth).removeFromLeft(colWidth);

    leftHandLabel.setBounds(leftCol.removeFromTop(30));
    rightHandLabel.setBounds(rightCol.removeFromTop(30));

    auto stackRow = [](juce::Rectangle<int>& area, juce::Component& c) {
        c.setBounds(area.removeFromTop(22));
        area.removeFromTop(4);
        };

    stackRow(leftCol, leftXRow);
    stackRow(leftCol, leftYRow);
    stackRow(leftCol, leftZRow);
    stackRow(leftCol, leftWristRow);
    stackRow(leftCol, leftGrabRow);
    stackRow(leftCol, leftPinchRow);
    leftCol.removeFromTop(10);
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
    rightCol.removeFromTop(10);
    stackRow(rightCol, rightThumbRow);
    stackRow(rightCol, rightIndexRow);
    stackRow(rightCol, rightMiddleRow);
    stackRow(rightCol, rightRingRow);
    stackRow(rightCol, rightPinkyRow);

    bool isMidi = (audioProcessor.currentOutputMode == OutputMode::MIDI_Only);
    instrumentSelector.setVisible(isMidi);
    instrumentLabel.setVisible(isMidi);



    if (isMidi) {
        instrumentSelector.setBounds(mainArea.getCentreX() - 250, mainArea.getBottom() - 40, 200, 25);
        invertTriggerButton.setBounds(mainArea.getCentreX() + 10, mainArea.getBottom() - 40, 250, 25);
    }

    closeButton.setBounds(getLocalBounds().getCentreX() - 50, getHeight() - 30, 100, 25);
}