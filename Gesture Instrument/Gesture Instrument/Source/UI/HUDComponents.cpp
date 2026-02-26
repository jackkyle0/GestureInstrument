#include "HUDComponents.h"
#include "../Helpers/ScaleQuantiser.h" 


HUDComponents::HUDComponents(GestureInstrumentAudioProcessor& p)
    : audioProcessor(p) {
}

void HUDComponents::paint(juce::Graphics& g) {
    int hudThick = 45;
    int barHeight = 300;
    int margin = 10;
    int spacing = 15;
    int topY = 10;
    int bottomY = 60;

    float sens = audioProcessor.sensitivityLevel;

    auto normaise = [](float rawValue, float minBound, float maxBound, float sensitivityMultiplier) {
        float mappedValue = juce::jmap(rawValue, minBound, maxBound, 0.0f, 1.0f);
        float centerOffset = 0.5f;
        mappedValue = centerOffset + ((mappedValue - centerOffset) * sensitivityMultiplier);
        return juce::jlimit(0.0f, 1.0f, mappedValue);
        };

    float lValX = -1.0f, lValY = -1.0f, lValZ = -1.0f, lValRoll = -1.0f;
    if (audioProcessor.leftHand.isPresent) {
        lValX = normaise(audioProcessor.leftHand.currentHandPositionX, audioProcessor.minWidthThreshold, audioProcessor.maxWidthThreshold, sens);
        lValY = normaise(audioProcessor.leftHand.currentHandPositionY, audioProcessor.minHeightThreshold, audioProcessor.maxHeightThreshold, sens);
        lValZ = 1.0f - normaise(audioProcessor.leftHand.currentHandPositionZ, audioProcessor.minDepthThreshold, audioProcessor.maxDepthThreshold, sens);
        lValRoll = 1.0f - juce::jlimit(0.0f, 1.0f, juce::jmap(audioProcessor.leftHand.currentWristRotation, -0.6f, 0.6f, 0.0f, 1.0f));
    }

    float rValX = -1.0f, rValY = -1.0f, rValZ = -1.0f, rValRoll = -1.0f;
    if (audioProcessor.rightHand.isPresent) {
        rValX = normaise(audioProcessor.rightHand.currentHandPositionX, audioProcessor.minWidthThreshold, audioProcessor.maxWidthThreshold, sens);
        rValY = normaise(audioProcessor.rightHand.currentHandPositionY, audioProcessor.minHeightThreshold, audioProcessor.maxHeightThreshold, sens);
        rValZ = 1.0f - normaise(audioProcessor.rightHand.currentHandPositionZ, audioProcessor.minDepthThreshold, audioProcessor.maxDepthThreshold, sens);
        rValRoll = 1.0f - juce::jlimit(0.0f, 1.0f, juce::jmap(audioProcessor.rightHand.currentWristRotation, -0.6f, 0.6f, 0.0f, 1.0f));
    }

    // Draw left hand
    juce::Rectangle<int> leftXRect(margin, topY, 300, hudThick);
    drawParameterHUD(g, leftXRect, audioProcessor.leftXTarget, lValX, false, "L-X", juce::Colours::cyan);

    juce::Rectangle<int> leftYRect(margin, bottomY, hudThick, barHeight);
    drawParameterHUD(g, leftYRect, audioProcessor.leftYTarget, lValY, true, "L-Y", juce::Colours::cyan);

    juce::Rectangle<int> leftZRect(leftYRect.getRight() + spacing, bottomY, hudThick, barHeight);
    drawParameterHUD(g, leftZRect, audioProcessor.leftZTarget, lValZ, true, "L-Z", juce::Colours::cyan.darker(0.2f));

    juce::Rectangle<int> leftRollRect(margin, leftYRect.getBottom() + spacing, (hudThick * 2) + spacing, 80);
    drawDial(g, leftRollRect, audioProcessor.leftRollTarget, lValRoll, "L-ROLL", juce::Colours::cyan.brighter());

    // Draw right hand
    juce::Rectangle<int> rightXRect(getWidth() - 300 - margin, topY, 400, hudThick);
    drawParameterHUD(g, rightXRect, audioProcessor.rightXTarget, rValX, false, "R-X", juce::Colours::magenta);

    juce::Rectangle<int> rightYRect(getWidth() - margin - hudThick, bottomY, hudThick, barHeight);
    drawParameterHUD(g, rightYRect, audioProcessor.rightYTarget, rValY, true, "R-Y", juce::Colours::magenta);

    juce::Rectangle<int> rightZRect(rightYRect.getX() - spacing - hudThick, bottomY, hudThick, barHeight);
    drawParameterHUD(g, rightZRect, audioProcessor.rightZTarget, rValZ, true, "R-Z", juce::Colours::magenta.darker(0.2f));

    juce::Rectangle<int> rightRollRect(rightZRect.getX(), rightYRect.getBottom() + spacing, (hudThick * 2) + spacing, 80);
    drawDial(g, rightRollRect, audioProcessor.rightRollTarget, rValRoll, "R-ROLL", juce::Colours::magenta.brighter());
}



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
    case 1: return { 0, 2, 4, 5, 7, 9, 11 };       // Major
    case 2: return { 0, 2, 3, 5, 7, 8, 10 };       // Minor
    case 3: return { 0, 2, 4, 7, 9 };              // Pentatonic 
    default: {                                     // Chromatic
        std::vector<int> chromatic;
        for (int i = 0; i < 12; ++i) chromatic.push_back(i);
        return chromatic;
    }
    }
}


void HUDComponents::drawScaleBlocks(juce::Graphics& g, juce::Rectangle<int> bounds, float value, bool isVertical, juce::Colour color) {
    std::vector<int> intervals = getScaleIntervals(audioProcessor.scaleType);

    int minNote, maxNote;
    if (audioProcessor.currentRangeMode == MusicalRangeMode::OctaveRange) {
        minNote = 48 + audioProcessor.rootNote;
        maxNote = minNote + (audioProcessor.octaveRange * 12);
    }
    else {
        minNote = audioProcessor.startNote;
        maxNote = audioProcessor.endNote;
        if (minNote > maxNote) std::swap(minNote, maxNote);
    }

    std::vector<int> displayNotes;
    for (int n = minNote; n <= maxNote; ++n) {
        int relativeNote = (n - audioProcessor.rootNote) % 12;
        if (relativeNote < 0) relativeNote += 12;

        bool inScale = false;
        for (int interval : intervals) {
            if (relativeNote == interval) { inScale = true; break; }
        }

        if (inScale || n == maxNote) {
            if (displayNotes.empty() || displayNotes.back() != n) {
                displayNotes.push_back(n);
            }
        }
    }
    if (displayNotes.empty()) displayNotes.push_back(minNote);

    int totalBlocks = (int)displayNotes.size();
    float exactNote = juce::jmap(value, 0.0f, 1.0f, (float)minNote, (float)maxNote);
    ScaleQuantiser quantiser;
    int currentPlayingNote = quantiser.getQuantisedNote(exactNote / 127.0f, audioProcessor.rootNote, audioProcessor.scaleType);

    float blockSize = isVertical ?
        (float)bounds.getHeight() / totalBlocks :
        (float)bounds.getWidth() / totalBlocks;

    for (int i = 0; i < totalBlocks; ++i) {
        int noteNum = displayNotes[i];
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

        if (noteNum == currentPlayingNote && value >= 0.0f) {
            g.setColour(color);
            g.fillRect(blockRect);
            g.setColour(juce::Colours::white.withAlpha(0.5f));
            g.drawRect(blockRect, 2.0f);
        }
        else {
            g.setColour(juce::Colours::white.withAlpha(0.1f));
            g.fillRect(blockRect);
        }

        if (audioProcessor.showNoteNames) {
            g.setColour(noteNum == currentPlayingNote && value >= 0.0f ? juce::Colours::black : juce::Colours::white.withAlpha(0.6f));
            g.setFont(isVertical ? 10.0f : 14.0f);
            juce::String noteName = juce::MidiMessage::getMidiNoteName(noteNum, true, true, 3);
            g.drawText(noteName, blockRect, juce::Justification::centred);
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

void HUDComponents::drawDial(juce::Graphics& g, juce::Rectangle<int> bounds, GestureTarget target, float value, juce::String label, juce::Colour color) {
    g.setColour(juce::Colours::black.withAlpha(0.5f));
    g.fillRect(bounds);
    g.setColour(juce::Colours::white.withAlpha(0.1f));
    g.drawRect(bounds);

    g.setColour(juce::Colours::white.withAlpha(0.6f));
    g.setFont(10.0f);
    g.drawText(label, bounds.removeFromTop(15), juce::Justification::centred);

    if (value < 0.0f) return;

    auto dialArea = bounds.reduced(5).toFloat();
    float radius = juce::jmin(dialArea.getWidth(), dialArea.getHeight()) / 2.0f;
    juce::Point<float> center = dialArea.getCentre();

    float startAngle = -juce::MathConstants<float>::pi * 0.75f;
    float endAngle = juce::MathConstants<float>::pi * 0.75f;

    juce::Path bgArc;
    bgArc.addCentredArc(center.x, center.y, radius, radius, 0.0f, startAngle, endAngle, true);
    g.setColour(juce::Colours::white.withAlpha(0.1f));
    g.strokePath(bgArc, juce::PathStrokeType(3.0f));

    float currentAngle = juce::jmap(value, 0.0f, 1.0f, startAngle, endAngle);
    juce::Path valArc;
    valArc.addCentredArc(center.x, center.y, radius, radius, 0.0f, startAngle, currentAngle, true);
    g.setColour(color);
    g.strokePath(valArc, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    juce::Path pointer;
    pointer.startNewSubPath(center);
    pointer.lineTo(center.x + radius * std::sin(currentAngle), center.y - radius * std::cos(currentAngle));
    g.strokePath(pointer, juce::PathStrokeType(2.0f));
}

void HUDComponents::mouseDown(const juce::MouseEvent& e) {
    myDragger.startDraggingComponent(this, e);
}

void HUDComponents::mouseDrag(const juce::MouseEvent& e) {
    myDragger.dragComponent(this, e, nullptr);
}