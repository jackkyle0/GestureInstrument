#include "SettingsComponent.h"

SettingsComponent::SettingsComponent(GestureInstrumentAudioProcessor& p)
    : audioProcessor(p),
    sensitivityControl("Sensitivity", 0.1f, 5.0f, p.sensitivityLevel),
    minHeightControl("Min Height", 0.0f, 200.0f, p.minHeightThreshold),
    maxHeightControl("Max Height", 200.0f, 600.0f, p.maxHeightThreshold)
{
    setOpaque(true);

    // Convert Dropdown ID to Enum
    auto getTargetFromId = [](int id) -> GestureTarget {
        if (id == 1) return GestureTarget::Volume;
        if (id == 2) return GestureTarget::Pitch;
        if (id == 3) return GestureTarget::None;
        return GestureTarget::None;
        };

    // Left Hand callbacks
    leftXRow.comboBox.onChange = [this, getTargetFromId] { audioProcessor.leftXTarget = getTargetFromId(leftXRow.comboBox.getSelectedId()); };
    leftYRow.comboBox.onChange = [this, getTargetFromId] { audioProcessor.leftYTarget = getTargetFromId(leftYRow.comboBox.getSelectedId()); };
    leftZRow.comboBox.onChange = [this, getTargetFromId] { audioProcessor.leftZTarget = getTargetFromId(leftZRow.comboBox.getSelectedId()); };
    leftWristRow.comboBox.onChange = [this, getTargetFromId] { audioProcessor.leftRollTarget = getTargetFromId(leftWristRow.comboBox.getSelectedId()); };

    // Right Hand callbacks
    rightXRow.comboBox.onChange = [this, getTargetFromId] { audioProcessor.rightXTarget = getTargetFromId(rightXRow.comboBox.getSelectedId()); };
    rightYRow.comboBox.onChange = [this, getTargetFromId] { audioProcessor.rightYTarget = getTargetFromId(rightYRow.comboBox.getSelectedId()); };
    rightZRow.comboBox.onChange = [this, getTargetFromId] { audioProcessor.rightZTarget = getTargetFromId(rightZRow.comboBox.getSelectedId()); };
    rightWristRow.comboBox.onChange = [this, getTargetFromId] { audioProcessor.rightRollTarget = getTargetFromId(rightWristRow.comboBox.getSelectedId()); };

    // UI Elements
    addAndMakeVisible(titleLabel);
    titleLabel.setFont(juce::Font(24.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(leftXRow);
    addAndMakeVisible(leftYRow);
    addAndMakeVisible(leftZRow);
    addAndMakeVisible(leftWristRow);

    addAndMakeVisible(rightXRow);
    addAndMakeVisible(rightYRow);
    addAndMakeVisible(rightZRow);
    addAndMakeVisible(rightWristRow);

    addAndMakeVisible(sensitivityControl);
    addAndMakeVisible(minHeightControl);
    addAndMakeVisible(maxHeightControl);

    //// Scale controls
    //addAndMakeVisible(scaleLabel);
    //scaleLabel.setText("Scale Quantize:", juce::dontSendNotification);

    //addAndMakeVisible(rootSelector);
    //rootSelector.addItemList({ "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" }, 1);
    //rootSelector.setSelectedId(p.rootNote + 1);
    //rootSelector.onChange = [this] { audioProcessor.rootNote = rootSelector.getSelectedId() - 1; };

    //addAndMakeVisible(scaleSelector);
    //scaleSelector.addItem("Chromatic", 1);
    //scaleSelector.addItem("Major", 2);
    //scaleSelector.addItem("Minor", 3);
    //scaleSelector.addItem("Pentatonic", 4);
    //scaleSelector.setSelectedId(p.scaleType + 1);
    //scaleSelector.onChange = [this] { audioProcessor.scaleType = scaleSelector.getSelectedId() - 1; };

    //// Sliders
    //sensitivityControl.slider.onValueChange = [this] { audioProcessor.sensitivityLevel = (float)sensitivityControl.slider.getValue(); };
    //minHeightControl.slider.onValueChange = [this] { audioProcessor.minHeightThreshold = (float)minHeightControl.slider.getValue(); };
    //maxHeightControl.slider.onValueChange = [this] { audioProcessor.maxHeightThreshold = (float)maxHeightControl.slider.getValue(); };

    addAndMakeVisible(closeButton);
}

SettingsComponent::~SettingsComponent() {}

void SettingsComponent::paint(juce::Graphics& g) {
    g.fillAll(juce::Colours::black);
}

void SettingsComponent::resized() {
    auto bounds = getLocalBounds().reduced(40);

    juce::FlexBox masterBox;
    masterBox.flexDirection = juce::FlexBox::Direction::column;
    masterBox.justifyContent = juce::FlexBox::JustifyContent::flexStart;

    // Title
    masterBox.items.add(juce::FlexItem(titleLabel).withHeight(40));

    // add rows
    auto addRow = [&](juce::Component& c) {
        masterBox.items.add(juce::FlexItem(c).withHeight(30).withMargin({ 0, 0, 5, 0 }));
        };

    // Left hand
    addRow(leftXRow);
    addRow(leftYRow);
    addRow(leftZRow);
    addRow(leftWristRow);

    masterBox.items.add(juce::FlexItem().withHeight(15)); // Spacer

    // Right Hand
    addRow(rightXRow);
    addRow(rightYRow);
    addRow(rightZRow);
    addRow(rightWristRow);

    masterBox.items.add(juce::FlexItem().withHeight(15)); // Spacer

    //// Scale controls row
    //juce::FlexBox scaleRow;
    //scaleRow.flexDirection = juce::FlexBox::Direction::row;
    //scaleRow.alignItems = juce::FlexBox::AlignItems::center; // Vertically align text and boxes

    //// Add Label
    //scaleRow.items.add(juce::FlexItem(scaleLabel).withWidth(100));

    //// Add Root Selector
    //scaleRow.items.add(juce::FlexItem(rootSelector).withWidth(80).withMargin({ 0, 10, 0, 0 }));

    //// Add Scale Type Selector
    //scaleRow.items.add(juce::FlexItem(scaleSelector).withWidth(120));

    //// Add the whole row to the master box
    //masterBox.items.add(juce::FlexItem(scaleRow).withHeight(30).withMargin({ 0, 0, 20, 0 }));

    // Sliders
    auto addSlider = [&](juce::Component& c) {
        masterBox.items.add(juce::FlexItem(c).withHeight(50).withMargin({ 0, 0, 10, 0 }));
        };
    addSlider(sensitivityControl);
    addSlider(minHeightControl);
    addSlider(maxHeightControl);

    masterBox.performLayout(bounds);

    closeButton.setBounds(getLocalBounds().getCentreX() - 50, getHeight() - 50, 100, 30);
}