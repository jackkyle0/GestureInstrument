#include "SettingsComponent.h"

SettingsComponent::SettingsComponent(GestureInstrumentAudioProcessor& p)
    : audioProcessor(p)
{
    // Setup Titles
    addAndMakeVisible(titleLabel);
    titleLabel.setFont(juce::Font(24.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(handAxisTitle);
    handAxisTitle.setFont(juce::Font(16.0f, juce::Font::bold));
    handAxisTitle.setColour(juce::Label::textColourId, juce::Colours::cyan);

    addAndMakeVisible(mappedParamTitle);
    mappedParamTitle.setFont(juce::Font(16.0f, juce::Font::bold));
    mappedParamTitle.setColour(juce::Label::textColourId, juce::Colours::cyan);

    // Hands text
    addAndMakeVisible(leftXLabel);
    addAndMakeVisible(leftYLabel);
    addAndMakeVisible(rightXLabel);
    addAndMakeVisible(rightYLabel);

    // Dropdowns
    setupComboBox(leftXBox, 3); 
    setupComboBox(leftYBox, 1);  
    setupComboBox(rightXBox, 2);
    setupComboBox(rightYBox, 3); 

    // Sliders

    // Sensitivity
    addAndMakeVisible(sensitivityLabel);
    addAndMakeVisible(sensitivitySlider);
    sensitivitySlider.setRange(0.1, 5.0, 0.1);
    sensitivitySlider.setValue(audioProcessor.sensitivityLevel);
    sensitivitySlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 50, 20);
    sensitivitySlider.onValueChange = [this] { audioProcessor.sensitivityLevel = (float)sensitivitySlider.getValue(); };

    // Min Height
    addAndMakeVisible(minHeightLabel);
    addAndMakeVisible(minHeightSlider);
    minHeightSlider.setRange(0.0, 200.0, 1.0);
    minHeightSlider.setValue(audioProcessor.minHeightThreshold);
    minHeightSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 50, 20);
    minHeightSlider.onValueChange = [this] { audioProcessor.minHeightThreshold = (float)minHeightSlider.getValue(); };

    // Max Height
    addAndMakeVisible(maxHeightLabel);
    addAndMakeVisible(maxHeightSlider);
    maxHeightSlider.setRange(200.0, 600.0, 1.0);
    maxHeightSlider.setValue(audioProcessor.maxHeightThreshold);
    maxHeightSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 50, 20);
    maxHeightSlider.onValueChange = [this] { audioProcessor.maxHeightThreshold = (float)maxHeightSlider.getValue(); };

    addAndMakeVisible(closeButton);

    setOpaque(true); // Background colour
}

SettingsComponent::~SettingsComponent() {
}

void SettingsComponent::setupComboBox(juce::ComboBox& box, int defaultID) {
    addAndMakeVisible(box);
    box.addItem("Volume", 1);
    box.addItem("Pitch", 2);
    box.addItem("None", 3);
    box.setSelectedId(defaultID);
}

void SettingsComponent::paint(juce::Graphics& g) {
    g.fillAll(juce::Colours::black.withAlpha(0.95f)); 

    g.setColour(juce::Colours::white.withAlpha(0.2f));
    g.drawHorizontalLine(100, 20.0f, (float)getWidth() - 20.0f); 
}

void SettingsComponent::resized() {
    auto area = getLocalBounds().reduced(40); 

    // Title
    titleLabel.setBounds(area.removeFromTop(40));
    area.removeFromTop(20); 

    //Headers
    auto headerArea = area.removeFromTop(30);
    handAxisTitle.setBounds(headerArea.removeFromLeft(getWidth() / 2 - 40));
    mappedParamTitle.setBounds(headerArea);

    // Map the rows
    int rowH = 40;
    int labelW = 150;
    int boxW = 200;

    auto performLayout = [&](juce::Label& lbl, juce::ComboBox& box) {
        auto row = area.removeFromTop(rowH);
        lbl.setBounds(row.removeFromLeft(labelW));
        box.setBounds(row.removeFromLeft(boxW));
        area.removeFromTop(10); 
        };

    performLayout(leftXLabel, leftXBox);
    performLayout(leftYLabel, leftYBox);
    performLayout(rightXLabel, rightXBox);
    performLayout(rightYLabel, rightYBox);

    area.removeFromTop(30); 

    // Sensivity and Thresholds
    int sliderH = 25;

    sensitivityLabel.setBounds(area.removeFromTop(sliderH));
    sensitivitySlider.setBounds(area.removeFromTop(sliderH));
    area.removeFromTop(10);

    minHeightLabel.setBounds(area.removeFromTop(sliderH));
    minHeightSlider.setBounds(area.removeFromTop(sliderH));
    area.removeFromTop(10);

    maxHeightLabel.setBounds(area.removeFromTop(sliderH));
    maxHeightSlider.setBounds(area.removeFromTop(sliderH));

    // close button
    closeButton.setBounds(getLocalBounds().getCentreX() - 50, getHeight() - 50, 100, 30);
}