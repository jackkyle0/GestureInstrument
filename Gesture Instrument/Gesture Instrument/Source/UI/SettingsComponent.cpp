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

    // Mode selector updated ****
    addAndMakeVisible(modeSelector);
    modeSelector.addItem("OSC", 1);
    modeSelector.addItem("MIDI", 2);
    modeSelector.setSelectedId(audioProcessor.currentOutputMode == OutputMode::OSC_Only ? 1 : 2, juce::dontSendNotification);
    modeSelector.onChange = [this] {
        audioProcessor.currentOutputMode = (modeSelector.getSelectedId() == 1) ? OutputMode::OSC_Only : OutputMode::MIDI_Only;

        bool isMidi = (audioProcessor.currentOutputMode == OutputMode::MIDI_Only);
        instrumentSelector.setVisible(isMidi);
        instrumentLabel.setVisible(isMidi);
        };

    addAndMakeVisible(modeLabel);
    modeLabel.setText("Output Mode:", juce::dontSendNotification);
    modeLabel.attachToComponent(&modeSelector, true);

    // Virtual Mouse
    addAndMakeVisible(enableGestureSwitchButton);
    enableGestureSwitchButton.setToggleState(true, juce::dontSendNotification);
    enableGestureSwitchButton.setColour(juce::ToggleButton::textColourId, juce::Colours::white);

    addAndMakeVisible(gestureTimerSlider);
    gestureTimerSlider.setRange(0.5, 5.0, 0.1);
    gestureTimerSlider.setValue(1.5);
    gestureTimerSlider.setTextValueSuffix("s");

    addAndMakeVisible(gestureTypeSelector);
    gestureTypeSelector.addItem("Both Fists", 1);
    gestureTypeSelector.addItem("Right Fist", 2);
    gestureTypeSelector.addItem("Left Fist", 3);
    gestureTypeSelector.setSelectedId(1);

    // Presetsss
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

    addAndMakeVisible(mpeButton);
    mpeButton.setToggleState(p.isMpeEnabled, juce::dontSendNotification);
    mpeButton.setColour(juce::ToggleButton::textColourId, juce::Colours::white);

    mpeButton.onClick = [this] {
        audioProcessor.isMpeEnabled = mpeButton.getToggleState();

        //// Disable dropdowns when MPE enabled
        //bool useDropdowns = !audioProcessor.isMpeEnabled;
        //leftThumbRow.setEnabled(useDropdowns);
        //leftIndexRow.setEnabled(useDropdowns);
        //leftMiddleRow.setEnabled(useDropdowns);
        //leftRingRow.setEnabled(useDropdowns);
        //leftPinkyRow.setEnabled(useDropdowns);

        //rightThumbRow.setEnabled(useDropdowns);
        //rightIndexRow.setEnabled(useDropdowns);
        //rightMiddleRow.setEnabled(useDropdowns);
        //rightRingRow.setEnabled(useDropdowns);
        //rightPinkyRow.setEnabled(useDropdowns);
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
        case 17: return GestureTarget::Waveform;
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
        case GestureTarget::Waveform: return 17;
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

    auto bounds = getLocalBounds().reduced(20);
    bounds.removeFromTop(60);
    bounds.removeFromBottom(60);

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

    drawPanel(col1, "SYSTEM & PRESETS");
    drawPanel(col2, "LEFT HAND");
    drawPanel(col3, "RIGHT HAND");
    drawPanel(col4, "MIDI & ADVANCED");
}

void SettingsComponent::resized() {
    auto bounds = getLocalBounds().reduced(20);

    // Top Title
    titleLabel.setBounds(bounds.removeFromTop(40));

    auto bottomArea = bounds.removeFromBottom(60);
    closeButton.setBounds(bottomArea.getCentreX() - 50, bottomArea.getY() + 15, 100, 30);

    bounds.removeFromTop(20);

    int spacing = 20;
    int colW = (bounds.getWidth() - (spacing * 3)) / 4;

    auto col1 = bounds.removeFromLeft(colW).reduced(20, 50); bounds.removeFromLeft(spacing);
    auto col2 = bounds.removeFromLeft(colW).reduced(20, 50); bounds.removeFromLeft(spacing);
    auto col3 = bounds.removeFromLeft(colW).reduced(20, 50); bounds.removeFromLeft(spacing);
    auto col4 = bounds.removeFromLeft(colW).reduced(20, 50);

    // Col 1
    modeLabel.setBounds(col1.removeFromTop(25));
    modeSelector.setBounds(col1.removeFromTop(25));
    col1.removeFromTop(20);

    savePresetButton.setBounds(col1.removeFromTop(25));
    col1.removeFromTop(5);
    loadPresetButton.setBounds(col1.removeFromTop(25));
    col1.removeFromTop(20);

    enableGestureSwitchButton.setBounds(col1.removeFromTop(25));
    gestureTypeSelector.setBounds(col1.removeFromTop(25));
    col1.removeFromTop(5);
    gestureTimerSlider.setBounds(col1.removeFromTop(25));

    // Col 2
    leftHandLabel.setVisible(false);
    auto stackRow = [](juce::Rectangle<int>& area, juce::Component& c) {
        c.setBounds(area.removeFromTop(24));
        area.removeFromTop(6);
        };

    stackRow(col2, leftXRow); stackRow(col2, leftYRow); stackRow(col2, leftZRow);
    stackRow(col2, leftWristRow); stackRow(col2, leftGrabRow); stackRow(col2, leftPinchRow);
    col2.removeFromTop(10);
    stackRow(col2, leftThumbRow); stackRow(col2, leftIndexRow); stackRow(col2, leftMiddleRow);
    stackRow(col2, leftRingRow); stackRow(col2, leftPinkyRow);

    // Col 3
    rightHandLabel.setVisible(false);
    stackRow(col3, rightXRow); stackRow(col3, rightYRow); stackRow(col3, rightZRow);
    stackRow(col3, rightWristRow); stackRow(col3, rightGrabRow); stackRow(col3, rightPinchRow);
    col3.removeFromTop(10);
    stackRow(col3, rightThumbRow); stackRow(col3, rightIndexRow); stackRow(col3, rightMiddleRow);
    stackRow(col3, rightRingRow); stackRow(col3, rightPinkyRow);

    // Col 4
    bool isMidi = (audioProcessor.currentOutputMode == OutputMode::MIDI_Only);
    instrumentLabel.setVisible(isMidi);
    instrumentSelector.setVisible(isMidi);
    mpeButton.setVisible(isMidi); 

    if (isMidi) {
        instrumentLabel.setBounds(col4.removeFromTop(25));
        instrumentSelector.setBounds(col4.removeFromTop(25));
        col4.removeFromTop(20);

        mpeButton.setBounds(col4.removeFromTop(25));
        col4.removeFromTop(20); 

        invertTriggerButton.setBounds(col4.removeFromTop(25));
    }
}