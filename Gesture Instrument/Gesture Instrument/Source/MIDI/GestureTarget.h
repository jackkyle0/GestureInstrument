#pragma once

#include <JuceHeader.h>

enum class GestureTarget {
    None,
    Volume,
    Pitch,
    NoteTrigger,
    Modulation,
    Expression,
    Breath,
    Cutoff,
    Resonance,
    Attack,
    Release,
    Vibrato,
    Pan,
    Reverb,
    Chorus,
    Sustain,
    Portamento
};

static juce::String getTargetName(GestureTarget t) {
    switch (t) {
    case GestureTarget::Volume: return "Volume";
    case GestureTarget::Pitch: return "Pitch (Bend)";
    case GestureTarget::NoteTrigger: return "Note Trigger";
    case GestureTarget::Modulation: return "Mod Wheel";
    case GestureTarget::Expression: return "Expression";
    case GestureTarget::Breath: return "Breath";
    case GestureTarget::Cutoff: return "Cutoff";
    case GestureTarget::Resonance: return "Resonance";
    case GestureTarget::Attack: return "Attack";
    case GestureTarget::Release: return "Release";
    case GestureTarget::Vibrato: return "Vibrato";
    case GestureTarget::Pan: return "Pan";
    case GestureTarget::Reverb: return "Reverb";
    case GestureTarget::Chorus: return "Chorus";
    case GestureTarget::Sustain: return "Sustain";
    case GestureTarget::Portamento: return "Portamento";
    default: return "None";
    }
}