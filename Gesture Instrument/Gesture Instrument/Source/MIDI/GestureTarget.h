#pragma once

#include <JuceHeader.h>

enum class GestureTarget {
    None = 1,
    Volume = 2,
    Pitch = 3,
    NoteTrigger = 4,
    Modulation = 5,
    Expression = 6,
    Breath = 7,
    Cutoff = 8,
    Resonance = 9,
    Attack = 10,
    Release = 11,
    Vibrato = 12,
    Pan = 13,
    Reverb = 14,
    Chorus = 15,
    Sustain = 16,
    Portamento = 17,
    Waveform = 18,
    Delay = 19,     
    Distortion = 20  
};

static juce::String getTargetName(GestureTarget t) {
    switch (t) {
    case GestureTarget::Volume: return "Volume";
    case GestureTarget::Pitch: return "Pitch";
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
    case GestureTarget::Waveform: return "Waveform";
    case GestureTarget::Delay: return "Delay"; 
    case GestureTarget::Distortion: return "Distortion";  
    default: return "None";
    }
}