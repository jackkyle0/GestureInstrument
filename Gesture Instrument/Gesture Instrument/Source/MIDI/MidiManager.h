#pragma once

#include <JuceHeader.h>
#include <algorithm> 
#include "GestureTarget.h"
#include "MusicalRangeMode.h" 
#include "../Helpers/HandData.h"
#include "../Helpers/ScaleQuantiser.h"

class MidiManager {
public:
    void sendProgramChange(juce::MidiBuffer& midiMessages, int programNumber) {
        for (int ch = 1; ch <= 15; ++ch) {
            midiMessages.addEvent(juce::MidiMessage::programChange(ch, programNumber), 0);
        }
    }

    void processHandData(juce::MidiBuffer& midiMessages,
        const HandData& leftHand, const HandData& rightHand,
        float sensitivity, float minX, float maxX, float minY, float maxY, float minZ, float maxZ,
        GestureTarget leftXTarget, GestureTarget leftYTarget, GestureTarget leftZTarget, GestureTarget leftRollTarget,
        GestureTarget leftGrabTarget, GestureTarget leftPinchTarget,
        GestureTarget lThumb, GestureTarget lIndex, GestureTarget lMiddle, GestureTarget lRing, GestureTarget lPinky,
        GestureTarget rightXTarget, GestureTarget rightYTarget, GestureTarget rightZTarget, GestureTarget rightRollTarget,
        GestureTarget rightGrabTarget, GestureTarget rightPinchTarget,
        GestureTarget rThumb, GestureTarget rIndex, GestureTarget rMiddle, GestureTarget rRing, GestureTarget rPinky,
        int rootNote, int scaleType, int octaveRange, bool invertNoteTrigger,
        MusicalRangeMode rangeMode, int startOctave, int endOctave, bool isMpeEnabled) {

        auto normalize = [](float rawValue, float minBound, float maxBound, float sensitivityMultiplier) {
            float mappedValue = juce::jmap(rawValue, minBound, maxBound, 0.0f, 1.0f);
            float centerOffset = 0.5f;
            mappedValue = centerOffset + ((mappedValue - centerOffset) * sensitivityMultiplier);
            return juce::jlimit(0.0f, 1.0f, mappedValue);
            };

        float leftX = -1.0f, leftY = -1.0f, leftZ = -1.0f, leftRoll = -1.0f, leftGrab = -1.0f, leftPinch = -1.0f;
        if (leftHand.isPresent) {
            leftX = normalize(leftHand.currentHandPositionX, minX, maxX, sensitivity);
            leftY = normalize(leftHand.currentHandPositionY, minY, maxY, sensitivity);
            leftZ = 1.0f - normalize(leftHand.currentHandPositionZ, minZ, maxZ, sensitivity);
            leftRoll = normalize(-leftHand.currentWristRotation, -1.5f, 1.5f, 1.0f);
            leftGrab = leftHand.grabStrength;
            leftPinch = leftHand.pinchStrength;
        }

        float rightX = -1.0f, rightY = -1.0f, rightZ = -1.0f, rightRoll = -1.0f, rightGrab = -1.0f, rightPinch = -1.0f;
        if (rightHand.isPresent) {
            rightX = normalize(rightHand.currentHandPositionX, minX, maxX, sensitivity);
            rightY = normalize(rightHand.currentHandPositionY, minY, maxY, sensitivity);
            rightZ = 1.0f - normalize(rightHand.currentHandPositionZ, minZ, maxZ, sensitivity);
            rightRoll = normalize(-rightHand.currentWristRotation, -1.5f, 1.5f, 1.0f);
            rightGrab = rightHand.grabStrength;
            rightPinch = rightHand.pinchStrength;
        }

        if (isMpeEnabled) {
           
            juce::Logger::writeToLog("MPE is enabled but not fully implemented");
        }
        else {
            int leftChannel = 2;
            int rightChannel = 3;

            auto getAxisValue = [&](GestureTarget targetSearch, float x, float y, float z, float roll, float grab, float pinch,
                GestureTarget tx, GestureTarget ty, GestureTarget tz, GestureTarget troll, GestureTarget tgrab, GestureTarget tpinch) {
                    if (tx == targetSearch) return x;
                    if (ty == targetSearch) return y;
                    if (tz == targetSearch) return z;
                    if (troll == targetSearch) return roll;
                    if (tgrab == targetSearch) return grab;
                    if (tpinch == targetSearch) return pinch;
                    return -1.0f;
                };

            float leftPitchVal = getAxisValue(GestureTarget::Pitch, leftX, leftY, leftZ, leftRoll, leftGrab, leftPinch, leftXTarget, leftYTarget, leftZTarget, leftRollTarget, leftGrabTarget, leftPinchTarget);
            float leftTriggerVal = getAxisValue(GestureTarget::NoteTrigger, leftX, leftY, leftZ, leftRoll, leftGrab, leftPinch, leftXTarget, leftYTarget, leftZTarget, leftRollTarget, leftGrabTarget, leftPinchTarget);
            float leftVolumeVal = getAxisValue(GestureTarget::Volume, leftX, leftY, leftZ, leftRoll, leftGrab, leftPinch, leftXTarget, leftYTarget, leftZTarget, leftRollTarget, leftGrabTarget, leftPinchTarget);

            float rightPitchVal = getAxisValue(GestureTarget::Pitch, rightX, rightY, rightZ, rightRoll, rightGrab, rightPinch, rightXTarget, rightYTarget, rightZTarget, rightRollTarget, rightGrabTarget, rightPinchTarget);
            float rightTriggerVal = getAxisValue(GestureTarget::NoteTrigger, rightX, rightY, rightZ, rightRoll, rightGrab, rightPinch, rightXTarget, rightYTarget, rightZTarget, rightRollTarget, rightGrabTarget, rightPinchTarget);
            float rightVolumeVal = getAxisValue(GestureTarget::Volume, rightX, rightY, rightZ, rightRoll, rightGrab, rightPinch, rightXTarget, rightYTarget, rightZTarget, rightRollTarget, rightGrabTarget, rightPinchTarget);

            float globalVolumeVal = std::max(leftVolumeVal, rightVolumeVal);

            handleNoteLogic(midiMessages, leftPitchVal, leftTriggerVal, globalVolumeVal, rootNote, scaleType, leftChannel, leftNoteState, invertNoteTrigger, rangeMode, octaveRange, startOctave, endOctave);
            handleNoteLogic(midiMessages, rightPitchVal, rightTriggerVal, globalVolumeVal, rootNote, scaleType, rightChannel, rightNoteState, invertNoteTrigger, rangeMode, octaveRange, startOctave, endOctave);

            if (globalVolumeVal >= 0.0f) {
                sendCC(midiMessages, leftChannel, GestureTarget::Volume, globalVolumeVal);
                sendCC(midiMessages, rightChannel, GestureTarget::Volume, globalVolumeVal);
            }

            auto processHandCCs = [&](const HandData& hand, int channel, float xValue, float yValue, float zValue, float rollValue, float grabValue, float pinchValue,
                GestureTarget tX, GestureTarget tY, GestureTarget tZ, GestureTarget tRoll, GestureTarget tGrab, GestureTarget tPinch,
                GestureTarget tThumb, GestureTarget tIndex, GestureTarget tMiddle, GestureTarget tRing, GestureTarget tPinky) {
                    if (!hand.isPresent) return;

                    auto isValidCC = [](GestureTarget t) { return t != GestureTarget::Pitch && t != GestureTarget::NoteTrigger && t != GestureTarget::Volume; };

                    if (isValidCC(tX)) sendCC(midiMessages, channel, tX, xValue);
                    if (isValidCC(tY)) sendCC(midiMessages, channel, tY, yValue);
                    if (isValidCC(tZ)) sendCC(midiMessages, channel, tZ, zValue);
                    if (isValidCC(tRoll)) sendCC(midiMessages, channel, tRoll, rollValue);
                    if (isValidCC(tGrab)) sendCC(midiMessages, channel, tGrab, grabValue);
                    if (isValidCC(tPinch)) sendCC(midiMessages, channel, tPinch, pinchValue);

                    auto getFingerVal = [&](int fingerIndex) {
                        if (fingerIndex < 0 || fingerIndex >= 5) return 0.0f;
                        return juce::jlimit(0.0f, 1.0f, juce::jmap(hand.fingers[fingerIndex].tipY, minY, maxY, 0.0f, 1.0f));
                        };

                    sendCC(midiMessages, channel, tThumb, getFingerVal(0));
                    sendCC(midiMessages, channel, tIndex, getFingerVal(1));
                    sendCC(midiMessages, channel, tMiddle, getFingerVal(2));
                    sendCC(midiMessages, channel, tRing, getFingerVal(3));
                    sendCC(midiMessages, channel, tPinky, getFingerVal(4));
                };

            processHandCCs(leftHand, leftChannel, leftX, leftY, leftZ, leftRoll, leftGrab, leftPinch, leftXTarget, leftYTarget, leftZTarget, leftRollTarget, leftGrabTarget, leftPinchTarget, lThumb, lIndex, lMiddle, lRing, lPinky);
            processHandCCs(rightHand, rightChannel, rightX, rightY, rightZ, rightRoll, rightGrab, rightPinch, rightXTarget, rightYTarget, rightZTarget, rightRollTarget, rightGrabTarget, rightPinchTarget, rThumb, rIndex, rMiddle, rRing, rPinky);
        }
    }

    void sendCC(juce::MidiBuffer& midiMessages, int channel, GestureTarget target, float axisValue) {
        if (axisValue < 0.0f) return;
        int controllerNumber = -1;
        int value7bit = (int)(axisValue * 127.0f);
        bool isSwitch = false;

        if (target == GestureTarget::Volume) liveVolume.store(axisValue);
        else if (target == GestureTarget::Pan) livePan.store(axisValue);
        else if (target == GestureTarget::Modulation) liveModulation.store(axisValue);
        else if (target == GestureTarget::Expression) liveExpression.store(axisValue);
        else if (target == GestureTarget::Cutoff) liveCutoff.store(axisValue);
        else if (target == GestureTarget::Resonance) liveResonance.store(axisValue);
        else if (target == GestureTarget::Attack) liveAttack.store(axisValue);
        else if (target == GestureTarget::Release) liveRelease.store(axisValue);
        else if (target == GestureTarget::Reverb) liveReverb.store(axisValue);
        else if (target == GestureTarget::Chorus) liveChorus.store(axisValue);
        else if (target == GestureTarget::Vibrato) liveVibrato.store(axisValue);
        else if (target == GestureTarget::Waveform) liveWaveform.store(axisValue);
        else if (target == GestureTarget::Sustain) liveSustain.store(axisValue);
        else if (target == GestureTarget::Portamento) livePortamento.store(axisValue);

        switch (target) {
        case GestureTarget::Volume:      controllerNumber = 7; break;
        case GestureTarget::Modulation:  controllerNumber = 1; break;
        case GestureTarget::Expression:  controllerNumber = 11; break;
        case GestureTarget::Breath:      controllerNumber = 2; break;
        case GestureTarget::Cutoff:      controllerNumber = 74; break;
        case GestureTarget::Resonance:   controllerNumber = 71; break;
        case GestureTarget::Attack:      controllerNumber = 73; break;
        case GestureTarget::Release:     controllerNumber = 72; break;
        case GestureTarget::Vibrato:     controllerNumber = 76; break;
        case GestureTarget::Pan:         controllerNumber = 10; break;
        case GestureTarget::Reverb:      controllerNumber = 91; break;
        case GestureTarget::Chorus:      controllerNumber = 93; break;
        case GestureTarget::Sustain:     controllerNumber = 64; isSwitch = true; break;
        case GestureTarget::Portamento:  controllerNumber = 65; isSwitch = true; break;
        default: return;
        }

        if (isSwitch) value7bit = (axisValue > 0.5f) ? 127 : 0;
        midiMessages.addEvent(juce::MidiMessage::controllerEvent(channel, controllerNumber, value7bit), 0);
    }

    std::atomic<float> liveVolume{ -1.0f };
    std::atomic<float> livePan{ -1.0f };
    std::atomic<float> liveModulation{ -1.0f };
    std::atomic<float> liveExpression{ -1.0f };
    std::atomic<float> liveCutoff{ -1.0f };
    std::atomic<float> liveResonance{ -1.0f };
    std::atomic<float> liveAttack{ -1.0f };
    std::atomic<float> liveRelease{ -1.0f };
    std::atomic<float> liveReverb{ -1.0f };
    std::atomic<float> liveChorus{ -1.0f };
    std::atomic<float> liveVibrato{ -1.0f };
    std::atomic<float> liveWaveform{ -1.0f };
    std::atomic<float> liveSustain{ -1.0f };

    std::atomic<float> livePortamento{ -1.0f };



private:
    struct HandNoteState {
        int lastNote = -1;
        bool isNoteOn = false;
    };

    HandNoteState leftNoteState;
    HandNoteState rightNoteState;
    ScaleQuantiser quantiser;

    void handleNoteLogic(juce::MidiBuffer& midiMessages, float pitchAxisValue, float triggerAxisValue, float volumeAxisValue,
        int rootNote, int scaleType, int channel, HandNoteState& state,
        bool invertNoteTrigger, MusicalRangeMode mode, int range, int startNote, int endNote) {

        if (pitchAxisValue < 0.0f && triggerAxisValue < 0.0f) {
            if (state.isNoteOn) {
                midiMessages.addEvent(juce::MidiMessage::noteOff(channel, state.lastNote), 0);
                state.isNoteOn = false;
            }
            return;
        }

        float minNote, maxNote;
        if (mode == MusicalRangeMode::OctaveRange) {
            minNote = 48.0f + rootNote;
            maxNote = minNote + (range * 12.0f);
        }
        else {
            minNote = (float)startNote;
            maxNote = (float)endNote;
            if (minNote > maxNote) std::swap(minNote, maxNote);
        }

        int targetNote = state.lastNote;
        if (pitchAxisValue >= 0.0f) {
            float exactNote = juce::jmap(pitchAxisValue, 0.0f, 1.0f, minNote, maxNote);
            targetNote = quantiser.getQuantisedNote(exactNote / 127.0f, rootNote, scaleType);
        }
        else if (triggerAxisValue >= 0.0f) {
            targetNote = (int)minNote;
        }

        bool hasExplicitTrigger = (triggerAxisValue >= 0.0f);
        bool isTriggerPressed = true;

        if (hasExplicitTrigger) {
            if (invertNoteTrigger) isTriggerPressed = (triggerAxisValue <= 0.5f);
            else isTriggerPressed = (triggerAxisValue > 0.5f);
        }

        juce::uint8 noteVelocity = 100;
        if (volumeAxisValue >= 0.0f) {
            int calcVel = (int)(volumeAxisValue * 127.0f);
            noteVelocity = (juce::uint8)juce::jlimit(1, 127, calcVel);
        }

        if (isTriggerPressed) {
            if (targetNote != state.lastNote || !state.isNoteOn) {
                if (state.isNoteOn) {
                    midiMessages.addEvent(juce::MidiMessage::noteOff(channel, state.lastNote), 0);
                }
                midiMessages.addEvent(juce::MidiMessage::noteOn(channel, targetNote, noteVelocity), 0);
                state.lastNote = targetNote;
                state.isNoteOn = true;
            }
        }
        else if (state.isNoteOn) {
            midiMessages.addEvent(juce::MidiMessage::noteOff(channel, state.lastNote), 0);
            state.isNoteOn = false;
        }
    }
};