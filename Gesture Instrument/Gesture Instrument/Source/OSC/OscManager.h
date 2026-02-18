#pragma once
#include <JuceHeader.h>
#include "../Helpers/HandData.h"
#include "../MIDI/GestureTarget.h"
#include "../Helpers/ScaleQuantiser.h" 

class OscManager : public juce::OSCReceiver::ListenerWithOSCAddress<juce::OSCReceiver::RealtimeCallback>
{
public:
    OscManager() {}

    ~OscManager() {
        receiver.removeListener(this);
        receiver.disconnect();
    }

    std::atomic<float> currentAIStyle{ 0.0f };
    std::atomic<float> currentAIConfidence{ 0.0f }; 

    void connectToReceiver(int port) {
        if (receiver.connect(port)) {
            receiver.addListener(this, "/ai/style");
            receiver.addListener(this, "/ai/confidence"); 
        }
    }

    void oscMessageReceived(const juce::OSCMessage& message) override {
        if (message.size() == 1 && message[0].isFloat32()) {
            auto address = message.getAddressPattern().toString();

            if (address == "/ai/style") {
                currentAIStyle.store(message[0].getFloat32());
            }
            else if (address == "/ai/confidence") {
                currentAIConfidence.store(message[0].getFloat32());
            }
        }
    }

    void connectSender(const juce::String& targetIP, int targetPort) {
        sender.connect(targetIP, targetPort);
    }

    void sendRawData(const HandData& left, const HandData& right) {
        juce::OSCMessage msg("/gesture/raw");
        auto addH = [&](const HandData& h) {
            if (h.isPresent) {
                msg.addFloat32(h.currentHandPositionX);
                msg.addFloat32(h.currentHandPositionY);
                msg.addFloat32(h.currentHandPositionZ);
                msg.addFloat32(h.grabStrength);
            }
            else {
                msg.addFloat32(0.0f); msg.addFloat32(0.0f); msg.addFloat32(0.0f); msg.addFloat32(0.0f);
            }
            };
        addH(left);
        addH(right);
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
        // Scale Info 
        int rootNote, int scaleType, int octaveRange)
    {
        // Calculate Raw Values (0.0 - 1.0 High Res)
        float leftX = calculateX(left, sensitivity);
        float leftY = calculateY(left, minH, maxH);
        float rightX = calculateX(right, sensitivity);
        float rightY = calculateY(right, minH, maxH);

        // Left hand
        if (left.isPresent) {
            routeMessage(leftXTarget, leftX, "left", rootNote, scaleType, octaveRange);
            routeMessage(leftYTarget, leftY, "left", rootNote, scaleType, octaveRange);
            routeMessage(leftGrabTarget, left.grabStrength, "left", rootNote, scaleType, octaveRange);
            routeMessage(leftPinchTarget, left.pinchStrength, "left", rootNote, scaleType, octaveRange);
            processFingers(left, "left", minH, maxH, rootNote, scaleType, octaveRange, lThumb, lIndex, lMiddle, lRing, lPinky);
        }

        // Right hand
        if (right.isPresent) {
            routeMessage(rightXTarget, rightX, "right", rootNote, scaleType, octaveRange);
            routeMessage(rightYTarget, rightY, "right", rootNote, scaleType, octaveRange);
            routeMessage(rightGrabTarget, right.grabStrength, "right", rootNote, scaleType, octaveRange);
            routeMessage(rightPinchTarget, right.pinchStrength, "right", rootNote, scaleType, octaveRange);
            processFingers(right, "right", minH, maxH, rootNote, scaleType, octaveRange, rThumb, rIndex, rMiddle, rRing, rPinky);
        }
    }

private:
    juce::OSCSender sender;
    juce::OSCReceiver receiver;
    ScaleQuantiser quantiser;

    void routeMessage(GestureTarget target, float val, juce::String handPrefix, int root, int scale, int octaveRange) {
        if (target == GestureTarget::None) return;
        juce::String paramName;
        switch (target) {
        case GestureTarget::Pitch:      paramName = "pitch"; break;
        case GestureTarget::NoteTrigger:paramName = "note"; break;
        case GestureTarget::Volume:     paramName = "volume"; break;
        case GestureTarget::Modulation: paramName = "mod"; break;
        case GestureTarget::Cutoff:     paramName = "cutoff"; break;
        case GestureTarget::Resonance:  paramName = "res"; break;
        case GestureTarget::Vibrato:    paramName = "vib"; break;
        case GestureTarget::Pan:        paramName = "pan"; break;
        case GestureTarget::Expression: paramName = "expr"; break;
        case GestureTarget::Reverb:     paramName = "reverb"; break;
        case GestureTarget::Attack:     paramName = "attack"; break;
        case GestureTarget::Release:    paramName = "release"; break;
        case GestureTarget::Chorus:     paramName = "chorus"; break;
        case GestureTarget::Sustain:    paramName = "sustain"; break;
        case GestureTarget::Portamento: paramName = "portamento"; break;
        default:                        paramName = "param"; break;
        }

        juce::String address = "/" + handPrefix + "/" + paramName;

        if (target == GestureTarget::Pitch) {
            float rangeInSemitones = (float)(octaveRange * 12);
            float noteInRange = juce::jmap(val, 0.0f, 1.0f, 48.0f, 48.0f + rangeInSemitones);
            int noteNumber = quantiser.getQuantisedNote(noteInRange / 127.0f, root, scale);
            sender.send(address, (float)noteNumber);
        }
        else {
            sender.send(address, val);
        }
    }

    void processFingers(const HandData& h, juce::String handPrefix, float minH, float maxH, int root, int scale, int octaveRange,
        GestureTarget tThumb, GestureTarget tIndex, GestureTarget tMiddle, GestureTarget tRing, GestureTarget tPinky) {
        auto getF = [&](int i) { return juce::jlimit(0.0f, 1.0f, juce::jmap(h.fingers[i].tipY, minH, maxH, 0.0f, 1.0f)); };
        routeMessage(tThumb, getF(0), handPrefix, root, scale, octaveRange);
        routeMessage(tIndex, getF(1), handPrefix, root, scale, octaveRange);
        routeMessage(tMiddle, getF(2), handPrefix, root, scale, octaveRange);
        routeMessage(tRing, getF(3), handPrefix, root, scale, octaveRange);
        routeMessage(tPinky, getF(4), handPrefix, root, scale, octaveRange);
    }

    float calculateX(const HandData& h, float sens) {
        float range = 200.0f / sens;
        return juce::jlimit(0.0f, 1.0f, juce::jmap(h.currentHandPositionX, -range, range, 0.0f, 1.0f));
    }

    float calculateY(const HandData& h, float min, float max) {
        return juce::jlimit(0.0f, 1.0f, juce::jmap(h.currentHandPositionY, min, max, 0.0f, 1.0f));
    }
};