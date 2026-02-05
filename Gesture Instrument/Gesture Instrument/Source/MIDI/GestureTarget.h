#pragma once

enum class GestureTarget {
    None,

    // Essentials
    Volume,         // CC 7
    Pitch,          // Note On + Pitch Bend
    NoteTrigger,    // Note On/Off Only 

    // Modulation
    Modulation,     // CC 1
    Expression,     // CC 11
    Breath,         // CC 2

    // Shaping
    Cutoff,         // CC 74 (Brightness/Timbre)
    Resonance,      // CC 71
    Attack,         // CC 73
    Release,        // CC 72
    Vibrato,        // CC 76

    // Spatial
    Pan,            // CC 10
    Reverb,         // CC 91
    Chorus,         // CC 93

    // Switches
    Sustain,        // CC 64 (On/Off)
    Portamento,     // CC 65 (On/Off)

    // Custom Macros
    Macro1,         // CC 20
    Macro2,         // CC 21
    Macro3,         // CC 22
    Macro4          // CC 23
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
    case GestureTarget::Macro1: return "Macro 1";
    case GestureTarget::Macro2: return "Macro 2";
    case GestureTarget::Macro3: return "Macro 3";
    case GestureTarget::Macro4: return "Macro 4";
    default: return "None";
    }
}