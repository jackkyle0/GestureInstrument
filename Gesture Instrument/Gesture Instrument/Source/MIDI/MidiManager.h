#pragma once
#include <JuceHeader.h>
#include "GestureTarget.h"
#include "../Helpers/HandData.h"

class MidiManager {
public:
    void processHandData(juce::MidiBuffer& midiMessages, const HandData& left, const HandData& right, float sensitivity, float minH, float maxH,
        GestureTarget leftXTarget, GestureTarget leftYTarget, GestureTarget rightXTarget, GestureTarget rightYTarget)
    {
        // Calculate values --0.0 to 1.0-- for every axis
        float leftX = calculateX(left, sensitivity);
        float leftY = calculateY(left, minH, maxH);
        float rightX = calculateX(right, sensitivity);
        float rightY = calculateY(right, minH, maxH);

        // check if pitch is assigned
        float pitchValue = -1.0f;

        if (leftXTarget == GestureTarget::Pitch)  pitchValue = leftX;
        if (leftYTarget == GestureTarget::Pitch)  pitchValue = leftY;
        if (rightXTarget == GestureTarget::Pitch) pitchValue = rightX;
        if (rightYTarget == GestureTarget::Pitch) pitchValue = rightY;

        handleNoteLogic(midiMessages, pitchValue);

       // sennd cc value to what is assigned 
        if (left.isPresent) {
            sendCC(midiMessages, leftXTarget, leftX);
            sendCC(midiMessages, leftYTarget, leftY);
        }
        if (right.isPresent) {
            sendCC(midiMessages, rightXTarget, rightX);
            sendCC(midiMessages, rightYTarget, rightY);
        }
    }

private:
    int lastMidiNote = -1;
    bool isNoteOn = false;

    // X axis
    float calculateX(const HandData& h, float sens) {
        if (!h.isPresent) return -1.0f;
        float range = 200.0f / sens;
        return juce::jlimit(0.0f, 1.0f, juce::jmap(h.currentHandPositionX, -range, range, 0.0f, 1.0f));
    }

    // Y axis
    float calculateY(const HandData& h, float min, float max) {
        if (!h.isPresent) return -1.0f;
        return juce::jlimit(0.0f, 1.0f, juce::jmap(h.currentHandPositionY, min, max, 0.0f, 1.0f));
    }

    // pitch related logic
    void handleNoteLogic(juce::MidiBuffer& midi, float val) {
        if (val >= 0.0f) {
            // Map 0-1 to C3(48) - C5(72)
            int note = (int)juce::jmap(val, 0.0f, 1.0f, 48.0f, 72.0f);

            if (note != lastMidiNote || !isNoteOn) {
                if (isNoteOn) midi.addEvent(juce::MidiMessage::noteOff(1, lastMidiNote), 0);
                midi.addEvent(juce::MidiMessage::noteOn(1, note, (juce::uint8)100), 0);
                lastMidiNote = note;
                isNoteOn = true;
            }
        }
        else if (isNoteOn) {
            // Hand removed? Kill note.
            midi.addEvent(juce::MidiMessage::noteOff(1, lastMidiNote), 0);
            isNoteOn = false;
        }
    }

    void sendCC(juce::MidiBuffer& midi, GestureTarget target, float val) {
        if (val < 0.0f) return; // Hand not present

        int ccNumber = -1;
        switch (target) {
        case GestureTarget::Volume:     ccNumber = 7; break;
        case GestureTarget::Modulation: ccNumber = 1; break;
        case GestureTarget::Expression: ccNumber = 11; break;
        case GestureTarget::Cutoff:     ccNumber = 74; break;
        case GestureTarget::Resonance:  ccNumber = 71; break;
        default: return; // none
        }

        // send CC message 
        midi.addEvent(juce::MidiMessage::controllerEvent(1, ccNumber, (int)(val * 127.0f)), 0);
    }
};