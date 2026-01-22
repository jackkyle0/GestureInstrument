#pragma once
#include <JuceHeader.h>
#include "../MIDI/GestureTarget.h"
#include "../Helpers/HandData.h"

class OscManager {
public:
    void connect(juce::String ip, int port) { sender.connect(ip, port); }

    void routeMessage(float value, GestureTarget target) {
        if (target == GestureTarget::None) return;

        juce::String address;
        switch (target) {
        case GestureTarget::Volume:     address = "/gesture/volume"; break;
        case GestureTarget::Pitch:      address = "/gesture/pitch"; break;
        case GestureTarget::Modulation: address = "/gesture/mod"; break;
        case GestureTarget::Expression: address = "/gesture/expr"; break;
        case GestureTarget::Cutoff:     address = "/gesture/cutoff"; break;
        case GestureTarget::Resonance:  address = "/gesture/res"; break;
        default: return;
        }
        sender.send(address, value);
    }

    // FIXED: Pass targets directly (No "Processor" dependency needed)
    void processHandData(const HandData& left, const HandData& right,
        float sensitivity, float minH, float maxH,
        GestureTarget leftX, GestureTarget leftY, GestureTarget leftZ, GestureTarget leftRoll,
        GestureTarget rightX, GestureTarget rightY, GestureTarget rightZ, GestureTarget rightRoll)
    {
        // 1. Process Left Hand
        if (left.isPresent) {
            routeMessage(calculateX(left, sensitivity), leftX);
            routeMessage(calculateY(left, minH, maxH), leftY);
            routeMessage(calculateZ(left), leftZ);
            // routeMessage(calculateRoll(left),        leftRoll); // If you implement roll later
        }

        // 2. Process Right Hand
        if (right.isPresent) {
            routeMessage(calculateX(right, sensitivity), rightX);
            routeMessage(calculateY(right, minH, maxH), rightY);
            routeMessage(calculateZ(right), rightZ);
        }
    }

private:
    juce::OSCSender sender;

    float calculateX(const HandData& h, float sens) {
        float range = 200.0f / sens;
        return juce::jlimit(0.0f, 1.0f, juce::jmap(h.currentHandPositionX, -range, range, 0.0f, 1.0f));
    }

    float calculateY(const HandData& h, float min, float max) {
        return juce::jlimit(0.0f, 1.0f, juce::jmap(h.currentHandPositionY, min, max, 0.0f, 1.0f));
    }

    float calculateZ(const HandData& h) {
        return juce::jlimit(0.0f, 1.0f, juce::jmap(h.currentHandPositionZ, -100.0f, 100.0f, 0.0f, 1.0f));
    }
};