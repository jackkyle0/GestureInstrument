#pragma once
#include <JuceHeader.h>

enum class GestureTarget
{
    None = 0,
    Volume,     // MIDI CC 7
    Pitch,      // MIDI Note
    Modulation, // MIDI CC 1
    Expression, // MIDI CC 11
    Cutoff,     // MIDI CC 74
    Resonance   // MIDI CC 71
};

static juce::String getTargetName(GestureTarget t) {
    switch (t) {
    case GestureTarget::Volume:     return "Volume";
    case GestureTarget::Pitch:      return "Pitch";
    case GestureTarget::Modulation: return "Modulation";
    case GestureTarget::Expression: return "Expression";
    case GestureTarget::Cutoff:     return "Cutoff";
    case GestureTarget::Resonance:  return "Resonance";
    default:                        return "None";
    }
}