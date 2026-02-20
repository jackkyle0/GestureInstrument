#include "HUDComponents.h"

HUDComponents::HUDComponents(GestureInstrumentAudioProcessor& p)
    : audioProcessor(p)
{
}

void HUDComponents::paint(juce::Graphics& g) {
    int hudThick = 30;
    int margin = 10;
    int spacing = 15; // Increased spacing for better visibility
    int topY = 10;    // Small padding from the top of the HUD area
    int bottomY = 50; //

    float sens = audioProcessor.sensitivityLevel;

    // --- Calculate Hand Values (with Z inverted for Push = Max) ---
    float lValX = -1.0f, lValY = -1.0f, lValZ = -1.0f;
    if (audioProcessor.leftHand.isPresent) {
        lValX = juce::jlimit(0.0f, 1.0f, juce::jmap(audioProcessor.leftHand.currentHandPositionX, audioProcessor.minWidthThreshold, audioProcessor.maxWidthThreshold, 0.0f, 1.0f));
        lValY = juce::jlimit(0.0f, 1.0f, juce::jmap(audioProcessor.leftHand.currentHandPositionY, audioProcessor.minHeightThreshold, audioProcessor.maxHeightThreshold, 0.0f, 1.0f));
        float rawZ = juce::jmap(audioProcessor.leftHand.currentHandPositionZ, audioProcessor.minDepthThreshold, audioProcessor.maxDepthThreshold, 0.0f, 1.0f);
        lValZ = juce::jlimit(0.0f, 1.0f, 1.0f - rawZ);
    }

    float rValX = -1.0f, rValY = -1.0f, rValZ = -1.0f;
    if (audioProcessor.rightHand.isPresent) {
        rValX = juce::jlimit(0.0f, 1.0f, juce::jmap(audioProcessor.rightHand.currentHandPositionX, audioProcessor.minWidthThreshold, audioProcessor.maxWidthThreshold, 0.0f, 1.0f));
        rValY = juce::jlimit(0.0f, 1.0f, juce::jmap(audioProcessor.rightHand.currentHandPositionY, audioProcessor.minHeightThreshold, audioProcessor.maxHeightThreshold, 0.0f, 1.0f));
        float rawZ = juce::jmap(audioProcessor.rightHand.currentHandPositionZ, audioProcessor.minDepthThreshold, audioProcessor.maxDepthThreshold, 0.0f, 1.0f);
        rValZ = juce::jlimit(0.0f, 1.0f, 1.0f - rawZ);
    }

    juce::Rectangle<int> leftXRect(margin, topY, 300, hudThick);
    drawParameterHUD(g, leftXRect, audioProcessor.leftXTarget, lValX, false, "L-X", juce::Colours::cyan);

    juce::Rectangle<int> leftYRect(margin, bottomY, hudThick, 250); // Adjusted height
    drawParameterHUD(g, leftYRect, audioProcessor.leftYTarget, lValY, true, "L-Y", juce::Colours::cyan);

    juce::Rectangle<int> leftZRect(leftYRect.getRight() + spacing, bottomY, hudThick, 250);
    drawParameterHUD(g, leftZRect, audioProcessor.leftZTarget, lValZ, true, "L-Z", juce::Colours::cyan.darker(0.2f));

    // --- Draw Right Hand HUD ---
    juce::Rectangle<int> rightXRect(getWidth() - 300 - margin, topY, 300, hudThick);
    drawParameterHUD(g, rightXRect, audioProcessor.rightXTarget, rValX, false, "R-X", juce::Colours::magenta);

    juce::Rectangle<int> rightYRect(getWidth() - margin - hudThick, bottomY, hudThick, 250);
    drawParameterHUD(g, rightYRect, audioProcessor.rightYTarget, rValY, true, "R-Y", juce::Colours::magenta);

    juce::Rectangle<int> rightZRect(rightYRect.getX() - spacing - hudThick, bottomY, hudThick, 250);
    drawParameterHUD(g, rightZRect, audioProcessor.rightZTarget, rValZ, true, "R-Z", juce::Colours::magenta.darker(0.2f));
}

// =========================================================================================
// HELPER FUNCTIONS (Prefixed with HUDComponents::)

void HUDComponents::drawParameterHUD(juce::Graphics& g, juce::Rectangle<int> bounds, GestureTarget target, float value, bool isVertical, juce::String label, juce::Colour color) {
    g.setColour(juce::Colours::black.withAlpha(0.5f));
    g.fillRect(bounds);
    g.setColour(juce::Colours::white.withAlpha(0.1f));
    g.drawRect(bounds);

    g.setColour(juce::Colours::white.withAlpha(0.6f));
    g.setFont(10.0f);
    if (isVertical) {
        g.drawText(label, bounds.removeFromTop(15), juce::Justification::centred);
    }
    else {
        g.drawText(label, bounds.removeFromLeft(30), juce::Justification::centredLeft);
    }

    if (value < 0.0f) return;

    switch (target) {
    case GestureTarget::Pitch:
        drawScaleBlocks(g, bounds, value, isVertical, color);
        break;
    case GestureTarget::Sustain:
    case GestureTarget::Portamento:
    case GestureTarget::NoteTrigger:
        drawBooleanBox(g, bounds, value, color);
        break;
    case GestureTarget::None:
        break;
    default:
        drawFaderBar(g, bounds, value, isVertical, color);
        break;
    }
}

std::vector<int> HUDComponents::getScaleIntervals(int scaleType) {
    switch (scaleType) {
    case 1: return { 0, 2, 4, 5, 7, 9, 11 };       // Major (7 notes)
    case 2: return { 0, 2, 3, 5, 7, 8, 10 };       // Minor (7 notes)
    case 3: return { 0, 2, 4, 7, 9 };              // Pentatonic (5 notes)
    default: {                                     // Chromatic (12 notes)
        std::vector<int> chromatic;
        for (int i = 0; i < 12; ++i) chromatic.push_back(i);
        return chromatic;
    }
    }
}

void HUDComponents::drawScaleBlocks(juce::Graphics& g, juce::Rectangle<int> bounds, float value, bool isVertical, juce::Colour color) {
    std::vector<int> intervals = getScaleIntervals(audioProcessor.scaleType);
    int notesPerOctave = (int)intervals.size();
    int totalBlocks = audioProcessor.octaveRange * notesPerOctave;

    float blockSize = isVertical ?
        (float)bounds.getHeight() / totalBlocks :
        (float)bounds.getWidth() / totalBlocks;

    int activeIndex = juce::jlimit(0, totalBlocks - 1, (int)(value * totalBlocks));

    for (int i = 0; i < totalBlocks; ++i) {
        int octave = i / notesPerOctave;
        int scaleIndex = i % notesPerOctave;
        int chromaticInterval = intervals[scaleIndex];
        int root = audioProcessor.rootNote;

        int noteNum = 48 + (octave * 12) + root + chromaticInterval;

        juce::Rectangle<float> blockRect;

        if (isVertical) {
            float yPos = bounds.getBottom() - ((i + 1) * blockSize);
            blockRect = juce::Rectangle<float>((float)bounds.getX(), yPos, (float)bounds.getWidth(), blockSize);
        }
        else {
            float xPos = bounds.getX() + (i * blockSize);
            blockRect = juce::Rectangle<float>(xPos, (float)bounds.getY(), blockSize, (float)bounds.getHeight());
        }

        blockRect.reduce(1.0f, 1.0f);

        if (i == activeIndex) {
            g.setColour(color);
            g.fillRect(blockRect);
            g.setColour(juce::Colours::white.withAlpha(0.5f));
            g.drawRect(blockRect, 2.0f);

            // Note names (assuming you want them always on for now, or you can pass a boolean down from the editor)
            g.setColour(juce::Colours::black);
            g.setFont(isVertical ? 10.0f : 14.0f);
            juce::String noteName = juce::MidiMessage::getMidiNoteName(noteNum, true, true, 3);
            g.drawText(noteName, blockRect, juce::Justification::centred);
        }
        else {
            g.setColour(juce::Colours::white.withAlpha(0.1f));
            g.fillRect(blockRect);
        }
    }
}

void HUDComponents::drawFaderBar(juce::Graphics& g, juce::Rectangle<int> bounds, float value, bool isVertical, juce::Colour color) {
    g.setColour(juce::Colours::white.withAlpha(0.05f));
    g.fillRect(bounds);

    if (isVertical) {
        int fillH = (int)(bounds.getHeight() * value);
        int y = bounds.getBottom() - fillH;
        g.setColour(color);
        g.fillRect(bounds.getX(), y, bounds.getWidth(), fillH);
    }
    else {
        int fillW = (int)(bounds.getWidth() * value);
        g.setColour(color);
        g.fillRect(bounds.getX(), bounds.getY(), fillW, bounds.getHeight());
    }
}

void HUDComponents::drawBooleanBox(juce::Graphics& g, juce::Rectangle<int> bounds, float value, juce::Colour color) {
    bool isOn = value > 0.5f;
    auto box = bounds.withSizeKeepingCentre(20, 20);

    if (isOn) {
        g.setColour(color);
        g.fillRect(box);
        g.setColour(juce::Colours::white);
        g.drawText("ON", box, juce::Justification::centred);
    }
    else {
        g.setColour(juce::Colours::white.withAlpha(0.2f));
        g.drawRect(box, 2.0f);
        g.setFont(10.0f);
        g.drawText("OFF", box, juce::Justification::centred);
    }
}

void HUDComponents::mouseDown(const juce::MouseEvent& e) {
    myDragger.startDraggingComponent(this, e);
}

void HUDComponents::mouseDrag(const juce::MouseEvent& e) {
    myDragger.dragComponent(this, e, nullptr);
}