#pragma once
#include <JuceHeader.h>
#include "GestureTarget.h"
#include "../Helpers/HandData.h"
#include "../Helpers/ScaleQuantiser.h"

class MidiManager {
public:

    void sendProgramChange(juce::MidiBuffer& midi, int programNumber) {
        // Send on Ch 1, 2, and 3 
        midi.addEvent(juce::MidiMessage::programChange(1, programNumber), 0);
        midi.addEvent(juce::MidiMessage::programChange(2, programNumber), 0);
        midi.addEvent(juce::MidiMessage::programChange(3, programNumber), 0);
    }

    void processHandData(juce::MidiBuffer& midiMessages,
        const HandData& left, const HandData& right,
        float sensitivity,
        float minX, float maxX,
        float minY, float maxY,
        float minZ, float maxZ,
        // Left Targets
        GestureTarget leftXTarget, GestureTarget leftYTarget, GestureTarget leftZTarget, GestureTarget leftRollTarget,
        GestureTarget leftGrabTarget, GestureTarget leftPinchTarget,
        GestureTarget lThumb, GestureTarget lIndex, GestureTarget lMiddle, GestureTarget lRing, GestureTarget lPinky,
        // Right Targets
        GestureTarget rightXTarget, GestureTarget rightYTarget, GestureTarget rightZTarget, GestureTarget rightRollTarget,
        GestureTarget rightGrabTarget, GestureTarget rightPinchTarget,
        GestureTarget rThumb, GestureTarget rIndex, GestureTarget rMiddle, GestureTarget rRing, GestureTarget rPinky,
        // Scale and octave range
        int rootNote, int scaleType, int octaveRange)
    {
        // 1. The Normalization Engine
        // This converts any raw mm coordinate into a 0.0 to 1.0 float, applying sensitivity
        auto normalize = [](float rawValue, float minBound, float maxBound, float sens) {
            float mapped = juce::jmap(rawValue, minBound, maxBound, 0.0f, 1.0f);
            float center = 0.5f;
            mapped = center + ((mapped - center) * sens);
            return juce::jlimit(0.0f, 1.0f, mapped);
            };

        // 2. Extract Left Hand Values
        float leftX = -1.0f, leftY = -1.0f, leftZ = -1.0f;
        if (left.isPresent) {
            leftX = normalize(left.currentHandPositionX, minX, maxX, sensitivity);
            leftY = normalize(left.currentHandPositionY, minY, maxY, sensitivity);

            // INVERTED Z-AXIS: 1.0f - normalized value means PUSH = High, PULL = Low
            leftZ = 1.0f - normalize(left.currentHandPositionZ, minZ, maxZ, sensitivity);
        }

        // 3. Extract Right Hand Values
        float rightX = -1.0f, rightY = -1.0f, rightZ = -1.0f;
        if (right.isPresent) {
            rightX = normalize(right.currentHandPositionX, minX, maxX, sensitivity);
            rightY = normalize(right.currentHandPositionY, minY, maxY, sensitivity);

            // INVERTED Z-AXIS
            rightZ = 1.0f - normalize(right.currentHandPositionZ, minZ, maxZ, sensitivity);
        }

        // Calculate Grab/Pinch (0.0 to 1.0 directly from Leap)
        float leftGrab = left.isPresent ? left.grabStrength : -1.0f;
        float leftPinch = left.isPresent ? left.pinchStrength : -1.0f;
        float rightGrab = right.isPresent ? right.grabStrength : -1.0f;
        float rightPinch = right.isPresent ? right.pinchStrength : -1.0f;

        // MPE Setup
        int chLeft = 2;
        int chRight = 3;

        // --- PITCH LOGIC ---
        // Left Hand Pitch Mapping
        float lPitchVal = -1.0f;
        if (leftXTarget == GestureTarget::Pitch) lPitchVal = leftX;
        if (leftYTarget == GestureTarget::Pitch) lPitchVal = leftY;
        if (leftZTarget == GestureTarget::Pitch) lPitchVal = leftZ;
        handleNoteLogic(midiMessages, lPitchVal, rootNote, scaleType, octaveRange, chLeft, leftNoteState);

        // Right Hand Pitch Mapping
        float rPitchVal = -1.0f;
        if (rightXTarget == GestureTarget::Pitch) rPitchVal = rightX;
        if (rightYTarget == GestureTarget::Pitch) rPitchVal = rightY;
        if (rightZTarget == GestureTarget::Pitch) rPitchVal = rightZ;
        handleNoteLogic(midiMessages, rPitchVal, rootNote, scaleType, octaveRange, chRight, rightNoteState);

        // --- CC LOGIC ---
        auto processHandCCs = [&](const HandData& h, int channel, float x, float y, float z, float grab, float pinch,
            GestureTarget tX, GestureTarget tY, GestureTarget tZ, GestureTarget tRoll, GestureTarget tGrab, GestureTarget tPinch,
            GestureTarget tThumb, GestureTarget tIndex, GestureTarget tMiddle, GestureTarget tRing, GestureTarget tPinky)
            {
                if (!h.isPresent) return;

                // Route axes to their selected CCs (Skip if mapped to Pitch, as Pitch is handled above)
                if (tX != GestureTarget::Pitch) sendCC(midiMessages, channel, tX, x);
                if (tY != GestureTarget::Pitch) sendCC(midiMessages, channel, tY, y);
                if (tZ != GestureTarget::Pitch) sendCC(midiMessages, channel, tZ, z);

                sendCC(midiMessages, channel, tGrab, grab);
                sendCC(midiMessages, channel, tPinch, pinch);

                // Fingers (Normalized based on Y height)
                auto getFingerVal = [&](int idx) {
                    if (idx < 0 || idx >= 5) return 0.0f;
                    return juce::jlimit(0.0f, 1.0f, juce::jmap(h.fingers[idx].tipY, minY, maxY, 0.0f, 1.0f));
                    };

                sendCC(midiMessages, channel, tThumb, getFingerVal(0));
                sendCC(midiMessages, channel, tIndex, getFingerVal(1));
                sendCC(midiMessages, channel, tMiddle, getFingerVal(2));
                sendCC(midiMessages, channel, tRing, getFingerVal(3));
                sendCC(midiMessages, channel, tPinky, getFingerVal(4));
            };

        // Process Left CCs
        processHandCCs(left, chLeft, leftX, leftY, leftZ, leftGrab, leftPinch,
            leftXTarget, leftYTarget, leftZTarget, leftRollTarget, leftGrabTarget, leftPinchTarget,
            lThumb, lIndex, lMiddle, lRing, lPinky);

        // Process Right CCs
        processHandCCs(right, chRight, rightX, rightY, rightZ, rightGrab, rightPinch,
            rightXTarget, rightYTarget, rightZTarget, rightRollTarget, rightGrabTarget, rightPinchTarget,
            rThumb, rIndex, rMiddle, rRing, rPinky);
    }

private:
    struct HandNoteState {
        int lastNote = -1;
        bool isNoteOn = false;
    };

    HandNoteState leftNoteState;
    HandNoteState rightNoteState;
    ScaleQuantiser quantiser;

    // MPE note logic
    void handleNoteLogic(juce::MidiBuffer& midi, float val, int root, int scale, int octaveRange, int channel, HandNoteState& state) {
        if (val >= 0.0f) {
            float minNote = 48.0f;
            float maxNote = 48.0f + (octaveRange * 12.0f);
            float exactNote = juce::jmap(val, 0.0f, 1.0f, minNote, maxNote);

            // Quantise to nearest note
            float quantiserInput = exactNote / 127.0f;
            int targetNote = quantiser.getQuantisedNote(quantiserInput, root, scale);

            if (targetNote != state.lastNote || !state.isNoteOn) {
                if (state.isNoteOn) midi.addEvent(juce::MidiMessage::noteOff(channel, state.lastNote), 0);
                midi.addEvent(juce::MidiMessage::noteOn(channel, targetNote, (juce::uint8)100), 0);
                state.lastNote = targetNote;
                state.isNoteOn = true;
            }
        }
        else if (state.isNoteOn) {
            midi.addEvent(juce::MidiMessage::noteOff(channel, state.lastNote), 0);
            state.isNoteOn = false;
        }
    }

    // CC Sender
    void sendCC(juce::MidiBuffer& midi, int channel, GestureTarget target, float val) {
        if (val < 0.0f) return;

        int cc = -1;
        int value7bit = (int)(val * 127.0f);
        bool isSwitch = false;

        switch (target) {
        case GestureTarget::Volume:     cc = 7; break;
        case GestureTarget::Modulation: cc = 1; break;
        case GestureTarget::Expression: cc = 11; break;
        case GestureTarget::Breath:     cc = 2; break;
        case GestureTarget::Cutoff:     cc = 74; break;
        case GestureTarget::Resonance:  cc = 71; break;
        case GestureTarget::Attack:     cc = 73; break;
        case GestureTarget::Release:    cc = 72; break;
        case GestureTarget::Vibrato:    cc = 76; break;
        case GestureTarget::Pan:        cc = 10; break;
        case GestureTarget::Reverb:     cc = 91; break;
        case GestureTarget::Chorus:     cc = 93; break;
        case GestureTarget::Macro1:     cc = 20; break;
        case GestureTarget::Macro2:     cc = 21; break;
        case GestureTarget::Macro3:     cc = 22; break;
        case GestureTarget::Macro4:     cc = 23; break;
        case GestureTarget::Sustain:    cc = 64; isSwitch = true; break;
        case GestureTarget::Portamento: cc = 65; isSwitch = true; break;
        default: return;
        }

        if (isSwitch) {
            value7bit = (val > 0.5f) ? 127 : 0;
        }

        midi.addEvent(juce::MidiMessage::controllerEvent(channel, cc, value7bit), 0);
    }
};