#pragma once
#include <JuceHeader.h>
#include "HandData.h" 
#include "GestureTarget.h"


class OscManager
{
public:
    OscManager() {}

    void connect(juce::String ipAddress, int port) {
        if (!sender.connect(ipAddress, port))
            DBG("OSC Connection failed");
        else
            DBG("OSC Connected to " + ipAddress);
    }

    void processHandData(const HandData& leftHand, const HandData& rightHand, float sensitivity, float minHeight, float maxHeight){
        // LEFT HAND
        if (leftHand.isPresent)
        {
            float vol = juce::jmap(leftHand.currentHandPositionY, minHeight, maxHeight, 0.0f, 1.0f);
            vol = juce::jlimit(0.0f, 1.0f, vol);

            if (std::abs(vol - lastVolume) > 0.005f)
            {
                sender.send("/gesture/volume", vol);
                lastVolume = vol;
            }
        }
        else {
            if (lastVolume > 0.0f) {
                sender.send("/gesture/volume", 0.0f);
                lastVolume = 0.0f;
            }
        }

        // RIGHT HAND
        if (rightHand.isPresent) {
            float activeRange = 200.0f / sensitivity;
            float pitch = juce::jmap(rightHand.currentHandPositionX, -activeRange, activeRange, 0.0f, 1.0f);
            pitch = juce::jlimit(0.0f, 1.0f, pitch);

            if (std::abs(pitch - lastPitch) > 0.005f) {
                sender.send("/gesture/pitch", pitch);
                lastPitch = pitch;
            }
        }
    }

private:
    juce::OSCSender sender;
    float lastVolume = -1.0f;
    float lastPitch = -1.0f;
};