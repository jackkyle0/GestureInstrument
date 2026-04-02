#pragma once

#include <JuceHeader.h>
#include "../Helpers/HandData.h"
#include "../MIDI/GestureTarget.h"
#include "../Helpers/ScaleQuantiser.h" 
#include "../Helpers/AdaptiveEngine.h"
#include <functional>

class OscManager {
public:
    OscManager() {}
    ~OscManager() {}

    std::function<void(float, float)> onStyleChanged;
    float leftAIStyle = 0.0f;
    float rightAIStyle = 0.0f;

    juce::OSCSender sender;

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


    void sendEnvelopeData(float envelopeShape) {
        juce::OSCMessage lAtt("/left/attack"); lAtt.addFloat32(envelopeShape); sender.send(lAtt);
        juce::OSCMessage rAtt("/right/attack"); rAtt.addFloat32(envelopeShape); sender.send(rAtt);

        juce::OSCMessage lRel("/left/release"); lRel.addFloat32(envelopeShape); sender.send(lRel);
        juce::OSCMessage rRel("/right/release"); rRel.addFloat32(envelopeShape); sender.send(rRel);
    }

    void sendGlobalWaveform(float waveValue) {
        juce::OSCMessage msg("/global/waveform");
        msg.addFloat32(waveValue);
        sender.send(msg);
    }

    void updateAI(const HandData& leftHand, const HandData& rightHand) {
        float current_l_speed = 0.0f;
        float current_r_speed = 0.0f;

        if (leftHand.isPresent && prevLeftX != 0.0f) {
            current_l_speed = std::sqrt(std::pow(leftHand.currentHandPositionX - prevLeftX, 2) +
                std::pow(leftHand.currentHandPositionY - prevLeftY, 2) +
                std::pow(leftHand.currentHandPositionZ - prevLeftZ, 2));
        }

        if (rightHand.isPresent && prevRightX != 0.0f) {
            current_r_speed = std::sqrt(std::pow(rightHand.currentHandPositionX - prevRightX, 2) +
                std::pow(rightHand.currentHandPositionY - prevRightY, 2) +
                std::pow(rightHand.currentHandPositionZ - prevRightZ, 2));
        }

        if (leftHand.isPresent) {
            prevLeftX = leftHand.currentHandPositionX; prevLeftY = leftHand.currentHandPositionY; prevLeftZ = leftHand.currentHandPositionZ;
        }
        if (rightHand.isPresent) {
            prevRightX = rightHand.currentHandPositionX; prevRightY = rightHand.currentHandPositionY; prevRightZ = rightHand.currentHandPositionZ;
        }

        float smoothingFactor = 0.8f;
        smoothedLeftSpeed = (smoothingFactor * smoothedLeftSpeed) + ((1.0f - smoothingFactor) * current_l_speed);
        smoothedRightSpeed = (smoothingFactor * smoothedRightSpeed) + ((1.0f - smoothingFactor) * current_r_speed);

        float current_l_jitter = std::abs(smoothedLeftSpeed - lastLeftSpeed);
        float current_r_jitter = std::abs(smoothedRightSpeed - lastRightSpeed);

        lastLeftSpeed = smoothedLeftSpeed;
        lastRightSpeed = smoothedRightSpeed;

        if (smoothedLeftSpeed > 0.5f || smoothedRightSpeed > 0.5f) {
            float aiMultiplier = 5.0f;

            float scaledLeftSpeed = smoothedLeftSpeed * aiMultiplier;
            float scaledLeftJitter = current_l_jitter * aiMultiplier;
            float scaledRightSpeed = smoothedRightSpeed * aiMultiplier;
            float scaledRightJitter = current_r_jitter * aiMultiplier;

            float rawLeft = aiEngine.predictStyle(scaledLeftSpeed, scaledLeftJitter, scaledLeftSpeed, scaledLeftJitter);
            leftStyleHistory.push_back(rawLeft);
            if (leftStyleHistory.size() > smoothWindow) leftStyleHistory.erase(leftStyleHistory.begin());
            float sumLeft = 0.0f;
            for (float s : leftStyleHistory) sumLeft += s;
            leftAIStyle = sumLeft / (float)leftStyleHistory.size();

            float rawRight = aiEngine.predictStyle(scaledRightSpeed, scaledRightJitter, scaledRightSpeed, scaledRightJitter);
            rightStyleHistory.push_back(rawRight);
            if (rightStyleHistory.size() > smoothWindow) rightStyleHistory.erase(rightStyleHistory.begin());
            float sumRight = 0.0f;
            for (float s : rightStyleHistory) sumRight += s;
            rightAIStyle = sumRight / (float)rightStyleHistory.size();

            juce::OSCMessage lMsg("/ai/left/style"); lMsg.addFloat32(leftAIStyle); sender.send(lMsg);
            juce::OSCMessage rMsg("/ai/right/style"); rMsg.addFloat32(rightAIStyle); sender.send(rMsg);

            if (onStyleChanged) onStyleChanged(leftAIStyle, rightAIStyle);
        }
    }

    void connectSender(const juce::String& targetIP, int targetPort) {
        sender.connect(targetIP, targetPort);
    }

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

    void processHandData(
        const HandData& leftHand, const HandData& rightHand,
        float sensitivity, float minX, float maxX, float minY, float maxY, float minZ, float maxZ,
        GestureTarget leftXTarget, GestureTarget leftYTarget, GestureTarget leftZTarget, GestureTarget leftRollTarget,
        GestureTarget leftGrabTarget, GestureTarget leftPinchTarget,
        GestureTarget lThumb, GestureTarget lIndex, GestureTarget lMiddle, GestureTarget lRing, GestureTarget lPinky,
        GestureTarget rightXTarget, GestureTarget rightYTarget, GestureTarget rightZTarget, GestureTarget rightRollTarget,
        GestureTarget rightGrabTarget, GestureTarget rightPinchTarget,
        GestureTarget rThumb, GestureTarget rIndex, GestureTarget rMiddle, GestureTarget rRing, GestureTarget rPinky,
        int rootNote, int scaleType, int octaveRange)
    {
        auto normalize = [](float rawValue, float minBound, float maxBound, float sensitivityMultiplier) {
            float mappedValue = juce::jmap(rawValue, minBound, maxBound, 0.0f, 1.0f);
            float centerOffset = 0.5f;
            mappedValue = centerOffset + ((mappedValue - centerOffset) * sensitivityMultiplier);
            return juce::jlimit(0.0f, 1.0f, mappedValue);
            };

        float leftX = -1.0f, leftY = -1.0f, leftZ = -1.0f;
        if (leftHand.isPresent) {
            leftX = normalize(leftHand.currentHandPositionX, minX, maxX, sensitivity);
            leftY = normalize(leftHand.currentHandPositionY, minY, maxY, sensitivity);
            leftZ = 1.0f - normalize(leftHand.currentHandPositionZ, minZ, maxZ, sensitivity);
            float leftRoll = normalize(leftHand.currentWristRotation, 0.0f, 1.0f, sensitivity);
            routeMessage(leftRollTarget, 1.0f - leftRoll, "left", rootNote, scaleType, octaveRange);

            routeMessage(leftXTarget, leftX, "left", rootNote, scaleType, octaveRange);
            routeMessage(leftYTarget, leftY, "left", rootNote, scaleType, octaveRange);
            routeMessage(leftZTarget, leftZ, "left", rootNote, scaleType, octaveRange);

            routeMessage(leftGrabTarget, leftHand.grabStrength, "left", rootNote, scaleType, octaveRange);
            routeMessage(leftPinchTarget, leftHand.pinchStrength, "left", rootNote, scaleType, octaveRange);
            processFingers(leftHand, "left", minY, maxY, rootNote, scaleType, octaveRange, lThumb, lIndex, lMiddle, lRing, lPinky);
        }

        float rightX = -1.0f, rightY = -1.0f, rightZ = -1.0f;
        if (rightHand.isPresent) {
            rightX = normalize(rightHand.currentHandPositionX, minX, maxX, sensitivity);
            rightY = normalize(rightHand.currentHandPositionY, minY, maxY, sensitivity);
            rightZ = 1.0f - normalize(rightHand.currentHandPositionZ, minZ, maxZ, sensitivity);
            float rightRoll = normalize(rightHand.currentWristRotation, 0.0f, 1.0f, sensitivity);
            routeMessage(rightRollTarget, 1.0f - rightRoll, "right", rootNote, scaleType, octaveRange);

            routeMessage(rightXTarget, rightX, "right", rootNote, scaleType, octaveRange);
            routeMessage(rightYTarget, rightY, "right", rootNote, scaleType, octaveRange);
            routeMessage(rightZTarget, rightZ, "right", rootNote, scaleType, octaveRange);

            routeMessage(rightGrabTarget, rightHand.grabStrength, "right", rootNote, scaleType, octaveRange);
            routeMessage(rightPinchTarget, rightHand.pinchStrength, "right", rootNote, scaleType, octaveRange);
            processFingers(rightHand, "right", minY, maxY, rootNote, scaleType, octaveRange, rThumb, rIndex, rMiddle, rRing, rPinky);
        }
    }

    void routeMessage(GestureTarget target, float axisValue, juce::String handPrefix, int rootNote, int scaleType, int octaveRange) {
        if (target == GestureTarget::None || axisValue < 0.0f) return;

        if (target == GestureTarget::Volume) liveVolume.store(axisValue);
        else if (target == GestureTarget::Pan) livePan.store(axisValue);
        else if (target == GestureTarget::Cutoff) liveCutoff.store(axisValue);
        else if (target == GestureTarget::Resonance) liveResonance.store(axisValue);
        else if (target == GestureTarget::Attack) liveAttack.store(axisValue);
        else if (target == GestureTarget::Release) liveRelease.store(axisValue);
        else if (target == GestureTarget::Reverb) liveReverb.store(axisValue);
        else if (target == GestureTarget::Chorus) liveChorus.store(axisValue);
        else if (target == GestureTarget::Vibrato) liveVibrato.store(axisValue);
        else if (target == GestureTarget::Waveform) liveWaveform.store(axisValue);
        else if (target == GestureTarget::Delay) liveDelay.store(axisValue);
        else if (target == GestureTarget::Distortion) liveDistortion.store(axisValue);
        else if (target == GestureTarget::Sustain) liveSustain.store(axisValue);

        juce::String paramName;
        switch (target) {
        case GestureTarget::Pitch:       paramName = "pitch"; break;
        case GestureTarget::NoteTrigger: paramName = "mute"; break;
        case GestureTarget::Volume:      paramName = "volume"; break;
        case GestureTarget::Cutoff:      paramName = "cutoff"; break;
        case GestureTarget::Resonance:   paramName = "res"; break;
        case GestureTarget::Vibrato:     paramName = "vib"; break;
        case GestureTarget::Pan:         paramName = "pan"; break;
        case GestureTarget::Reverb:      paramName = "reverb"; break;
        case GestureTarget::Attack:      paramName = "attack"; break;
        case GestureTarget::Release:     paramName = "release"; break;
        case GestureTarget::Chorus:      paramName = "chorus"; break;
        case GestureTarget::Sustain:     paramName = "sustain"; break;
        case GestureTarget::Waveform:    paramName = "waveform"; break;
        case GestureTarget::Delay:       paramName = "delay"; break;
        case GestureTarget::Distortion:  paramName = "dist"; break;;
        default:                         paramName = "param"; break;
        }

        juce::String address = "/" + handPrefix + "/" + paramName;

        if (target == GestureTarget::Pitch) {
            float minNote = 48.0f;
            float maxNote = 48.0f + (octaveRange * 12.0f);
            float exactNote = juce::jmap(axisValue, 0.0f, 1.0f, minNote, maxNote);
            int targetNote = quantiser.getQuantisedNote(exactNote / 127.0f, rootNote, scaleType);

            juce::OSCMessage msg(address);
            msg.addFloat32((float)targetNote);
            sender.send(msg);
        }
        else {
            juce::OSCMessage msg(address);
            msg.addFloat32(axisValue);
            sender.send(msg);
        }
    }

private:
    ScaleQuantiser quantiser;
    AdaptiveEngine aiEngine;

    float prevLeftX = 0.0f, prevLeftY = 0.0f, prevLeftZ = 0.0f;
    float prevRightX = 0.0f, prevRightY = 0.0f, prevRightZ = 0.0f;
    float lastLeftSpeed = 0.0f;
    float lastRightSpeed = 0.0f;
    int frameCount = 0;

    std::vector<float> leftStyleHistory;
    std::vector<float> rightStyleHistory;
    int smoothWindow = 6;
    float smoothedLeftSpeed = 0.0f;
    float smoothedRightSpeed = 0.0f;

    void processFingers(const HandData& hand, juce::String handPrefix, float minY, float maxY, int rootNote, int scaleType, int octaveRange,
        GestureTarget tThumb, GestureTarget tIndex, GestureTarget tMiddle, GestureTarget tRing, GestureTarget tPinky) {
        auto getFingerVal = [&](int fingerIndex) {
            return juce::jlimit(0.0f, 1.0f, juce::jmap(hand.fingers[fingerIndex].tipY, minY, maxY, 0.0f, 1.0f));
            };

        routeMessage(tThumb, getFingerVal(0), handPrefix, rootNote, scaleType, octaveRange);
        routeMessage(tIndex, getFingerVal(1), handPrefix, rootNote, scaleType, octaveRange);
        routeMessage(tMiddle, getFingerVal(2), handPrefix, rootNote, scaleType, octaveRange);
        routeMessage(tRing, getFingerVal(3), handPrefix, rootNote, scaleType, octaveRange);
        routeMessage(tPinky, getFingerVal(4), handPrefix, rootNote, scaleType, octaveRange);
    }
};