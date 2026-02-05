#pragma once
#include <JuceHeader.h>
#include "../Helpers/HandData.h"
#include "../MIDI/GestureTarget.h"
#include "../Helpers/ScaleQuantiser.h" 

class OscManager
{
public:
    OscManager() {}
    ~OscManager() {}

    void connect(const juce::String& targetIP, int targetPort) {
        sender.connect(targetIP, targetPort);
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

        float leftGrab = left.isPresent ? left.grabStrength : 0.0f;
        float leftPinch = left.isPresent ? left.pinchStrength : 0.0f;
        float rightGrab = right.isPresent ? right.grabStrength : 0.0f;
        float rightPinch = right.isPresent ? right.pinchStrength : 0.0f;

        // Left hand
        if (left.isPresent) {
            routeMessage(leftXTarget, leftX, "left", rootNote, scaleType, octaveRange);
            routeMessage(leftYTarget, leftY, "left", rootNote, scaleType, octaveRange);
            routeMessage(leftGrabTarget, leftGrab, "left", rootNote, scaleType, octaveRange);
            routeMessage(leftPinchTarget, leftPinch, "left", rootNote, scaleType, octaveRange);

            processFingers(left, "left", minH, maxH, rootNote, scaleType, octaveRange, lThumb, lIndex, lMiddle, lRing, lPinky);
        }

        // Right hand
        if (right.isPresent) {
            routeMessage(rightXTarget, rightX, "right", rootNote, scaleType, octaveRange);
            routeMessage(rightYTarget, rightY, "right", rootNote, scaleType, octaveRange);
            routeMessage(rightGrabTarget, rightGrab, "right", rootNote, scaleType, octaveRange);
            routeMessage(rightPinchTarget, rightPinch, "right", rootNote, scaleType, octaveRange);

            processFingers(right, "right", minH, maxH, rootNote, scaleType, octaveRange, rThumb, rIndex, rMiddle, rRing, rPinky);
        }
    }

private:
    juce::OSCSender sender;
    ScaleQuantiser quantiser;

    void routeMessage(GestureTarget target, float val, juce::String handPrefix, int root, int scale, int octaveRange) {
        if (target == GestureTarget::None) return;

        juce::String paramName;

        switch (target) {
        case GestureTarget::Volume:     paramName = "volume"; break;
        case GestureTarget::Pitch:      paramName = "pitch"; break;
        case GestureTarget::NoteTrigger:paramName = "note"; break;
        case GestureTarget::Modulation: paramName = "mod"; break;
        case GestureTarget::Expression: paramName = "expr"; break;
        case GestureTarget::Breath:     paramName = "breath"; break;

        case GestureTarget::Cutoff:     paramName = "cutoff"; break;
        case GestureTarget::Resonance:  paramName = "res"; break;
        case GestureTarget::Attack:     paramName = "attack"; break;
        case GestureTarget::Release:    paramName = "release"; break;
        case GestureTarget::Vibrato:    paramName = "vib"; break;

        case GestureTarget::Pan:        paramName = "pan"; break;
        case GestureTarget::Reverb:     paramName = "reverb"; break;
        case GestureTarget::Chorus:     paramName = "chorus"; break;

        case GestureTarget::Sustain:    paramName = "sustain"; break;
        case GestureTarget::Portamento: paramName = "portamento"; break;

        case GestureTarget::Macro1:     paramName = "macro1"; break;
        case GestureTarget::Macro2:     paramName = "macro2"; break;
        case GestureTarget::Macro3:     paramName = "macro3"; break;
        case GestureTarget::Macro4:     paramName = "macro4"; break;

        default: return;
        }

        juce::String address = "/" + handPrefix + "/" + paramName;

        if (target == GestureTarget::Pitch) {
            float rangeInSemitones = (float)(octaveRange * 12);
            float minNote = 48.0f;
            float maxNote = 48.0f + rangeInSemitones;

            float noteInRange = juce::jmap(val, 0.0f, 1.0f, minNote, maxNote);
            float quantiserInput = noteInRange / 127.0f;

            int noteNumber = quantiser.getQuantisedNote(quantiserInput, root, scale);

            sender.send(address, (float)noteNumber);
        }
        else {
            sender.send(address, val);
        }
    }

    void processFingers(const HandData& h, juce::String handPrefix, float minH, float maxH, int root, int scale, int octaveRange,
        GestureTarget tThumb, GestureTarget tIndex, GestureTarget tMiddle, GestureTarget tRing, GestureTarget tPinky)
    {
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