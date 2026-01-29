#pragma once
#include <JuceHeader.h>

enum class GestureTarget
{
    None = 0,
    Volume,     // CC 7
    Pitch,      // Note
    Modulation, // CC 1
    Expression, // CC 11
    Cutoff,     // CC 74
    Resonance   // CC 71
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