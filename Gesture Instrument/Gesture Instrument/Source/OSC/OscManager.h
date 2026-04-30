#pragma once

#include <JuceHeader.h>
#include "../Helpers/HandData.h"
#include "../MIDI/GestureTarget.h"
#include "../Helpers/ScaleQuantiser.h" 
#include "../Helpers/MusicalRangeMode.h"

class OscManager {
public:
    OscManager() {}
    ~OscManager() {}

    juce::OSCSender sender;

    // Live hud trackers
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
    std::atomic<float> liveDelay{ -1.0f };
    std::atomic<float> liveDistortion{ -1.0f };
    std::atomic<float> liveSustain{ -1.0f };

    void connectSender(const juce::String& targetIP, int targetPort) {
        sender.connect(targetIP, targetPort);
    }

    void updateCustomScale(std::vector<int> newScale) {
        quantiser.customIntervals = newScale;
    }

    // Static params broadcasts
    void sendEnvelopeData(float envelopeShape) {
        juce::OSCMessage leftAttack("/left/attack"); leftAttack.addFloat32(envelopeShape); sender.send(leftAttack);
        juce::OSCMessage rightAttack("/right/attack"); rightAttack.addFloat32(envelopeShape); sender.send(rightAttack);

        juce::OSCMessage leftRelease("/left/release"); leftRelease.addFloat32(envelopeShape); sender.send(leftRelease);
        juce::OSCMessage rightRelease("/right/release"); rightRelease.addFloat32(envelopeShape); sender.send(rightRelease);
    }

    void sendGlobalWaveform(float waveValue) {
        juce::OSCMessage msg("/global/waveform");
        msg.addFloat32(waveValue);
        sender.send(msg);
    }

    // Send raw data
    void sendRawData(const HandData& leftHand, const HandData& rightHand) {
        juce::OSCMessage msg("/gesture/raw");

        auto addHandData = [&](const HandData& hand) {
            if (hand.isPresent) {
                msg.addFloat32(hand.currentHandPositionX);
                msg.addFloat32(hand.currentHandPositionY);
                msg.addFloat32(hand.currentHandPositionZ);
                msg.addFloat32(hand.grabStrength);
            }
            else {
                msg.addFloat32(0.0f); msg.addFloat32(0.0f); msg.addFloat32(0.0f); msg.addFloat32(0.0f);
            }
            };

        addHandData(leftHand);
        addHandData(rightHand);
        sender.send(msg);
    }

    void sendMidiData(const juce::MidiBuffer& buffer) {
        for (const auto metadata : buffer) {
            auto msg = metadata.getMessage();
            if (msg.isNoteOn()) {
                juce::OSCMessage m("/midi/note");
                m.addInt32(1);
                m.addInt32(msg.getNoteNumber());
                m.addInt32(msg.getVelocity());
                sender.send(m);
            }
            else if (msg.isNoteOff()) {
                juce::OSCMessage m("/midi/note");
                m.addInt32(0);
                m.addInt32(msg.getNoteNumber());
                m.addInt32(0);
                sender.send(m);
            }
            else if (msg.isController()) {
                juce::OSCMessage m("/midi/cc");
                m.addInt32(msg.getControllerNumber());
                m.addInt32(msg.getControllerValue());
                sender.send(m);
            }
        }
    }

    // Routing engine
    void processHandData(
        const HandData& leftHand, const HandData& rightHand,
        float sensitivity, float minX, float maxX, float minY, float maxY, float minZ, float maxZ,
        float wMult, float gMult, float pMult,
        GestureTarget leftTargetX, GestureTarget leftTargetY, GestureTarget leftTargetZ, GestureTarget leftTargetRoll, GestureTarget leftTargetGrab, GestureTarget leftTargetPinch,
        GestureTarget lThumb, GestureTarget lIndex, GestureTarget lMiddle, GestureTarget lRing, GestureTarget lPinky,
        GestureTarget rightTargetX, GestureTarget rightTargetY, GestureTarget rightTargetZ, GestureTarget rightTargetRoll, GestureTarget rightTargetGrab, GestureTarget rightTargetPinch,
        GestureTarget rThumb, GestureTarget rIndex, GestureTarget rMiddle, GestureTarget rRing, GestureTarget rPinky,
        int rootNote, int scaleType, int octaveRange, MusicalRangeMode rangeMode, int startNote, int endNote, bool enableSplitXAxis,
        std::atomic<int> activeLeftNotes[8], std::atomic<int> activeRightNotes[8]) {

        auto isTargetMapped = [&](GestureTarget searchTarget, GestureTarget tX, GestureTarget tY, GestureTarget tZ, GestureTarget tRoll, GestureTarget tGrab, GestureTarget tPinch, GestureTarget fThumb, GestureTarget fIndex, GestureTarget fMiddle, GestureTarget fRing, GestureTarget fPinky) {
            return searchTarget == tX || searchTarget == tY || searchTarget == tZ || searchTarget == tRoll || searchTarget == tGrab || searchTarget == tPinch || searchTarget == fThumb || searchTarget == fIndex || searchTarget == fMiddle || searchTarget == fRing || searchTarget == fPinky;
            };

        float centerX = (minX + maxX) / 2.0f;
        float leftMaxX = enableSplitXAxis ? centerX : maxX;
        float rightMinX = enableSplitXAxis ? centerX : minX;

        // Process left hand
        if (leftHand.isPresent) {
            float leftX = normalizeAxis(leftHand.currentHandPositionX, minX, leftMaxX, sensitivity);
            float leftY = normalizeAxis(leftHand.currentHandPositionY, minY, maxY, sensitivity);
            float leftZ = 1.0f - normalizeAxis(leftHand.currentHandPositionZ, minZ, maxZ, sensitivity);
            float baseLeftRoll = normalizeAxis(-leftHand.currentWristRotation, -1.5f, 1.5f, 1.0f);
            float leftRoll = juce::jlimit(0.0f, 1.0f, baseLeftRoll * wMult);
            float leftGrab = juce::jlimit(0.0f, 1.0f, leftHand.grabStrength * gMult);
            float leftPinch = juce::jlimit(0.0f, 1.0f, leftHand.pinchStrength * pMult);

            routeMessage(leftTargetRoll, leftRoll, "left", rootNote, scaleType, octaveRange, rangeMode, startNote, endNote, activeLeftNotes);
            routeMessage(leftTargetX, leftX, "left", rootNote, scaleType, octaveRange, rangeMode, startNote, endNote, activeLeftNotes);
            routeMessage(leftTargetY, leftY, "left", rootNote, scaleType, octaveRange, rangeMode, startNote, endNote, activeLeftNotes);
            routeMessage(leftTargetZ, leftZ, "left", rootNote, scaleType, octaveRange, rangeMode, startNote, endNote, activeLeftNotes);
            routeMessage(leftTargetGrab, leftGrab, "left", rootNote, scaleType, octaveRange, rangeMode, startNote, endNote, activeLeftNotes);
            routeMessage(leftTargetPinch, leftPinch, "left", rootNote, scaleType, octaveRange, rangeMode, startNote, endNote, activeLeftNotes);

            processFingers(leftHand, "left", minY, maxY, rootNote, scaleType, octaveRange, lThumb, lIndex, lMiddle, lRing, lPinky, rangeMode, startNote, endNote, activeLeftNotes);

            // If the trigger isn't mapped, enforce a note to keep synth active
            if (!isTargetMapped(GestureTarget::NoteTrigger, leftTargetX, leftTargetY, leftTargetZ, leftTargetRoll, leftTargetGrab, leftTargetPinch, lThumb, lIndex, lMiddle, lRing, lPinky) && lastLeftMuteSent != 1.0f) {
                juce::OSCMessage msg("/left/note"); msg.addFloat32(1.0f); sender.send(msg);
                lastLeftMuteSent = 1.0f;
            }

            // If vol isnt mapped, send to max
            if (!isTargetMapped(GestureTarget::Volume, leftTargetX, leftTargetY, leftTargetZ, leftTargetRoll, leftTargetGrab, leftTargetPinch, lThumb, lIndex, lMiddle, lRing, lPinky) && lastLeftVolSent != 1.0f) {
                juce::OSCMessage msg("/left/volume"); msg.addFloat32(1.0f); sender.send(msg);
                lastLeftVolSent = 1.0f;
            }
        }
        else {
            for (int i = 0; i < 8; ++i) activeLeftNotes[i].store(-1);
        }

        // Process right hand
        if (rightHand.isPresent) {
            float rightX = normalizeAxis(rightHand.currentHandPositionX, rightMinX, maxX, sensitivity);
            float rightY = normalizeAxis(rightHand.currentHandPositionY, minY, maxY, sensitivity);
            float rightZ = 1.0f - normalizeAxis(rightHand.currentHandPositionZ, minZ, maxZ, sensitivity);
            float baseRightRoll = normalizeAxis(-rightHand.currentWristRotation, -1.5f, 1.5f, 1.0f);
            float rightRoll = juce::jlimit(0.0f, 1.0f, baseRightRoll * wMult);
            float rightGrab = juce::jlimit(0.0f, 1.0f, rightHand.grabStrength * gMult);
            float rightPinch = juce::jlimit(0.0f, 1.0f, rightHand.pinchStrength * pMult);

            routeMessage(rightTargetRoll, rightRoll, "right", rootNote, scaleType, octaveRange, rangeMode, startNote, endNote, activeRightNotes);
            routeMessage(rightTargetX, rightX, "right", rootNote, scaleType, octaveRange, rangeMode, startNote, endNote, activeRightNotes);
            routeMessage(rightTargetY, rightY, "right", rootNote, scaleType, octaveRange, rangeMode, startNote, endNote, activeRightNotes);
            routeMessage(rightTargetZ, rightZ, "right", rootNote, scaleType, octaveRange, rangeMode, startNote, endNote, activeRightNotes);
            routeMessage(rightTargetGrab, rightGrab, "right", rootNote, scaleType, octaveRange, rangeMode, startNote, endNote, activeRightNotes);
            routeMessage(rightTargetPinch, rightPinch, "right", rootNote, scaleType, octaveRange, rangeMode, startNote, endNote, activeRightNotes);

            processFingers(rightHand, "right", minY, maxY, rootNote, scaleType, octaveRange, rThumb, rIndex, rMiddle, rRing, rPinky, rangeMode, startNote, endNote, activeRightNotes);

            if (!isTargetMapped(GestureTarget::NoteTrigger, rightTargetX, rightTargetY, rightTargetZ, rightTargetRoll, rightTargetGrab, rightTargetPinch, rThumb, rIndex, rMiddle, rRing, rPinky) && lastRightMuteSent != 1.0f) {
                juce::OSCMessage msg("/right/note"); msg.addFloat32(1.0f); sender.send(msg);
                lastRightMuteSent = 1.0f;
            }

            if (!isTargetMapped(GestureTarget::Volume, rightTargetX, rightTargetY, rightTargetZ, rightTargetRoll, rightTargetGrab, rightTargetPinch, rThumb, rIndex, rMiddle, rRing, rPinky) && lastRightVolSent != 1.0f) {
                juce::OSCMessage msg("/right/volume"); msg.addFloat32(1.0f); sender.send(msg);
                lastRightVolSent = 1.0f;
            }
        }
        else {
            for (int i = 0; i < 8; ++i) activeRightNotes[i].store(-1);
        }
    }

    // Route messages
    void routeMessage(GestureTarget target, float axisValue, juce::String handPrefix, int rootNote, int scaleType, int octaveRange, MusicalRangeMode mode, int startNote, int endNote, std::atomic<int> activeNotes[8]) {
        if (target == GestureTarget::None || axisValue < 0.0f) return;

        juce::String paramName;
        switch (target) {
        case GestureTarget::Pitch:       paramName = "pitch"; break;
        case GestureTarget::NoteTrigger: paramName = "note"; break;
        case GestureTarget::Volume:      paramName = "volume"; liveVolume.store(axisValue); break;
        case GestureTarget::Cutoff:      paramName = "cutoff"; liveCutoff.store(axisValue); break;
        case GestureTarget::Resonance:   paramName = "res"; liveResonance.store(axisValue); break;
        case GestureTarget::Vibrato:     paramName = "vib"; liveVibrato.store(axisValue); break;
        case GestureTarget::Pan:         paramName = "pan"; livePan.store(axisValue); break;
        case GestureTarget::Reverb:      paramName = "reverb"; liveReverb.store(axisValue); break;
        case GestureTarget::Attack:      paramName = "attack"; liveAttack.store(axisValue); break;
        case GestureTarget::Release:     paramName = "release"; liveRelease.store(axisValue); break;
        case GestureTarget::Chorus:      paramName = "chorus"; liveChorus.store(axisValue); break;
        case GestureTarget::Waveform:    paramName = "waveform"; liveWaveform.store(axisValue); break;
        case GestureTarget::Delay:       paramName = "delay"; liveDelay.store(axisValue); break;
        case GestureTarget::Distortion:  paramName = "dist"; liveDistortion.store(axisValue); break;
        default:                         paramName = "param"; break;
        }

        juce::String address = "/" + handPrefix + "/" + paramName;

        // Process Pitch Quantisation 
        if (target == GestureTarget::Pitch) {
            float minNote, maxNote;
            if (mode == MusicalRangeMode::OctaveRange) {
                minNote = (float)startNote + rootNote;
                maxNote = minNote + (octaveRange * 12.0f);
            }
            else {
                minNote = (float)startNote;
                maxNote = (float)endNote;
                if (minNote > maxNote) std::swap(minNote, maxNote);
            }

            float exactNote = juce::jmap(axisValue, 0.0f, 1.0f, minNote, maxNote);
            juce::OSCMessage msg(address);

            if (scaleType == 12) { // 12 = Unquantised Mode
                msg.addFloat32(exactNote);
                activeNotes[0].store((int)exactNote);
            }
            else {
                int targetNote = quantiser.getQuantisedNote(exactNote / 127.0f, rootNote, scaleType);
                msg.addFloat32((float)targetNote);
                activeNotes[0].store(targetNote);
            }
            sender.send(msg);
        }
        else {
            if (target == GestureTarget::NoteTrigger) {
                if (handPrefix == "left") lastLeftMuteSent = axisValue;
                else lastRightMuteSent = axisValue;
            }
            else if (target == GestureTarget::Volume) {
                if (handPrefix == "left") lastLeftVolSent = axisValue;
                else lastRightVolSent = axisValue;
            }

            juce::OSCMessage msg(address);
            msg.addFloat32(axisValue);
            sender.send(msg);
        }
    }

    void panicLeft() {
        juce::OSCMessage msg("/left/note");
        msg.addFloat32(0.0f);
        sender.send(msg);
        lastLeftMuteSent = 0.0f;
    }

    void panicRight() {
        juce::OSCMessage msg("/right/note");
        msg.addFloat32(0.0f);
        sender.send(msg);
        lastRightMuteSent = 0.0f;
    }

private:
    ScaleQuantiser quantiser;

    // State trackers
    float lastLeftMuteSent = -1.0f;
    float lastRightMuteSent = -1.0f;
    float lastLeftVolSent = -1.0f;
    float lastRightVolSent = -1.0f;

    float normalizeAxis(float rawValue, float minBound, float maxBound, float sensitivityMultiplier) {
        float mappedValue = juce::jmap(rawValue, minBound, maxBound, 0.0f, 1.0f);
        float centerOffset = 0.5f;
        mappedValue = centerOffset + ((mappedValue - centerOffset) * sensitivityMultiplier);
        return juce::jlimit(0.0f, 1.0f, mappedValue);
    }

    void processFingers(const HandData& hand, juce::String handPrefix, float minY, float maxY, int rootNote, int scaleType, int octaveRange,
        GestureTarget tThumb, GestureTarget tIndex, GestureTarget tMiddle, GestureTarget tRing, GestureTarget tPinky, MusicalRangeMode rangeMode, int startNote, int endNote, std::atomic<int> activeNotes[8]) {

        auto getFingerVal = [&](int fingerIndex) {
            return juce::jlimit(0.0f, 1.0f, juce::jmap(hand.fingers[fingerIndex].tipY, minY, maxY, 0.0f, 1.0f));
            };

        routeMessage(tThumb, getFingerVal(0), handPrefix, rootNote, scaleType, octaveRange, rangeMode, startNote, endNote, activeNotes);
        routeMessage(tIndex, getFingerVal(1), handPrefix, rootNote, scaleType, octaveRange, rangeMode, startNote, endNote, activeNotes);
        routeMessage(tMiddle, getFingerVal(2), handPrefix, rootNote, scaleType, octaveRange, rangeMode, startNote, endNote, activeNotes);
        routeMessage(tRing, getFingerVal(3), handPrefix, rootNote, scaleType, octaveRange, rangeMode, startNote, endNote, activeNotes);
        routeMessage(tPinky, getFingerVal(4), handPrefix, rootNote, scaleType, octaveRange, rangeMode, startNote, endNote, activeNotes);
    }
};