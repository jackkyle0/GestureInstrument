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

    auto normaise = [](float rawValue, float minBound, float maxBound, float sensitivityMultiplier) {
        float mappedValue = juce::jmap(rawValue, minBound, maxBound, 0.0f, 1.0f);
        float centerOffset = 0.5f;
        mappedValue = centerOffset + ((mappedValue - centerOffset) * sensitivityMultiplier);
        return juce::jlimit(0.0f, 1.0f, mappedValue);
        };

    bool splitX = audioProcessor.enableSplitXAxis.load();
    float centerX = (audioProcessor.minWidthThreshold + audioProcessor.maxWidthThreshold) / 2.0f;
    float leftMaxX = splitX ? centerX : audioProcessor.maxWidthThreshold;
    float rightMinX = splitX ? centerX : audioProcessor.minWidthThreshold;

    float lValX = -1.0f, lValY = -1.0f, lValZ = -1.0f, lValRoll = -1.0f;
    if (audioProcessor.leftHand.isPresent) {
        lValX = normaise(audioProcessor.leftHand.currentHandPositionX, audioProcessor.minWidthThreshold, leftMaxX, sens);
        lValY = normaise(audioProcessor.leftHand.currentHandPositionY, audioProcessor.minHeightThreshold, audioProcessor.maxHeightThreshold, sens);
        lValZ = 1.0f - normaise(audioProcessor.leftHand.currentHandPositionZ, audioProcessor.minDepthThreshold, audioProcessor.maxDepthThreshold, sens);
        lValRoll = 1.0f - juce::jlimit(0.0f, 1.0f, juce::jmap(audioProcessor.leftHand.currentWristRotation, -0.6f, 0.6f, 0.0f, 1.0f));
    }

    float rValX = -1.0f, rValY = -1.0f, rValZ = -1.0f, rValRoll = -1.0f;
    if (audioProcessor.rightHand.isPresent) {
        rValX = normaise(audioProcessor.rightHand.currentHandPositionX, rightMinX, audioProcessor.maxWidthThreshold, sens);
        rValY = normaise(audioProcessor.rightHand.currentHandPositionY, audioProcessor.minHeightThreshold, audioProcessor.maxHeightThreshold, sens);
        rValZ = 1.0f - normaise(audioProcessor.rightHand.currentHandPositionZ, audioProcessor.minDepthThreshold, audioProcessor.maxDepthThreshold, sens);
        rValRoll = 1.0f - juce::jlimit(0.0f, 1.0f, juce::jmap(audioProcessor.rightHand.currentWristRotation, -0.6f, 0.6f, 0.0f, 1.0f));
    }

    // --- 2. UPDATED LEFT HAND DRAWING (Notice the 'true' passed at the end for Left Hand) ---
    juce::Rectangle<int> leftXRect(margin, topY, topBarWidth, hudThick);
    drawParameterHUD(g, leftXRect, audioProcessor.leftXTarget, lValX, false, "L-X", juce::Colours::cyan, true);

    juce::Rectangle<int> leftYRect(margin, bottomY, hudThick, barHeight);
    drawParameterHUD(g, leftYRect, audioProcessor.leftYTarget, lValY, true, "L-Y", juce::Colours::cyan, true);

    juce::Rectangle<int> leftZRect(leftYRect.getRight() + spacing, bottomY, hudThick, barHeight);
    drawParameterHUD(g, leftZRect, audioProcessor.leftZTarget, lValZ, true, "L-Z", juce::Colours::cyan.darker(0.2f), true);

    juce::Rectangle<int> leftRollRect(margin, leftYRect.getBottom() + spacing, (hudThick * 2) + spacing, 80);
    drawDial(g, leftRollRect, audioProcessor.leftRollTarget, lValRoll, "L-ROLL", juce::Colours::cyan.brighter());

    // --- 3. UPDATED RIGHT HAND DRAWING (Notice the 'false' passed at the end for Right Hand) ---
    juce::Rectangle<int> rightXRect(getWidth() - topBarWidth - margin, topY, topBarWidth, hudThick);
    drawParameterHUD(g, rightXRect, audioProcessor.rightXTarget, rValX, false, "R-X", juce::Colours::magenta, false);

    juce::Rectangle<int> rightYRect(getWidth() - margin - hudThick, bottomY, hudThick, barHeight);
    drawParameterHUD(g, rightYRect, audioProcessor.rightYTarget, rValY, true, "R-Y", juce::Colours::magenta, false);

    juce::Rectangle<int> rightZRect(rightYRect.getX() - spacing - hudThick, bottomY, hudThick, barHeight);
    drawParameterHUD(g, rightZRect, audioProcessor.rightZTarget, rValZ, true, "R-Z", juce::Colours::magenta.darker(0.2f), false);

    juce::Rectangle<int> rightRollRect(rightZRect.getX(), rightYRect.getBottom() + spacing, (hudThick * 2) + spacing, 80);
    drawDial(g, rightRollRect, audioProcessor.rightRollTarget, rValRoll, "R-ROLL", juce::Colours::magenta.brighter());


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
            drawScaleBlocks(g, bounds, value, isVertical, color, isLeftHand); // <--- PASSING THE HAND DATA DOWN!
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

std::vector<int> HUDComponents::getScaleIntervals(int scaleType) {
    switch (scaleType) {
    case 1: return { 0, 2, 4, 5, 7, 9, 11 };       // Major
    case 2: return { 0, 2, 3, 5, 7, 8, 10 };       // Minor
    case 3: return { 0, 2, 4, 7, 9 };              // Major Pentatonic 
    case 4: return { 0, 3, 5, 7, 10 };             // Minor Pentatonic
    case 5: return { 0, 3, 5, 6, 7, 10 };          // Blues
    case 6: return { 0, 2, 3, 5, 7, 9, 10 };       // Dorian
    case 7: return { 0, 2, 4, 5, 7, 9, 10 };       // Mixolydian
    case 8: return { 0, 2, 4, 6, 7, 9, 11 };       // Lydian
    case 9: return { 0, 1, 3, 5, 7, 8, 10 };       // Phrygian
    case 10: return { 0, 2, 3, 5, 7, 8, 11 };      // Harmonic Minor
    case 11: return { 0, 1, 3, 5, 6, 8, 10 };      // Locrian
    case 13: return audioProcessor.customScaleIntervals;
    case 12:

    default: {                                     // Chromatic (0)
        std::vector<int> chromatic;
        for (int i = 0; i < 12; ++i) chromatic.push_back(i);
        return chromatic;
    }
    }
}


void HUDComponents::drawScaleBlocks(juce::Graphics& g, juce::Rectangle<int> bounds, float value, bool isVertical, juce::Colour color, bool isLeftHand) {
    std::vector<int> intervals = getScaleIntervals(audioProcessor.scaleType);

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

    // --- THE HEADROOM & FLOOR-ROOM FIX ---
    // Store the actual physical boundaries so your hand math stays perfect
    int physicalMinNote = minNote;
    int physicalMaxNote = maxNote;

    bool dropBassOn = false;

    std::array<bool, 7> activeRoots = { true, true, true, true, true, true, true };

    if (engineOn) {
        // Fetch the specific hand's data
        if (isLeftHand) {
            activeRoots = { audioProcessor.leftRootI.load(), audioProcessor.leftRootII.load(), audioProcessor.leftRootIII.load(), audioProcessor.leftRootIV.load(), audioProcessor.leftRootV.load(), audioProcessor.leftRootVI.load(), audioProcessor.leftRootVII.load() };
            dropBassOn = audioProcessor.leftDropBass.load();
        }
        else {
            activeRoots = { audioProcessor.rightRootI.load(), audioProcessor.rightRootII.load(), audioProcessor.rightRootIII.load(), audioProcessor.rightRootIV.load(), audioProcessor.rightRootV.load(), audioProcessor.rightRootVI.load(), audioProcessor.rightRootVII.load() };
            dropBassOn = audioProcessor.rightDropBass.load();
        }

        // Expand the visual grid up by 2 octaves for chord extensions
        maxNote += 24;

        // Expand the visual grid down by 1 octave if Drop Bass is active!
        if (dropBassOn) {
            minNote -= 12;
        }
    }

    // 1. DRAW THE FULL EXTENDED SCALE GRID
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

    // 2. Fetch active playing notes
    std::atomic<int>* activeNotes = isLeftHand ? audioProcessor.activeLeftNotes : audioProcessor.activeRightNotes;

    bool anyNotePlaying = false;
    for (int n = 0; n < 8; ++n) {
        if (activeNotes[n].load() != -1) {
            anyNotePlaying = true;
            break;
        }
    }

    // 3. THE MAGIC CURSOR: Maps to 'physicalMinNote' and 'physicalMaxNote'
    // This perfectly isolates your hand mapping to the real physical space,
    // while the UI draws the lush extra octaves around it!
    float exactNote = juce::jmap(value, 0.0f, 1.0f, (float)physicalMinNote, (float)physicalMaxNote);
    ScaleQuantiser quantiser;
    quantiser.customIntervals = audioProcessor.customScaleIntervals;

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

        // Highlight Logic
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