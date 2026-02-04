#pragma once
#include <JuceHeader.h>
#include "GestureTarget.h"
#include "../Helpers/HandData.h"
#include "../Helpers/ScaleQuantiser.h" 

class MidiManager {
public:

    void sendProgramChange(juce::MidiBuffer& midi, int programNumber) {
        midi.addEvent(juce::MidiMessage::programChange(1, programNumber), 0);
    }

    void processHandData(juce::MidiBuffer& midiMessages,
        const HandData& left, const HandData& right,
        float sensitivity, float minH, float maxH,
        // Left Targets
        GestureTarget leftXTarget, GestureTarget leftYTarget,
        GestureTarget leftZTarget, GestureTarget leftRollTarget,
        GestureTarget leftGrabTarget, GestureTarget leftPinchTarget,
        GestureTarget lThumb, GestureTarget lIndex, GestureTarget lMiddle, GestureTarget lRing, GestureTarget lPinky,
        // Right Targets
        GestureTarget rightXTarget, GestureTarget rightYTarget,
        GestureTarget rightZTarget, GestureTarget rightRollTarget,
        GestureTarget rightGrabTarget, GestureTarget rightPinchTarget,
        GestureTarget rThumb, GestureTarget rIndex, GestureTarget rMiddle, GestureTarget rRing, GestureTarget rPinky,
        // Scale and octave range
        int rootNote, int scaleType, int octaveRange) 
    {
        // Calculate values (0.0 to 1.0)
        float leftX = calculateX(left, sensitivity);
        float leftY = calculateY(left, minH, maxH);
        float rightX = calculateX(right, sensitivity);
        float rightY = calculateY(right, minH, maxH);

        // Calculate Grab/Pinch
        float leftGrab = left.isPresent ? left.grabStrength : -1.0f;
        float leftPinch = left.isPresent ? left.pinchStrength : -1.0f;
        float rightGrab = right.isPresent ? right.grabStrength : -1.0f;
        float rightPinch = right.isPresent ? right.pinchStrength : -1.0f;

        float pitchValue = -1.0f;

        if (leftXTarget == GestureTarget::Pitch)  pitchValue = leftX;
        if (leftYTarget == GestureTarget::Pitch)  pitchValue = leftY;
        if (rightXTarget == GestureTarget::Pitch) pitchValue = rightX;
        if (rightYTarget == GestureTarget::Pitch) pitchValue = rightY;

        // Pass octaveRange to the note handler
        handleNoteLogic(midiMessages, pitchValue, rootNote, scaleType, octaveRange);

        auto processHandCCs = [&](const HandData& h, float x, float y, float grab, float pinch,
            GestureTarget tX, GestureTarget tY, GestureTarget tZ, GestureTarget tRoll, GestureTarget tGrab, GestureTarget tPinch,
            GestureTarget tThumb, GestureTarget tIndex, GestureTarget tMiddle, GestureTarget tRing, GestureTarget tPinky)
            {
                if (!h.isPresent) return;

                sendCC(midiMessages, tX, x);
                sendCC(midiMessages, tY, y);
                sendCC(midiMessages, tGrab, grab);
                sendCC(midiMessages, tPinch, pinch);

                // Fingers
                auto getFingerVal = [&](int idx) {
                    if (idx < 0 || idx >= 5) return 0.0f;
                    return juce::jlimit(0.0f, 1.0f, juce::jmap(h.fingers[idx].tipY, minH, maxH, 0.0f, 1.0f));
                    };

                sendCC(midiMessages, tThumb, getFingerVal(0));
                sendCC(midiMessages, tIndex, getFingerVal(1));
                sendCC(midiMessages, tMiddle, getFingerVal(2));
                sendCC(midiMessages, tRing, getFingerVal(3));
                sendCC(midiMessages, tPinky, getFingerVal(4));
            };


        // Process Left
        processHandCCs(left, leftX, leftY, leftGrab, leftPinch,
            leftXTarget, leftYTarget, leftZTarget, leftRollTarget, leftGrabTarget, leftPinchTarget,
            lThumb, lIndex, lMiddle, lRing, lPinky);

        // Process Right
        processHandCCs(right, rightX, rightY, rightGrab, rightPinch,
            rightXTarget, rightYTarget, rightZTarget, rightRollTarget, rightGrabTarget, rightPinchTarget,
            rThumb, rIndex, rMiddle, rRing, rPinky);
    }

private:
    int lastMidiNote = -1;
    bool isNoteOn = false;
    ScaleQuantiser quantiser; 

    // X axis
    float calculateX(const HandData& h, float sens) {
        if (!h.isPresent) return -1.0f;
        float range = 200.0f / sens;
        return juce::jlimit(0.0f, 1.0f, juce::jmap(h.currentHandPositionX, -range, range, 0.0f, 1.0f));
    }

    // Y axis
    float calculateY(const HandData& h, float min, float max) {
        if (!h.isPresent) return -1.0f;
        return juce::jlimit(0.0f, 1.0f, juce::jmap(h.currentHandPositionY, min, max, 0.0f, 1.0f));
    }

    void handleNoteLogic(juce::MidiBuffer& midi, float val, int root, int scale, int octaveRange) {
        if (val >= 0.0f) {


            float minNote = 48.0f; // C3
            float maxNote = 48.0f + (octaveRange * 12.0f);

            float noteInRange = juce::jmap(val, 0.0f, 1.0f, minNote, maxNote);
            float quantiserInput = noteInRange / 127.0f;

            int note = quantiser.getQuantisedNote(quantiserInput, root, scale);

            if (note != lastMidiNote || !isNoteOn) {
                if (isNoteOn) midi.addEvent(juce::MidiMessage::noteOff(1, lastMidiNote), 0);
                midi.addEvent(juce::MidiMessage::noteOn(1, note, (juce::uint8)100), 0);
                lastMidiNote = note;
                isNoteOn = true;
            }
        }
        else if (isNoteOn) {
            midi.addEvent(juce::MidiMessage::noteOff(1, lastMidiNote), 0);
            isNoteOn = false;
        }
    }

    void sendCC(juce::MidiBuffer& midi, GestureTarget target, float val) {
        if (val < 0.0f) return;

        int ccNumber = -1;
        switch (target) {
        case GestureTarget::Volume:     ccNumber = 7; break;
        case GestureTarget::Modulation: ccNumber = 1; break;
        case GestureTarget::Expression: ccNumber = 11; break;
        case GestureTarget::Cutoff:     ccNumber = 74; break;
        case GestureTarget::Resonance:  ccNumber = 71; break;
        case GestureTarget::Vibrato:    ccNumber = 76; break;
        default: return;
        }

        midi.addEvent(juce::MidiMessage::controllerEvent(1, ccNumber, (int)(val * 127.0f)), 0);
    }
};