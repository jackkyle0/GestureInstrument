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
        float sensitivity, float minH, float maxH,
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
        // Calculate 0-1 Values
        float leftX = calculateX(left, sensitivity);
        float leftY = calculateY(left, minH, maxH);
        float rightX = calculateX(right, sensitivity);
        float rightY = calculateY(right, minH, maxH);

        // Calculate Grab/Pinch
        float leftGrab = left.isPresent ? left.grabStrength : -1.0f;
        float leftPinch = left.isPresent ? left.pinchStrength : -1.0f;
        float rightGrab = right.isPresent ? right.grabStrength : -1.0f;
        float rightPinch = right.isPresent ? right.pinchStrength : -1.0f;

        // MPE
        // Left Hand = Channel 2
        // Right Hand = Channel 3
        int chLeft = 2;
        int chRight = 3;

        // Pitch
        // Left Hand Pitch
        float lPitchVal = -1.0f;
        if (leftXTarget == GestureTarget::Pitch) lPitchVal = leftX;
        if (leftYTarget == GestureTarget::Pitch) lPitchVal = leftY;
        handleNoteLogic(midiMessages, lPitchVal, rootNote, scaleType, octaveRange, chLeft, leftNoteState);

        // Right Hand Pitch
        float rPitchVal = -1.0f;
        if (rightXTarget == GestureTarget::Pitch) rPitchVal = rightX;
        if (rightYTarget == GestureTarget::Pitch) rPitchVal = rightY;
        handleNoteLogic(midiMessages, rPitchVal, rootNote, scaleType, octaveRange, chRight, rightNoteState);

        // CC Logic
        auto processHandCCs = [&](const HandData& h, int channel, float x, float y, float grab, float pinch,
            GestureTarget tX, GestureTarget tY, GestureTarget tZ, GestureTarget tRoll, GestureTarget tGrab, GestureTarget tPinch,
            GestureTarget tThumb, GestureTarget tIndex, GestureTarget tMiddle, GestureTarget tRing, GestureTarget tPinky)
            {
                if (!h.isPresent) return;

                sendCC(midiMessages, channel, tX, x);
                sendCC(midiMessages, channel, tY, y);
                sendCC(midiMessages, channel, tGrab, grab);
                sendCC(midiMessages, channel, tPinch, pinch);

                // Fingers
                auto getFingerVal = [&](int idx) {
                    if (idx < 0 || idx >= 5) return 0.0f;
                    return juce::jlimit(0.0f, 1.0f, juce::jmap(h.fingers[idx].tipY, minH, maxH, 0.0f, 1.0f));
                    };

                sendCC(midiMessages, channel, tThumb, getFingerVal(0));
                sendCC(midiMessages, channel, tIndex, getFingerVal(1));
                sendCC(midiMessages, channel, tMiddle, getFingerVal(2));
                sendCC(midiMessages, channel, tRing, getFingerVal(3));
                sendCC(midiMessages, channel, tPinky, getFingerVal(4));
            };

        // Process Left (Channel 2)
        processHandCCs(left, chLeft, leftX, leftY, leftGrab, leftPinch,
            leftXTarget, leftYTarget, leftZTarget, leftRollTarget, leftGrabTarget, leftPinchTarget,
            lThumb, lIndex, lMiddle, lRing, lPinky);

        // Process Right (Channel 3)
        processHandCCs(right, chRight, rightX, rightY, rightGrab, rightPinch,
            rightXTarget, rightYTarget, rightZTarget, rightRollTarget, rightGrabTarget, rightPinchTarget,
            rThumb, rIndex, rMiddle, rRing, rPinky);
    }

private:
    //  struct to track Note On/Off per hand independently
    struct HandNoteState {
        int lastNote = -1;
        bool isNoteOn = false;
    };

    HandNoteState leftNoteState;
    HandNoteState rightNoteState;

    ScaleQuantiser quantiser;

    float calculateX(const HandData& h, float sens) {
        if (!h.isPresent) return -1.0f;
        float range = 200.0f / sens;
        return juce::jlimit(0.0f, 1.0f, juce::jmap(h.currentHandPositionX, -range, range, 0.0f, 1.0f));
    }

    float calculateY(const HandData& h, float min, float max) {
        if (!h.isPresent) return -1.0f;
        return juce::jlimit(0.0f, 1.0f, juce::jmap(h.currentHandPositionY, min, max, 0.0f, 1.0f));
    }

    // MPE note logic
    void handleNoteLogic(juce::MidiBuffer& midi, float val, int root, int scale, int octaveRange, int channel, HandNoteState& state) {
        if (val >= 0.0f) {

            float minNote = 48.0f;
            float maxNote = 48.0f + (octaveRange * 12.0f);

            float exactNote = juce::jmap(val, 0.0f, 1.0f, minNote, maxNote);

            // Quantise to nearest nnote
            float quantiserInput = exactNote / 127.0f; // Scale back to 0-1 for quantiser
            int targetNote = quantiser.getQuantisedNote(quantiserInput, root, scale); 
            float pitchError = exactNote - targetNote;
            

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
        bool isSwitch = false; // For Sustain/Portamento

        switch (target) {
            // Essentials
        case GestureTarget::Volume:     cc = 7; break;
        case GestureTarget::Modulation: cc = 1; break;
        case GestureTarget::Expression: cc = 11; break;
        case GestureTarget::Breath:     cc = 2; break;

            //  Shaping
        case GestureTarget::Cutoff:     cc = 74; break; // Timbre (Brightness)
        case GestureTarget::Resonance:  cc = 71; break; // Timbre (Harmonic)
        case GestureTarget::Attack:     cc = 73; break;
        case GestureTarget::Release:    cc = 72; break;
        case GestureTarget::Vibrato:    cc = 76; break;

            // Spatial
        case GestureTarget::Pan:        cc = 10; break;
        case GestureTarget::Reverb:     cc = 91; break;
        case GestureTarget::Chorus:     cc = 93; break;

            // Macros
        case GestureTarget::Macro1:     cc = 20; break;
        case GestureTarget::Macro2:     cc = 21; break;
        case GestureTarget::Macro3:     cc = 22; break;
        case GestureTarget::Macro4:     cc = 23; break;

            // Switches 
        case GestureTarget::Sustain:    cc = 64; isSwitch = true; break;
        case GestureTarget::Portamento: cc = 65; isSwitch = true; break;

        default: return;
        }

        if (isSwitch) {
            // Convert float 0.0-1.0 to  0 or 127
            value7bit = (val > 0.5f) ? 127 : 0;
        }

        midi.addEvent(juce::MidiMessage::controllerEvent(channel, cc, value7bit), 0);
    }
};