#include "SettingsComponent.h"

SettingsComponent::SettingsComponent(GestureInstrumentAudioProcessor& p)
    : audioProcessor(p),
    sensitivityControl("Sensitivity", 0.1f, 5.0f, p.sensitivityLevel),
    minHeightControl("Min Height", 0.0f, 200.0f, p.minHeightThreshold),
    maxHeightControl("Max Height", 200.0f, 600.0f, p.maxHeightThreshold)
{
    addAndMakeVisible(titleLabel);
    titleLabel.setFont(juce::Font(24.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centred);

    // Add Left Hand Rows
    addAndMakeVisible(leftXRow);
    addAndMakeVisible(leftYRow);
    addAndMakeVisible(leftZRow);
    addAndMakeVisible(leftWristRow);

    // Add Right Hand Rows
    addAndMakeVisible(rightXRow);
    addAndMakeVisible(rightYRow);
    addAndMakeVisible(rightZRow);
    addAndMakeVisible(rightWristRow);

    // Add Sliders
    addAndMakeVisible(sensitivityControl);
    addAndMakeVisible(minHeightControl);
    addAndMakeVisible(maxHeightControl);

    // Setup Callbacks 
    sensitivityControl.slider.onValueChange = [this] { audioProcessor.sensitivityLevel = (float)sensitivityControl.slider.getValue(); };
    minHeightControl.slider.onValueChange = [this] { audioProcessor.minHeightThreshold = (float)minHeightControl.slider.getValue(); };
    maxHeightControl.slider.onValueChange = [this] { audioProcessor.maxHeightThreshold = (float)maxHeightControl.slider.getValue(); };

    addAndMakeVisible(closeButton);
    setOpaque(true);
}

SettingsComponent::~SettingsComponent() {}

void SettingsComponent::paint(juce::Graphics& g) {
    g.fillAll(juce::Colours::black.withAlpha(0.95f));
}

void SettingsComponent::resized() {
    auto bounds = getLocalBounds().reduced(40);

    juce::FlexBox masterBox;
    masterBox.flexDirection = juce::FlexBox::Direction::column;
    masterBox.justifyContent = juce::FlexBox::JustifyContent::flexStart;

    // Title
    masterBox.items.add(juce::FlexItem(titleLabel).withHeight(40));

    // Helper to add rows quickly with margin
    auto addRow = [&](juce::Component& c) {
        masterBox.items.add(juce::FlexItem(c).withHeight(30).withMargin({ 0, 0, 5, 0 }));
        };

    // Left Hand
    addRow(leftXRow);
    addRow(leftYRow);
    addRow(leftZRow);
    addRow(leftWristRow);

    // Spacer
    masterBox.items.add(juce::FlexItem().withHeight(20));

    // Right Hand 
    addRow(rightXRow);
    addRow(rightYRow);
    addRow(rightZRow);
    addRow(rightWristRow);

    // Spacer
    masterBox.items.add(juce::FlexItem().withHeight(20));

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