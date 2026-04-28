#include "SettingsComponent.h"

SettingsComponent::SettingsComponent(GestureInstrumentAudioProcessor& p)
    : audioProcessor(p)
{
    setOpaque(true);

    addAndMakeVisible(mpeRoutingLabel);
    mpeRoutingLabel.setColour(juce::Label::textColourId, juce::Colours::orange);

    auto setupMpeBox = [&](juce::ComboBox& cb, juce::Label& lbl, std::atomic<int>& target) {
        addAndMakeVisible(lbl);
        addAndMakeVisible(cb);
        cb.addItem("Finger X-Axis", 1);
        cb.addItem("Finger Y-Axis", 2);
        cb.addItem("Finger Z-Axis", 3);
        cb.setSelectedId(target.load(), juce::dontSendNotification);
        cb.onChange = [this, &cb, &target] { target.store(cb.getSelectedId()); };
        };

    setupMpeBox(mpePitchSelector, mpePitchLabel, audioProcessor.mpePitchBendAxis);
    setupMpeBox(mpeTimbreSelector, mpeTimbreLabel, audioProcessor.mpeTimbreAxis);
    setupMpeBox(mpePressureSelector, mpePressureLabel, audioProcessor.mpePressureAxis);

    addAndMakeVisible(mpeButton);
    mpeButton.setToggleState(p.isMpeEnabled, juce::dontSendNotification);
    mpeButton.setColour(juce::ToggleButton::textColourId, juce::Colours::white);

    mpeButton.onClick = [this] {
        audioProcessor.isMpeEnabled = mpeButton.getToggleState();
        bool isMpe = audioProcessor.isMpeEnabled;

        leftThumbRow.setEnabled(!isMpe); leftIndexRow.setEnabled(!isMpe); leftMiddleRow.setEnabled(!isMpe); leftRingRow.setEnabled(!isMpe); leftPinkyRow.setEnabled(!isMpe);
        rightThumbRow.setEnabled(!isMpe); rightIndexRow.setEnabled(!isMpe); rightMiddleRow.setEnabled(!isMpe); rightRingRow.setEnabled(!isMpe); rightPinkyRow.setEnabled(!isMpe);

        mpeRoutingLabel.setVisible(isMpe);
        mpePitchLabel.setVisible(isMpe); mpePitchSelector.setVisible(isMpe);
        mpeTimbreLabel.setVisible(isMpe); mpeTimbreSelector.setVisible(isMpe);
        mpePressureLabel.setVisible(isMpe); mpePressureSelector.setVisible(isMpe);
        };

    addAndMakeVisible(splitXAxisToggle);
    splitXAxisToggle.setToggleState(audioProcessor.enableSplitXAxis.load(), juce::dontSendNotification);
    splitXAxisToggle.setColour(juce::ToggleButton::textColourId, juce::Colours::white);
    splitXAxisToggle.onClick = [this] { audioProcessor.enableSplitXAxis.store(splitXAxisToggle.getToggleState()); };

    addAndMakeVisible(floorShadowToggle);
    floorShadowToggle.setToggleState(audioProcessor.showFloorShadow, juce::dontSendNotification);
    floorShadowToggle.setColour(juce::ToggleButton::textColourId, juce::Colours::white);
    floorShadowToggle.onClick = [this] { audioProcessor.showFloorShadow = floorShadowToggle.getToggleState(); };

    addAndMakeVisible(wallShadowToggle);
    wallShadowToggle.setToggleState(audioProcessor.showWallShadow, juce::dontSendNotification);
    wallShadowToggle.setColour(juce::ToggleButton::textColourId, juce::Colours::white);
    wallShadowToggle.onClick = [this] { audioProcessor.showWallShadow = wallShadowToggle.getToggleState(); };

    addAndMakeVisible(invertTriggerButton);
    invertTriggerButton.setToggleState(audioProcessor.invertNoteTrigger, juce::dontSendNotification);
    invertTriggerButton.onClick = [this] {
        audioProcessor.invertNoteTrigger = invertTriggerButton.getToggleState();
        };

    addAndMakeVisible(modeSelector);
    modeSelector.addItem("OSC", 1);
    modeSelector.addItem("MIDI", 2);
    modeSelector.setSelectedId(audioProcessor.currentOutputMode == OutputMode::OSC_Only ? 1 : 2, juce::dontSendNotification);

    modeSelector.onChange = [this] {
        audioProcessor.currentOutputMode = (modeSelector.getSelectedId() == 1) ? OutputMode::OSC_Only : OutputMode::MIDI_Only;

        bool isMidi = (audioProcessor.currentOutputMode == OutputMode::MIDI_Only);

        bool isStandalone = juce::JUCEApplicationBase::isStandaloneApp();
        instrumentSelector.setVisible(isStandalone);
        instrumentLabel.setVisible(isStandalone);

        instrumentSelector.setEnabled(isMidi && isStandalone);
        instrumentLabel.setEnabled(isMidi && isStandalone);
        invertTriggerButton.setEnabled(isMidi);
        mpeButton.setEnabled(isMidi);

        bool isOsc = !isMidi;
        leftXRow.updateList(isOsc); leftYRow.updateList(isOsc); leftZRow.updateList(isOsc);
        leftWristRow.updateList(isOsc); leftGrabRow.updateList(isOsc); leftPinchRow.updateList(isOsc);
        leftThumbRow.updateList(isOsc); leftIndexRow.updateList(isOsc); leftMiddleRow.updateList(isOsc);
        leftRingRow.updateList(isOsc); leftPinkyRow.updateList(isOsc);

        rightXRow.updateList(isOsc); rightYRow.updateList(isOsc); rightZRow.updateList(isOsc);
        rightWristRow.updateList(isOsc); rightGrabRow.updateList(isOsc); rightPinchRow.updateList(isOsc);
        rightThumbRow.updateList(isOsc); rightIndexRow.updateList(isOsc); rightMiddleRow.updateList(isOsc);
        rightRingRow.updateList(isOsc); rightPinkyRow.updateList(isOsc);
        };

    modeSelector.onChange();

    addAndMakeVisible(visualsLabel);
    visualsLabel.setText("VISUALS", juce::dontSendNotification);
    visualsLabel.setColour(juce::Label::textColourId, juce::Colours::orange);
    visualsLabel.setFont(juce::Font(14.0f, juce::Font::bold));

    addAndMakeVisible(standaloneLabel);
    standaloneLabel.setText("STANDALONE SYNTH", juce::dontSendNotification);
    standaloneLabel.setColour(juce::Label::textColourId, juce::Colours::orange);
    standaloneLabel.setFont(juce::Font(14.0f, juce::Font::bold));

    addAndMakeVisible(modeLabel);
    modeLabel.setText("Output Mode:", juce::dontSendNotification);

    addAndMakeVisible(enableGestureSwitchButton);
    enableGestureSwitchButton.setToggleState(audioProcessor.isGestureToMouseEnabled.load(), juce::dontSendNotification);
    enableGestureSwitchButton.setColour(juce::ToggleButton::textColourId, juce::Colours::white);
    enableGestureSwitchButton.onClick = [this] { audioProcessor.isGestureToMouseEnabled.store(enableGestureSwitchButton.getToggleState()); };

    addAndMakeVisible(gestureTimerSlider);
    gestureTimerSlider.setRange(0.5, 5.0, 0.1);
    gestureTimerSlider.setValue(1.5);
    gestureTimerSlider.setTextValueSuffix("s");
    gestureTimerSlider.onValueChange = [this] { audioProcessor.virtualMouseHoldTime = (float)gestureTimerSlider.getValue(); };

    addAndMakeVisible(gestureTypeSelector);
    gestureTypeSelector.addItem("Both Fists", 1);
    gestureTypeSelector.addItem("Right Fist", 2);
    gestureTypeSelector.addItem("Left Fist", 3);
    gestureTypeSelector.setSelectedId(1);
    gestureTypeSelector.onChange = [this] { audioProcessor.virtualMouseGestureType = gestureTypeSelector.getSelectedId(); };

    addAndMakeVisible(savePresetButton);
    savePresetButton.onClick = [this] {
        fileChooser = std::make_unique<juce::FileChooser>("Save Preset", lastPresetDirectory, "*.xml");
        fileChooser->launchAsync(juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
            [this](const juce::FileChooser& fc) {
                juce::File file = fc.getResult();
                if (file != juce::File{}) {
                    lastPresetDirectory = file.getParentDirectory();
                    auto xml = audioProcessor.createPresetXml();
                    xml->writeTo(file);
                }
            });
        };

    addAndMakeVisible(loadPresetButton);
    loadPresetButton.onClick = [this] {
        fileChooser = std::make_unique<juce::FileChooser>("Load Preset", lastPresetDirectory, "*.xml");
        fileChooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this](const juce::FileChooser& fc) {
                juce::File file = fc.getResult();
                if (file.existsAsFile()) {
                    lastPresetDirectory = file.getParentDirectory();
                    auto xml = juce::XmlDocument::parse(file);
                    audioProcessor.loadPresetXml(xml.get());
                    modeSelector.setSelectedId(audioProcessor.currentOutputMode == OutputMode::OSC_Only ? 1 : 2, juce::dontSendNotification);
                    if (onPresetLoaded) onPresetLoaded();
                }
            });
        };

    auto setupRow = [&](MappingRow& row, GestureTarget& target) {
        addAndMakeVisible(row);
        row.comboBox.setSelectedId(this->getIdFromTarget(target), juce::dontSendNotification);
        row.comboBox.onChange = [this, &row, &target] { target = this->getTargetFromId(row.comboBox.getSelectedId()); };
        };

    setupRow(leftXRow, audioProcessor.leftXTarget); setupRow(leftYRow, audioProcessor.leftYTarget); setupRow(leftZRow, audioProcessor.leftZTarget);
    setupRow(leftWristRow, audioProcessor.leftRollTarget); setupRow(leftGrabRow, audioProcessor.leftGrabTarget); setupRow(leftPinchRow, audioProcessor.leftPinchTarget);
    setupRow(leftThumbRow, audioProcessor.leftThumbTarget); setupRow(leftIndexRow, audioProcessor.leftIndexTarget); setupRow(leftMiddleRow, audioProcessor.leftMiddleTarget);
    setupRow(leftRingRow, audioProcessor.leftRingTarget); setupRow(leftPinkyRow, audioProcessor.leftPinkyTarget);

    setupRow(rightXRow, audioProcessor.rightXTarget); setupRow(rightYRow, audioProcessor.rightYTarget); setupRow(rightZRow, audioProcessor.rightZTarget);
    setupRow(rightWristRow, audioProcessor.rightRollTarget); setupRow(rightGrabRow, audioProcessor.rightGrabTarget); setupRow(rightPinchRow, audioProcessor.rightPinchTarget);
    setupRow(rightThumbRow, audioProcessor.rightThumbTarget); setupRow(rightIndexRow, audioProcessor.rightIndexTarget); setupRow(rightMiddleRow, audioProcessor.rightMiddleTarget);
    setupRow(rightRingRow, audioProcessor.rightRingTarget); setupRow(rightPinkyRow, audioProcessor.rightPinkyTarget);

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
    instrumentSelector.addItem("Piano", 1); instrumentSelector.addItem("Warm Pad", 90); instrumentSelector.addItem("Choir Pad", 92);
    instrumentSelector.addItem("Sci-Fi", 97); instrumentSelector.addItem("Atmosphere", 100);
    instrumentSelector.setSelectedId(p.currentInstrument, juce::dontSendNotification);
    instrumentSelector.onChange = [this] {
        audioProcessor.currentInstrument = instrumentSelector.getSelectedId();
        audioProcessor.instrumentChanged = true;
        };
}

SettingsComponent::~SettingsComponent() {}

void SettingsComponent::paint(juce::Graphics& g) {
    g.fillAll(juce::Colours::black);
    auto bounds = getLocalBounds().reduced(20);
    bounds.removeFromTop(60); bounds.removeFromBottom(60);

    int spacing = 20;
    int colW = (bounds.getWidth() - (spacing * 3)) / 4;

    auto drawPanel = [&](juce::Rectangle<int> area, juce::String title) {
        g.setColour(juce::Colour::fromFloatRGBA(0.1f, 0.1f, 0.12f, 1.0f));
        g.fillRoundedRectangle(area.toFloat(), 10.0f);
        g.setColour(juce::Colours::white.withAlpha(0.8f));
        g.setFont(juce::Font(18.0f, juce::Font::bold));
        g.drawText(title, area.withY(area.getY() + 15).withHeight(20), juce::Justification::centredTop);
        g.setColour(juce::Colours::white.withAlpha(0.1f));
        g.drawLine((float)area.getX() + 20, (float)area.getY() + 45, (float)area.getRight() - 20, (float)area.getY() + 45, 2.0f);
        };

    auto col1 = bounds.removeFromLeft(colW); bounds.removeFromLeft(spacing);
    auto col2 = bounds.removeFromLeft(colW); bounds.removeFromLeft(spacing);
    auto col3 = bounds.removeFromLeft(colW); bounds.removeFromLeft(spacing);
    auto col4 = bounds.removeFromLeft(colW);

    drawPanel(col1, "SYSTEM AND VISUALS");
    drawPanel(col2, "LEFT HAND");
    drawPanel(col3, "RIGHT HAND");
    drawPanel(col4, "MIDI AND MPE");
}

void SettingsComponent::resized() {
    auto bounds = getLocalBounds().reduced(20);
    titleLabel.setBounds(bounds.removeFromTop(40));

    auto bottomArea = bounds.removeFromBottom(60);
    closeButton.setBounds(bottomArea.getCentreX() - 50, bottomArea.getY() + 15, 100, 30);
    bounds.removeFromTop(20);

    int spacing = 20;
    int colW = (bounds.getWidth() - (spacing * 3)) / 4;

    // THE FIX PART 1: Only shrink the left/right padding here, NOT the top/bottom!
    auto col1 = bounds.removeFromLeft(colW).reduced(15, 0); bounds.removeFromLeft(spacing);
    auto col2 = bounds.removeFromLeft(colW).reduced(15, 0); bounds.removeFromLeft(spacing);
    auto col3 = bounds.removeFromLeft(colW).reduced(15, 0); bounds.removeFromLeft(spacing);
    auto col4 = bounds.removeFromLeft(colW).reduced(15, 0);

    // THE FIX PART 2: Manually push every column down by 55px so they safely clear the painted titles and line!
    auto prepareCol = [](juce::Rectangle<int>& col) {
        col.removeFromTop(55);     // Clears the header line
        col.removeFromBottom(15);  // Gives a little padding at the floor
        };
    prepareCol(col1); prepareCol(col2); prepareCol(col3); prepareCol(col4);

    // ==========================================
    // COLUMN 1: SYSTEM & VISUALS
    // ==========================================
    modeLabel.setBounds(col1.removeFromTop(25));
    modeSelector.setBounds(col1.removeFromTop(25));
    col1.removeFromTop(15);

    savePresetButton.setBounds(col1.removeFromTop(25));
    col1.removeFromTop(5);
    loadPresetButton.setBounds(col1.removeFromTop(25));

    col1.removeFromTop(30);

    visualsLabel.setBounds(col1.removeFromTop(20));
    floorShadowToggle.setBounds(col1.removeFromTop(25));
    wallShadowToggle.setBounds(col1.removeFromTop(25));
    splitXAxisToggle.setBounds(col1.removeFromTop(25));

    // ==========================================
    // COLUMNS 2 & 3: HAND MAPPINGS
    // ==========================================
    // THE FIX PART 3: Make the rows 35px high with a 5px gap so all 11 rows fit safely on screen!
    auto stackRow = [](juce::Rectangle<int>& area, juce::Component& c) {
        c.setBounds(area.removeFromTop(35));
        area.removeFromTop(5);
        };

    stackRow(col2, leftXRow); stackRow(col2, leftYRow); stackRow(col2, leftZRow);
    stackRow(col2, leftWristRow); stackRow(col2, leftGrabRow); stackRow(col2, leftPinchRow);
    col2.removeFromTop(8); // Small visual gap between spatial and finger mappings
    stackRow(col2, leftThumbRow); stackRow(col2, leftIndexRow); stackRow(col2, leftMiddleRow);
    stackRow(col2, leftRingRow); stackRow(col2, leftPinkyRow);

    stackRow(col3, rightXRow); stackRow(col3, rightYRow); stackRow(col3, rightZRow);
    stackRow(col3, rightWristRow); stackRow(col3, rightGrabRow); stackRow(col3, rightPinchRow);
    col3.removeFromTop(8);
    stackRow(col3, rightThumbRow); stackRow(col3, rightIndexRow); stackRow(col3, rightMiddleRow);
    stackRow(col3, rightRingRow); stackRow(col3, rightPinkyRow);

    // ==========================================
    // COLUMN 4: MIDI & MPE
    // ==========================================
    if (instrumentSelector.isVisible()) {
        standaloneLabel.setBounds(col4.removeFromTop(20));
        instrumentLabel.setBounds(col4.removeFromTop(20));
        instrumentSelector.setBounds(col4.removeFromTop(25));
        col4.removeFromTop(20);
    }

    invertTriggerButton.setBounds(col4.removeFromTop(25));
    col4.removeFromTop(20);

    mpeButton.setBounds(col4.removeFromTop(25));
    col4.removeFromTop(10);

    mpeRoutingLabel.setBounds(col4.removeFromTop(20));
    mpePitchLabel.setBounds(col4.removeFromTop(20));
    mpePitchSelector.setBounds(col4.removeFromTop(25));

    col4.removeFromTop(5);
    mpeTimbreLabel.setBounds(col4.removeFromTop(20));
    mpeTimbreSelector.setBounds(col4.removeFromTop(25));

    col4.removeFromTop(5);
    mpePressureLabel.setBounds(col4.removeFromTop(20));
    mpePressureSelector.setBounds(col4.removeFromTop(25));
}

void SettingsComponent::refreshUI() {
    modeSelector.setSelectedId(audioProcessor.currentOutputMode == OutputMode::OSC_Only ? 1 : 2, juce::dontSendNotification);
    instrumentSelector.setSelectedId(audioProcessor.currentInstrument, juce::dontSendNotification);
    invertTriggerButton.setToggleState(audioProcessor.invertNoteTrigger, juce::dontSendNotification);
    mpeButton.setToggleState(audioProcessor.isMpeEnabled, juce::dontSendNotification);
    floorShadowToggle.setToggleState(audioProcessor.showFloorShadow, juce::dontSendNotification);
    wallShadowToggle.setToggleState(audioProcessor.showWallShadow, juce::dontSendNotification);
    splitXAxisToggle.setToggleState(audioProcessor.enableSplitXAxis.load(), juce::dontSendNotification);
    enableGestureSwitchButton.setToggleState(audioProcessor.isGestureToMouseEnabled.load(), juce::dontSendNotification);
    gestureTypeSelector.setSelectedId(audioProcessor.virtualMouseGestureType, juce::dontSendNotification);
    gestureTimerSlider.setValue(audioProcessor.virtualMouseHoldTime, juce::dontSendNotification);

    mpeButton.onClick(); // Forces the enable/disable logic to refresh correctly

    bool isOsc = (audioProcessor.currentOutputMode == OutputMode::OSC_Only);
    leftXRow.updateList(isOsc); leftYRow.updateList(isOsc); leftZRow.updateList(isOsc);
    leftWristRow.updateList(isOsc); leftGrabRow.updateList(isOsc); leftPinchRow.updateList(isOsc);
    leftThumbRow.updateList(isOsc); leftIndexRow.updateList(isOsc); leftMiddleRow.updateList(isOsc);
    leftRingRow.updateList(isOsc); leftPinkyRow.updateList(isOsc);

    rightXRow.updateList(isOsc); rightYRow.updateList(isOsc); rightZRow.updateList(isOsc);
    rightWristRow.updateList(isOsc); rightGrabRow.updateList(isOsc); rightPinchRow.updateList(isOsc);
    rightThumbRow.updateList(isOsc); rightIndexRow.updateList(isOsc); rightMiddleRow.updateList(isOsc);
    rightRingRow.updateList(isOsc); rightPinkyRow.updateList(isOsc);

    auto syncRow = [&](MappingRow& row, GestureTarget target) {
        row.comboBox.setSelectedId(getIdFromTarget(target), juce::dontSendNotification);
        };

    syncRow(leftXRow, audioProcessor.leftXTarget); syncRow(leftYRow, audioProcessor.leftYTarget); syncRow(leftZRow, audioProcessor.leftZTarget);
    syncRow(leftWristRow, audioProcessor.leftRollTarget); syncRow(leftGrabRow, audioProcessor.leftGrabTarget); syncRow(leftPinchRow, audioProcessor.leftPinchTarget);
    syncRow(leftThumbRow, audioProcessor.leftThumbTarget); syncRow(leftIndexRow, audioProcessor.leftIndexTarget); syncRow(leftMiddleRow, audioProcessor.leftMiddleTarget);
    syncRow(leftRingRow, audioProcessor.leftRingTarget); syncRow(leftPinkyRow, audioProcessor.leftPinkyTarget);

    syncRow(rightXRow, audioProcessor.rightXTarget); syncRow(rightYRow, audioProcessor.rightYTarget); syncRow(rightZRow, audioProcessor.rightZTarget);
    syncRow(rightWristRow, audioProcessor.rightRollTarget); syncRow(rightGrabRow, audioProcessor.rightGrabTarget); syncRow(rightPinchRow, audioProcessor.rightPinchTarget);
    syncRow(rightThumbRow, audioProcessor.rightThumbTarget); syncRow(rightIndexRow, audioProcessor.rightIndexTarget); syncRow(rightMiddleRow, audioProcessor.rightMiddleTarget);
    syncRow(rightRingRow, audioProcessor.rightRingTarget); syncRow(rightPinkyRow, audioProcessor.rightPinkyTarget);

    repaint();
}

int SettingsComponent::getIdFromTarget(GestureTarget target) {
    switch (target) {
    case GestureTarget::None: return 1;
    case GestureTarget::Volume: return 2;
    case GestureTarget::Pitch: return 3;
    case GestureTarget::NoteTrigger: return 4;
    case GestureTarget::Modulation: return 5;
    case GestureTarget::Expression: return 6;
    case GestureTarget::Breath: return 7;
    case GestureTarget::Cutoff: return 8;
    case GestureTarget::Resonance: return 9;
    case GestureTarget::Attack: return 10;
    case GestureTarget::Release: return 11;
    case GestureTarget::Vibrato: return 12;
    case GestureTarget::Pan: return 13;
    case GestureTarget::Reverb: return 14;
    case GestureTarget::Chorus: return 15;
    case GestureTarget::Sustain: return 16;
    case GestureTarget::Portamento: return 17;
    case GestureTarget::Waveform: return 18;
    case GestureTarget::Delay: return 19;
    case GestureTarget::Distortion: return 20;
    default: return 1;
    }
}

GestureTarget SettingsComponent::getTargetFromId(int id) {
    switch (id) {
    case 1: return GestureTarget::None;
    case 2: return GestureTarget::Volume;
    case 3: return GestureTarget::Pitch;
    case 4: return GestureTarget::NoteTrigger;
    case 5: return GestureTarget::Modulation;
    case 6: return GestureTarget::Expression;
    case 7: return GestureTarget::Breath;
    case 8: return GestureTarget::Cutoff;
    case 9: return GestureTarget::Resonance;
    case 10: return GestureTarget::Attack;
    case 11: return GestureTarget::Release;
    case 12: return GestureTarget::Vibrato;
    case 13: return GestureTarget::Pan;
    case 14: return GestureTarget::Reverb;
    case 15: return GestureTarget::Chorus;
    case 16: return GestureTarget::Sustain;
    case 17: return GestureTarget::Portamento;
    case 18: return GestureTarget::Waveform;
    case 19: return GestureTarget::Delay;
    case 20: return GestureTarget::Distortion;
    default: return GestureTarget::None;
    }
}