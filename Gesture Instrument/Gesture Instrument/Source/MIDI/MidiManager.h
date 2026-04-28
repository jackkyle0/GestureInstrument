#pragma once

#include <JuceHeader.h>
#include <algorithm> 
#include <array>
#include "GestureTarget.h"
#include "MusicalRangeMode.h" 
#include "../Helpers/HandData.h"
#include "../Helpers/ScaleQuantiser.h"

class MidiManager {
private:
    // ======================================================
    // 1. MEMORY STRUCTS 
    // ======================================================
    struct MpeVoice {
        bool isActive = false;
        int note = -1;
        int channel = 2;
        float startX = 0.0f;
        float startY = 0.0f;
        float startZ = 0.0f;
    };

    struct HandMpeState {
        MpeVoice voices[5];
        bool isTriggered = false;
    };

    struct HandNoteState {
        std::vector<int> activeNotes;
        bool isNoteOn = false;
    };

    HandMpeState leftMpeState;
    HandMpeState rightMpeState;
    HandNoteState leftNoteState;
    HandNoteState rightNoteState;
    ScaleQuantiser quantiser;

public:
    MidiManager() {
        for (int i = 0; i < 5; ++i) {
            leftMpeState.voices[i].channel = 2 + i;

            // Skip Channel 10 (Drums) for the right hand!
            int rightCh = 7 + i;
            if (rightCh >= 10) rightCh++;
            rightMpeState.voices[i].channel = rightCh;
        }
    }

    void updateCustomScale(const std::vector<int>& newScale) {
        quantiser.customIntervals = newScale;
    }

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
        MusicalRangeMode rangeMode, int startNote, int endNote, bool isMpeEnabled, bool enableSplitXAxis,
        int mpePitchAxis, int mpeTimbreAxis, int mpePressureAxis,
        std::array<bool, 7> leftDiatonicDegrees, std::array<bool, 7> leftAllowedRoots, int leftInversionMode, bool leftDropBass,
        std::array<bool, 7> rightDiatonicDegrees, std::array<bool, 7> rightAllowedRoots, int rightInversionMode, bool rightDropBass,
        bool chordEngineEnabled,
        std::atomic<int>* leftNotesOut, std::atomic<int>* rightNotesOut) {

        auto normalize = [](float rawValue, float minBound, float maxBound, float sensitivityMultiplier) {
            float mappedValue = juce::jmap(rawValue, minBound, maxBound, 0.0f, 1.0f);
            float centerOffset = 0.5f;
            mappedValue = centerOffset + ((mappedValue - centerOffset) * sensitivityMultiplier);
            return juce::jlimit(0.0f, 1.0f, mappedValue);
            };

        float centerX = (minX + maxX) / 2.0f;
        float leftMaxX = enableSplitXAxis ? centerX : maxX;
        float rightMinX = enableSplitXAxis ? centerX : minX;

        float leftX = -1.0f, leftY = -1.0f, leftZ = -1.0f, leftRoll = -1.0f, leftGrab = -1.0f, leftPinch = -1.0f;
        if (leftHand.isPresent) {
            leftX = normalize(leftHand.currentHandPositionX, minX, leftMaxX, sensitivity);
            leftY = normalize(leftHand.currentHandPositionY, minY, maxY, sensitivity);
            leftZ = 1.0f - normalize(leftHand.currentHandPositionZ, minZ, maxZ, sensitivity);
            leftRoll = normalize(-leftHand.currentWristRotation, -1.5f, 1.5f, 1.0f);
            leftGrab = leftHand.grabStrength;
            leftPinch = leftHand.pinchStrength;
        }

        float rightX = -1.0f, rightY = -1.0f, rightZ = -1.0f, rightRoll = -1.0f, rightGrab = -1.0f, rightPinch = -1.0f;
        if (rightHand.isPresent) {
            rightX = normalize(rightHand.currentHandPositionX, rightMinX, maxX, sensitivity);
            rightY = normalize(rightHand.currentHandPositionY, minY, maxY, sensitivity);
            rightZ = 1.0f - normalize(rightHand.currentHandPositionZ, minZ, maxZ, sensitivity);
            rightRoll = normalize(-rightHand.currentWristRotation, -1.5f, 1.5f, 1.0f);
            rightGrab = rightHand.grabStrength;
            rightPinch = rightHand.pinchStrength;
        }

        auto getAxisValue = [&](GestureTarget targetSearch, float x, float y, float z, float roll, float grab, float pinch,
            GestureTarget tx, GestureTarget ty, GestureTarget tz, GestureTarget troll, GestureTarget tgrab, GestureTarget tpinch) {
                if (tx == targetSearch) return x; if (ty == targetSearch) return y; if (tz == targetSearch) return z;
                if (troll == targetSearch) return roll; if (tgrab == targetSearch) return grab; if (tpinch == targetSearch) return pinch;
                return -1.0f;
            };

        float leftPitchVal = getAxisValue(GestureTarget::Pitch, leftX, leftY, leftZ, leftRoll, leftGrab, leftPinch, leftXTarget, leftYTarget, leftZTarget, leftRollTarget, leftGrabTarget, leftPinchTarget);
        float leftTriggerVal = getAxisValue(GestureTarget::NoteTrigger, leftX, leftY, leftZ, leftRoll, leftGrab, leftPinch, leftXTarget, leftYTarget, leftZTarget, leftRollTarget, leftGrabTarget, leftPinchTarget);
        float leftVolumeVal = getAxisValue(GestureTarget::Volume, leftX, leftY, leftZ, leftRoll, leftGrab, leftPinch, leftXTarget, leftYTarget, leftZTarget, leftRollTarget, leftGrabTarget, leftPinchTarget);

        float rightPitchVal = getAxisValue(GestureTarget::Pitch, rightX, rightY, rightZ, rightRoll, rightGrab, rightPinch, rightXTarget, rightYTarget, rightZTarget, rightRollTarget, rightGrabTarget, rightPinchTarget);
        float rightTriggerVal = getAxisValue(GestureTarget::NoteTrigger, rightX, rightY, rightZ, rightRoll, rightGrab, rightPinch, rightXTarget, rightYTarget, rightZTarget, rightRollTarget, rightGrabTarget, rightPinchTarget);
        float rightVolumeVal = getAxisValue(GestureTarget::Volume, rightX, rightY, rightZ, rightRoll, rightGrab, rightPinch, rightXTarget, rightYTarget, rightZTarget, rightRollTarget, rightGrabTarget, rightPinchTarget);

        float globalVolumeVal = std::max(leftVolumeVal, rightVolumeVal);

        if (isMpeEnabled) {
            handleMpeLogic(midiMessages, leftPitchVal, leftTriggerVal, globalVolumeVal, rootNote, scaleType, leftMpeState, invertNoteTrigger, rangeMode, octaveRange, startNote, endNote, leftHand, minX, leftMaxX, minY, maxY, minZ, maxZ, mpePitchAxis, mpeTimbreAxis, mpePressureAxis, leftDiatonicDegrees, leftAllowedRoots, leftInversionMode, leftDropBass, chordEngineEnabled, leftNotesOut);
            handleMpeLogic(midiMessages, rightPitchVal, rightTriggerVal, globalVolumeVal, rootNote, scaleType, rightMpeState, invertNoteTrigger, rangeMode, octaveRange, startNote, endNote, rightHand, rightMinX, maxX, minY, maxY, minZ, maxZ, mpePitchAxis, mpeTimbreAxis, mpePressureAxis, rightDiatonicDegrees, rightAllowedRoots, rightInversionMode, rightDropBass, chordEngineEnabled, rightNotesOut);

            if (globalVolumeVal >= 0.0f) sendCC(midiMessages, 1, GestureTarget::Volume, globalVolumeVal);
        }
        else {
            int leftChannel = 2;
            int rightChannel = 3;

            handleNoteLogic(midiMessages, leftPitchVal, leftTriggerVal, globalVolumeVal, rootNote, scaleType, leftChannel, leftNoteState, invertNoteTrigger, rangeMode, octaveRange, startNote, endNote, leftDiatonicDegrees, leftAllowedRoots, leftInversionMode, leftDropBass, chordEngineEnabled, leftNotesOut);
            handleNoteLogic(midiMessages, rightPitchVal, rightTriggerVal, globalVolumeVal, rootNote, scaleType, rightChannel, rightNoteState, invertNoteTrigger, rangeMode, octaveRange, startNote, endNote, rightDiatonicDegrees, rightAllowedRoots, rightInversionMode, rightDropBass, chordEngineEnabled, rightNotesOut);

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

    void panicLeft(juce::MidiBuffer& midiMessages) {
        midiMessages.addEvent(juce::MidiMessage::controllerEvent(2, 123, 0), 0);
        midiMessages.addEvent(juce::MidiMessage::controllerEvent(2, 120, 0), 0);
        midiMessages.addEvent(juce::MidiMessage::controllerEvent(2, 64, 0), 0);

        if (leftNoteState.isNoteOn) {
            for (int note : leftNoteState.activeNotes) {
                midiMessages.addEvent(juce::MidiMessage::noteOff(2, note), 0);
            }
            leftNoteState.activeNotes.clear();
            leftNoteState.isNoteOn = false;
        }
    }

    void panicRight(juce::MidiBuffer& midiMessages) {
        midiMessages.addEvent(juce::MidiMessage::controllerEvent(3, 123, 0), 0);
        midiMessages.addEvent(juce::MidiMessage::controllerEvent(3, 120, 0), 0);
        midiMessages.addEvent(juce::MidiMessage::controllerEvent(3, 64, 0), 0);

        if (rightNoteState.isNoteOn) {
            for (int note : rightNoteState.activeNotes) {
                midiMessages.addEvent(juce::MidiMessage::noteOff(3, note), 0);
            }
            rightNoteState.activeNotes.clear();
            rightNoteState.isNoteOn = false;
        }
    }

private:

    std::vector<int> buildChordShape(int targetNote, bool chordEngineEnabled, int scaleType, int rootNote,
        const std::array<bool, 7>& diatonicDegrees, int inversionMode, bool dropBass) {

        std::vector<int> newChord;
        if (targetNote == -1) return newChord;

        bool isDiatonic = (scaleType > 0 && scaleType < 12);

        if (!chordEngineEnabled ||!isDiatonic) {
            newChord.push_back(targetNote);
        }
        else {
            std::vector<int> intervals = quantiser.getScaleIntervals(scaleType);
            int numNotesInScale = (int)intervals.size();
            int relativeNote = (targetNote - rootNote) % 12;
            if (relativeNote < 0) relativeNote += 12;

            int scaleIndex = 0;
            for (int i = 0; i < numNotesInScale; ++i) {
                if (intervals[i] == relativeNote) { scaleIndex = i; break; }
            }

            int baseRoot = targetNote - relativeNote;
            for (int d = 0; d < 7; ++d) {
                if (diatonicDegrees[d]) {
                    int wrappedIndex = (scaleIndex + d) % numNotesInScale;
                    int octavesToAdd = (scaleIndex + d) / numNotesInScale;
                    newChord.push_back(baseRoot + intervals[wrappedIndex] + (octavesToAdd * 12));
                }
            }
        }

        if (newChord.size() >= 2) {
            if (inversionMode == 1) { newChord[0] += 12; }
            else if (inversionMode == 2 && newChord.size() >= 3) { newChord[0] += 12; newChord[1] += 12; }

            std::sort(newChord.begin(), newChord.end());
            if (dropBass) newChord[0] -= 12;
        }

        return newChord;
    }


    void handleNoteLogic(juce::MidiBuffer& midiMessages, float pitchAxisValue, float triggerAxisValue, float volumeAxisValue,
        int rootNote, int scaleType, int channel, HandNoteState& state,
        bool invertNoteTrigger, MusicalRangeMode mode, int range, int startNote, int endNote,
        std::array<bool, 7> diatonicDegrees, std::array<bool, 7> allowedRoots,
        int inversionMode, bool dropBass, bool chordEngineEnabled, std::atomic<int>* outNotes) {

        if (pitchAxisValue < 0.0f && triggerAxisValue < 0.0f) {
            if (state.isNoteOn) {
                for (int note : state.activeNotes) {
                    midiMessages.addEvent(juce::MidiMessage::noteOff(channel, note), 0);
                }
                if (outNotes) { for (int i = 0; i < 8; ++i) outNotes[i].store(-1); }
                state.activeNotes.clear();
                state.isNoteOn = false;
            }
            return;
        }

        float minNote, maxNote;
        if (mode == MusicalRangeMode::OctaveRange) {
            minNote = (float)startNote + rootNote;
            maxNote = minNote + (range * 12.0f);
        }
        else {
            minNote = (float)startNote;
            maxNote = (float)endNote;
            if (minNote > maxNote) std::swap(minNote, maxNote);
        }

        int targetNote = state.activeNotes.empty() ? -1 : state.activeNotes[0];

        if (pitchAxisValue >= 0.0f) {
            float exactNote = juce::jmap(pitchAxisValue, 0.0f, 1.0f, minNote, maxNote);

            bool isDiatonic = (scaleType > 0 && scaleType < 12);

            std::array<bool, 7> bypassRoots = { true, true, true, true, true, true, true };
            std::array<bool, 7> activeRoots = (chordEngineEnabled && isDiatonic) ? allowedRoots : bypassRoots;
            targetNote = quantiser.getQuantisedNote(exactNote / 127.0f, rootNote, scaleType, activeRoots);
        }
        else if (triggerAxisValue >= 0.0f) {
            targetNote = (int)minNote;
        }

        bool hasExplicitTrigger = (triggerAxisValue >= 0.0f);
        bool isTriggerPressed = true;

        if (hasExplicitTrigger) {
            isTriggerPressed = invertNoteTrigger ? (triggerAxisValue <= 0.5f) : (triggerAxisValue > 0.5f);
        }

        juce::uint8 noteVelocity = 100;
        if (volumeAxisValue >= 0.0f) {
            noteVelocity = (juce::uint8)juce::jlimit(1, 127, (int)(volumeAxisValue * 127.0f));
        }

        std::vector<int> newChord = buildChordShape(targetNote, chordEngineEnabled, scaleType, rootNote, diatonicDegrees, inversionMode, dropBass);

        if (isTriggerPressed && targetNote != -1) {
            bool chordChanged = (newChord != state.activeNotes);

            if (chordChanged || !state.isNoteOn) {
                if (state.isNoteOn) {
                    for (int oldNote : state.activeNotes) {
                        midiMessages.addEvent(juce::MidiMessage::noteOff(channel, oldNote), 0);
                    }
                }

                for (int i = 0; i < 8; ++i) {
                    if (i < newChord.size()) {
                        if (newChord[i] >= 0 && newChord[i] <= 127) {
                            midiMessages.addEvent(juce::MidiMessage::noteOn(channel, newChord[i], noteVelocity), 0);
                        }
                        if (outNotes) outNotes[i].store(newChord[i]);
                    }
                    else {
                        if (outNotes) outNotes[i].store(-1);
                    }
                }

                state.activeNotes = newChord;
                state.isNoteOn = true;
            }
        }
        else if (state.isNoteOn) {
            for (int oldNote : state.activeNotes) {
                midiMessages.addEvent(juce::MidiMessage::noteOff(channel, oldNote), 0);
            }
            if (outNotes) { for (int i = 0; i < 8; ++i) outNotes[i].store(-1); }
            state.activeNotes.clear();
            state.isNoteOn = false;
        }
    }

    void handleMpeLogic(juce::MidiBuffer& midiMessages, float pitchAxisValue, float triggerAxisValue, float volumeAxisValue,
        int rootNote, int scaleType, HandMpeState& state, bool invertNoteTrigger, MusicalRangeMode mode, int range, int startNote, int endNote,
        const HandData& hand, float minX, float maxX, float minY, float maxY, float minZ, float maxZ,
        int mpePitchAxis, int mpeTimbreAxis, int mpePressureAxis,
        std::array<bool, 7> diatonicDegrees, std::array<bool, 7> allowedRoots,
        int inversionMode, bool dropBass, bool chordEngineEnabled, std::atomic<int>* outNotes) {

        if (pitchAxisValue < 0.0f && triggerAxisValue < 0.0f) {
            if (state.isTriggered) {
                for (int i = 0; i < 5; ++i) {
                    if (state.voices[i].isActive) {
                        midiMessages.addEvent(juce::MidiMessage::noteOff(state.voices[i].channel, state.voices[i].note), 0);
                        state.voices[i].isActive = false;
                    }
                }
                if (outNotes) { for (int i = 0; i < 8; ++i) outNotes[i].store(-1); }
                state.isTriggered = false;
            }
            return;
        }

        float minBound = (mode == MusicalRangeMode::OctaveRange) ? (float)startNote + rootNote : (float)std::min(startNote, endNote);
        float maxBound = (mode == MusicalRangeMode::OctaveRange) ? minBound + (range * 12.0f) : (float)std::max(startNote, endNote);

        int targetNote = -1;
        for (int i = 0; i < 5; ++i) { if (state.voices[i].isActive) { targetNote = state.voices[0].note; break; } }

        if (pitchAxisValue >= 0.0f) {
            float exactNote = juce::jmap(pitchAxisValue, 0.0f, 1.0f, minBound, maxBound);

            bool isDiatonic = (scaleType > 0 && scaleType < 12);
            std::array<bool, 7> bypassRoots = { true, true, true, true, true, true, true };
            std::array<bool, 7> activeRoots = (chordEngineEnabled && isDiatonic) ? allowedRoots : bypassRoots;

            targetNote = quantiser.getQuantisedNote(exactNote / 127.0f, rootNote, scaleType, activeRoots);
        }
        else if (triggerAxisValue >= 0.0f) {
            targetNote = (int)minBound;
        }

        bool isTriggerPressed = true;
        if (triggerAxisValue >= 0.0f) {
            isTriggerPressed = invertNoteTrigger ? (triggerAxisValue <= 0.5f) : (triggerAxisValue > 0.5f);
        }

        juce::uint8 noteVelocity = 100;
        if (volumeAxisValue >= 0.0f) noteVelocity = (juce::uint8)juce::jlimit(1, 127, (int)(volumeAxisValue * 127.0f));

        std::vector<int> newChord = buildChordShape(targetNote, chordEngineEnabled, scaleType, rootNote, diatonicDegrees, inversionMode, dropBass);

        if (isTriggerPressed && targetNote != -1) {
            bool chordChanged = false;
            for (int i = 0; i < newChord.size() && i < 5; ++i) {
                if (state.voices[i].note != newChord[i]) chordChanged = true;
            }

            if (chordChanged || !state.isTriggered) {
                if (state.isTriggered) {
                    for (int i = 0; i < 5; ++i) {
                        if (state.voices[i].isActive) {
                            midiMessages.addEvent(juce::MidiMessage::noteOff(state.voices[i].channel, state.voices[i].note), 0);
                            state.voices[i].isActive = false;
                        }
                    }
                }
                for (int i = 0; i < 8; ++i) {
                    if (i < newChord.size() && i < 5) {
                        auto& voice = state.voices[i];
                        if (newChord[i] >= 0 && newChord[i] <= 127) {
                            voice.note = newChord[i];
                            voice.isActive = true;

                            int fingerIdx = std::min(i + 1, 4);

                            voice.startX = hand.fingers[fingerIdx].tipX;
                            voice.startY = hand.fingers[fingerIdx].tipY;
                            voice.startZ = hand.fingers[fingerIdx].tipZ;

                            midiMessages.addEvent(juce::MidiMessage::noteOn(voice.channel, voice.note, noteVelocity), 0);
                            midiMessages.addEvent(juce::MidiMessage::pitchWheel(voice.channel, 8192), 0);
                        }
                        if (outNotes) outNotes[i].store(newChord[i]);
                    }
                    else {
                        if (outNotes) outNotes[i].store(-1);
                    }
                }
                state.isTriggered = true;
            }
            else {
                for (int i = 0; i < 5; ++i) {
                    auto& voice = state.voices[i];
                    if (voice.isActive) {

                        int fingerIdx = std::min(i + 1, 4);
                        const auto& finger = hand.fingers[fingerIdx];

                        auto getDelta = [&](int axis) {
                            if (axis == 1) return finger.tipX - voice.startX;
                            if (axis == 2) return finger.tipY - voice.startY;
                            if (axis == 3) return finger.tipZ - voice.startZ;
                            return 0.0f;
                            };

                        auto getAbsolute = [&](int axis) {
                            if (axis == 1) return juce::jlimit(0.0f, 127.0f, juce::jmap(finger.tipX, minX, maxX, 0.0f, 127.0f));
                            if (axis == 2) return juce::jlimit(0.0f, 127.0f, juce::jmap(finger.tipY, minY, maxY, 0.0f, 127.0f));
                            if (axis == 3) return juce::jlimit(0.0f, 127.0f, juce::jmap(finger.tipZ, minZ, maxZ, 127.0f, 0.0f));
                            return 0.0f;
                            };

                        float pitchDelta = getDelta(mpePitchAxis);
                        int pbValue = (int)juce::jmap(pitchDelta, -150.0f, 150.0f, 0.0f, 16383.0f);
                        midiMessages.addEvent(juce::MidiMessage::pitchWheel(voice.channel, juce::jlimit(0, 16383, pbValue)), 0);

                        int timbre = (int)getAbsolute(mpeTimbreAxis);
                        midiMessages.addEvent(juce::MidiMessage::controllerEvent(voice.channel, 74, timbre), 0);

                        int pressure = (int)getAbsolute(mpePressureAxis);
                        midiMessages.addEvent(juce::MidiMessage::channelPressureChange(voice.channel, pressure), 0);
                    }
                }
            }
        }
        else if (state.isTriggered) {
            for (int i = 0; i < 5; ++i) {
                if (state.voices[i].isActive) {
                    midiMessages.addEvent(juce::MidiMessage::noteOff(state.voices[i].channel, state.voices[i].note), 0);
                    state.voices[i].isActive = false;
                }
            }
            if (outNotes) { for (int i = 0; i < 8; ++i) outNotes[i].store(-1); }
            state.isTriggered = false;
        }
    }
};