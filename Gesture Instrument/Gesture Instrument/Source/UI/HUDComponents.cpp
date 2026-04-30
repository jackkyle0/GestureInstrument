#include "HUDComponents.h"
#include "../Helpers/ScaleQuantiser.h" 

HUDComponents::HUDComponents(GestureInstrumentAudioProcessor& p)
    : audioProcessor(p) {
}

void HUDComponents::paint(juce::Graphics& g) {
    int hudThick = juce::jmax(45, getWidth() / 35);
    int barHeight = getHeight() * 0.65f;
    int topBarWidth = getWidth() * 0.25f;

    int margin = 10;
    int spacing = 15;
    int topY = 10;
    int bottomY = topY + hudThick + spacing;
    float sens = audioProcessor.sensitivityLevel;

    auto normalizeAxis = [](float rawValue, float minBound, float maxBound, float sensitivityMultiplier) {
        float mappedValue = juce::jmap(rawValue, minBound, maxBound, 0.0f, 1.0f);
        float centerOffset = 0.5f;
        mappedValue = centerOffset + ((mappedValue - centerOffset) * sensitivityMultiplier);
        return juce::jlimit(0.0f, 1.0f, mappedValue);
        };

    bool splitX = audioProcessor.enableSplitXAxis.load();
    float centerX = (audioProcessor.minWidthThreshold + audioProcessor.maxWidthThreshold) / 2.0f;
    float leftMaxX = splitX ? centerX : audioProcessor.maxWidthThreshold;
    float rightMinX = splitX ? centerX : audioProcessor.minWidthThreshold;

    float leftValX = -1.0f, leftValY = -1.0f, leftValZ = -1.0f, leftValRoll = -1.0f;
    if (audioProcessor.leftHand.isPresent) {
        leftValX = normalizeAxis(audioProcessor.leftHand.currentHandPositionX, audioProcessor.minWidthThreshold, leftMaxX, sens);
        leftValY = normalizeAxis(audioProcessor.leftHand.currentHandPositionY, audioProcessor.minHeightThreshold, audioProcessor.maxHeightThreshold, sens);
        leftValZ = 1.0f - normalizeAxis(audioProcessor.leftHand.currentHandPositionZ, audioProcessor.minDepthThreshold, audioProcessor.maxDepthThreshold, sens);
        float baseLeftRoll = juce::jmap(audioProcessor.leftHand.currentWristRotation, -0.6f, 0.6f, 0.0f, 1.0f);
        leftValRoll = 1.0f - juce::jlimit(0.0f, 1.0f, baseLeftRoll * audioProcessor.wristMultiplier.load());
    }

    float rightValX = -1.0f, rightValY = -1.0f, rightValZ = -1.0f, rightValRoll = -1.0f;
    if (audioProcessor.rightHand.isPresent) {
        rightValX = normalizeAxis(audioProcessor.rightHand.currentHandPositionX, rightMinX, audioProcessor.maxWidthThreshold, sens);
        rightValY = normalizeAxis(audioProcessor.rightHand.currentHandPositionY, audioProcessor.minHeightThreshold, audioProcessor.maxHeightThreshold, sens);
        rightValZ = 1.0f - normalizeAxis(audioProcessor.rightHand.currentHandPositionZ, audioProcessor.minDepthThreshold, audioProcessor.maxDepthThreshold, sens);
        float baseRightRoll = juce::jmap(audioProcessor.rightHand.currentWristRotation, -0.6f, 0.6f, 0.0f, 1.0f);
        rightValRoll = 1.0f - juce::jlimit(0.0f, 1.0f, baseRightRoll * audioProcessor.wristMultiplier.load());
    }

    // Left hand hud
    juce::Rectangle<int> leftXRect(margin, topY, topBarWidth, hudThick);
    drawParameterHUD(g, leftXRect, audioProcessor.leftXTarget, leftValX, false, "L-X", juce::Colours::cyan, true);

    juce::Rectangle<int> leftYRect(margin, bottomY, hudThick, barHeight);
    drawParameterHUD(g, leftYRect, audioProcessor.leftYTarget, leftValY, true, "L-Y", juce::Colours::cyan, true);

    juce::Rectangle<int> leftZRect(leftYRect.getRight() + spacing, bottomY, hudThick, barHeight);
    drawParameterHUD(g, leftZRect, audioProcessor.leftZTarget, leftValZ, true, "L-Z", juce::Colours::cyan.darker(0.2f), true);

    juce::Rectangle<int> leftRollRect(margin, leftYRect.getBottom() + spacing, (hudThick * 2) + spacing, 80);
    drawDial(g, leftRollRect, audioProcessor.leftRollTarget, leftValRoll, "L-ROLL", juce::Colours::cyan.brighter());

    // Right hand hud
    juce::Rectangle<int> rightXRect(getWidth() - topBarWidth - margin, topY, topBarWidth, hudThick);
    drawParameterHUD(g, rightXRect, audioProcessor.rightXTarget, rightValX, false, "R-X", juce::Colours::magenta, false);

    juce::Rectangle<int> rightYRect(getWidth() - margin - hudThick, bottomY, hudThick, barHeight);
    drawParameterHUD(g, rightYRect, audioProcessor.rightYTarget, rightValY, true, "R-Y", juce::Colours::magenta, false);

    juce::Rectangle<int> rightZRect(rightYRect.getX() - spacing - hudThick, bottomY, hudThick, barHeight);
    drawParameterHUD(g, rightZRect, audioProcessor.rightZTarget, rightValZ, true, "R-Z", juce::Colours::magenta.darker(0.2f), false);

    juce::Rectangle<int> rightRollRect(rightZRect.getX(), rightYRect.getBottom() + spacing, (hudThick * 2) + spacing, 80);
    drawDial(g, rightRollRect, audioProcessor.rightRollTarget, rightValRoll, "R-ROLL", juce::Colours::magenta.brighter());


    auto centerScreen = getLocalBounds().getCentre().toFloat();

    if (menuGestureTimer > 0.0f && !menuGestureFired && !isCalibrating) {
        float progress = menuGestureTimer / requiredHoldTime;

        g.setColour(juce::Colours::orange);
        g.setFont(18.0f);
        g.drawText(isEditMode ? "Closing Virtual Mouse..." : "Opening Virtual Mouse...",
            (int)centerScreen.x - 150, (int)centerScreen.y - 60, 300, 30, juce::Justification::centred);

        float radius = 40.0f;
        juce::Path p;
        p.addCentredArc(centerScreen.x, centerScreen.y, radius, radius, 0.0f, 0.0f, juce::MathConstants<float>::twoPi * progress, true);
        g.strokePath(p, juce::PathStrokeType(4.0f));
    }

    float sustainVal = -1.0f;
    float portamentoVal = -1.0f;

    if (audioProcessor.currentOutputMode == OutputMode::MIDI_Only) {
        sustainVal = audioProcessor.midiManager.liveSustain.load();
        portamentoVal = audioProcessor.midiManager.livePortamento.load();
    }
    else if (audioProcessor.currentOutputMode == OutputMode::OSC_Only) {
        sustainVal = audioProcessor.oscManager.liveSustain.load();
    }

    auto isLeftHand = [&](GestureTarget t) {
        return audioProcessor.leftXTarget == t || audioProcessor.leftYTarget == t || audioProcessor.leftZTarget == t ||
            audioProcessor.leftRollTarget == t || audioProcessor.leftGrabTarget == t || audioProcessor.leftPinchTarget == t ||
            audioProcessor.leftThumbTarget == t || audioProcessor.leftIndexTarget == t || audioProcessor.leftMiddleTarget == t ||
            audioProcessor.leftRingTarget == t || audioProcessor.leftPinkyTarget == t;
        };

    auto isRightHand = [&](GestureTarget t) {
        return audioProcessor.rightXTarget == t || audioProcessor.rightYTarget == t || audioProcessor.rightZTarget == t ||
            audioProcessor.rightRollTarget == t || audioProcessor.rightGrabTarget == t || audioProcessor.rightPinchTarget == t ||
            audioProcessor.rightThumbTarget == t || audioProcessor.rightIndexTarget == t || audioProcessor.rightMiddleTarget == t ||
            audioProcessor.rightRingTarget == t || audioProcessor.rightPinkyTarget == t;
        };

    auto drawStaticBadge = [&](juce::String name, float value, juce::Rectangle<int> drawArea, bool isMappedToThisHand, bool isHandPresent) {
        bool isOn = isMappedToThisHand && isHandPresent && (value > 0.5f);
        juce::Colour badgeCol = isOn ? juce::Colours::orange : juce::Colours::darkgrey;
        float fillAlpha = isMappedToThisHand ? 0.2f : 0.05f;
        float lineAlpha = isMappedToThisHand ? 1.0f : 0.3f;

        g.setColour(badgeCol.withAlpha(fillAlpha));
        g.fillRoundedRectangle(drawArea.toFloat(), 6.0f);
        g.setColour(badgeCol.withAlpha(lineAlpha));
        g.drawRoundedRectangle(drawArea.toFloat(), 2.0f, 6.0f);
        g.setColour(juce::Colours::white.withAlpha(lineAlpha));
        g.setFont(juce::Font(14.0f, juce::Font::bold));
        g.drawText(name + (isOn ? ": ON" : ": OFF"), drawArea, juce::Justification::centred);
        };

    int badgeW = 130;
    int badgeH = 35;
    int badgePadding = 10;

    int leftX = leftRollRect.getRight() + spacing;
    int rightX = rightRollRect.getX() - badgeW - spacing;

    int portY = leftRollRect.getY();
    int susY = portY + badgeH + badgePadding;

    if (audioProcessor.currentOutputMode == OutputMode::MIDI_Only) {
        bool susMappedLeft = isLeftHand(GestureTarget::Sustain);
        bool susMappedRight = isRightHand(GestureTarget::Sustain);

        drawStaticBadge("SUSTAIN", sustainVal, juce::Rectangle<int>(leftX, susY, badgeW, badgeH), susMappedLeft, audioProcessor.leftHand.isPresent);
        drawStaticBadge("SUSTAIN", sustainVal, juce::Rectangle<int>(rightX, susY, badgeW, badgeH), susMappedRight, audioProcessor.rightHand.isPresent);

        bool portMappedLeft = isLeftHand(GestureTarget::Portamento);
        bool portMappedRight = isRightHand(GestureTarget::Portamento);

        drawStaticBadge("PORTAMENTO", portamentoVal, juce::Rectangle<int>(leftX, portY, badgeW, badgeH), portMappedLeft, audioProcessor.leftHand.isPresent);
        drawStaticBadge("PORTAMENTO", portamentoVal, juce::Rectangle<int>(rightX, portY, badgeW, badgeH), portMappedRight, audioProcessor.rightHand.isPresent);
    }
}

void HUDComponents::drawParameterHUD(juce::Graphics& g, juce::Rectangle<int> bounds, GestureTarget target, float value, bool isVertical, juce::String label, juce::Colour color, bool isLeftHand) {
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
        if (audioProcessor.scaleType == 12) {
            drawUnquantisedPitch(g, bounds, value, isVertical, color);
        }
        else {
            drawScaleBlocks(g, bounds, value, isVertical, color, isLeftHand);
        }
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

void HUDComponents::drawScaleBlocks(juce::Graphics& g, juce::Rectangle<int> bounds, float value, bool isVertical, juce::Colour color, bool isLeftHand) {
    ScaleQuantiser quantiser;
    quantiser.customIntervals = audioProcessor.customScaleIntervals;
    std::vector<int> intervals = quantiser.getScaleIntervals(audioProcessor.scaleType);

    int minNote, maxNote;
    if (audioProcessor.currentRangeMode == MusicalRangeMode::OctaveRange) {
        minNote = audioProcessor.startNote + audioProcessor.rootNote;
        maxNote = minNote + (audioProcessor.octaveRange * 12);
    }
    else {
        minNote = audioProcessor.startNote;
        maxNote = audioProcessor.endNote;
        if (minNote > maxNote) std::swap(minNote, maxNote);
    }

    bool engineOn = audioProcessor.chordEngineEnabled.load() && (audioProcessor.currentOutputMode != OutputMode::OSC_Only);
    int physicalMinNote = minNote;
    int physicalMaxNote = maxNote;
    bool dropBassOn = false;
    std::array<bool, 7> activeRoots = { true, true, true, true, true, true, true };

    if (engineOn) {
        if (isLeftHand) {
            activeRoots = { audioProcessor.leftRootI.load(), audioProcessor.leftRootII.load(), audioProcessor.leftRootIII.load(), audioProcessor.leftRootIV.load(), audioProcessor.leftRootV.load(), audioProcessor.leftRootVI.load(), audioProcessor.leftRootVII.load() };
            dropBassOn = audioProcessor.leftDropBass.load();
        }
        else {
            activeRoots = { audioProcessor.rightRootI.load(), audioProcessor.rightRootII.load(), audioProcessor.rightRootIII.load(), audioProcessor.rightRootIV.load(), audioProcessor.rightRootV.load(), audioProcessor.rightRootVI.load(), audioProcessor.rightRootVII.load() };
            dropBassOn = audioProcessor.rightDropBass.load();
        }

        maxNote += 24;
        if (dropBassOn) {
            minNote -= 12;
        }
    }

    std::vector<int> displayNotes;
    for (int n = minNote; n <= maxNote; ++n) {
        int relativeNote = (n - audioProcessor.rootNote) % 12;
        if (relativeNote < 0) relativeNote += 12;

        bool inScale = false;
        for (int interval : intervals) {
            if (relativeNote == interval) {
                inScale = true;
                break;
            }
        }
        if (inScale) displayNotes.push_back(n);
    }

    if (displayNotes.empty()) displayNotes.push_back(minNote);

    int totalBlocks = (int)displayNotes.size();
    std::atomic<int>* activeNotes = isLeftHand ? audioProcessor.activeLeftNotes : audioProcessor.activeRightNotes;

    bool anyNotePlaying = false;
    for (int n = 0; n < 8; ++n) {
        if (activeNotes[n].load() != -1) {
            anyNotePlaying = true;
            break;
        }
    }

    float exactNote = juce::jmap(value, 0.0f, 1.0f, (float)physicalMinNote, (float)physicalMaxNote);
    int currentHoverNote = -1;
    if (value >= 0.0f) {
        currentHoverNote = quantiser.getQuantisedNote(exactNote / 127.0f, audioProcessor.rootNote, audioProcessor.scaleType, activeRoots);
    }

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
        bool isHighlighted = false;

        if (anyNotePlaying) {
            for (int n = 0; n < 8; ++n) {
                if (activeNotes[n].load() == noteNum) {
                    isHighlighted = true;
                    break;
                }
            }
        }
        else if (noteNum == currentHoverNote) {
            isHighlighted = true;
        }

        if (isHighlighted) {
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
            g.setColour(isHighlighted ? juce::Colours::black : juce::Colours::white.withAlpha(0.8f));

            float fontSize = isVertical ? juce::jmin(12.0f, blockSize * 0.8f) : juce::jmin(14.0f, blockSize * 0.5f);

            if (fontSize >= 6.0f) {
                g.setFont(fontSize);
                bool showOctave = (isVertical ? blockSize > 15.0f : blockSize > 30.0f);
                juce::String noteName = juce::MidiMessage::getMidiNoteName(noteNum, true, showOctave, 3);
                g.drawFittedText(noteName, blockRect.toNearestInt(), juce::Justification::centred, 1, 0.8f);
            }
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

void HUDComponents::drawUnquantisedPitch(juce::Graphics& g, juce::Rectangle<int> bounds, float value, bool isVertical, juce::Colour color) {
    g.setColour(juce::Colours::black.withAlpha(0.5f));
    g.fillRect(bounds);

    juce::ColourGradient grad(color.withAlpha(0.1f), bounds.getTopLeft().toFloat(), color.withAlpha(0.6f), bounds.getBottomRight().toFloat(), false);
    g.setGradientFill(grad);
    g.fillRect(bounds);

    g.setColour(juce::Colours::white);
    if (isVertical) {
        float yPos = bounds.getBottom() - (bounds.getHeight() * value);
        g.drawLine(bounds.getX(), yPos, bounds.getRight(), yPos, 3.0f);
        g.setFont(12.0f);
    }
    else {
        float xPos = bounds.getX() + (bounds.getWidth() * value);
        g.drawLine(xPos, bounds.getY(), xPos, bounds.getBottom(), 3.0f);
    }

    g.setColour(juce::Colours::white.withAlpha(0.2f));
    g.drawRect(bounds);
}