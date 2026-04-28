#pragma once
#include <JuceHeader.h>
#include <vector>
#include <array>

class ScaleQuantiser {
public:
    ScaleQuantiser() {}

    std::vector<int> customIntervals = { 0, 2, 4, 7, 9 };

    int getQuantisedNote(float value, int rootNote, int scaleType) {
        std::array<bool, 7> allAllowed = { true, true, true, true, true, true, true };
        return getQuantisedNote(value, rootNote, scaleType, allAllowed);
    }
    int getQuantisedNote(float value, int rootNote, int scaleType, const std::array<bool, 7>& allowedRoots) {

        // --- THE FIX: Clamp the raw input before any math happens! ---
        // This guarantees that even if the Leap Motion sends -5.0f or 10.5f, 
        // it gets locked safely between 0.0f and 1.0f.
        value = juce::jlimit(0.0f, 1.0f, value);

        int rawNote = (int)(value * 127.0f);
        std::vector<int> fullIntervals = getScaleIntervals(scaleType);

        // --- PROGRESSION FILTER ---
        // Strip out any intervals that aren't allowed by the user
        // --- THE FIX: PROGRESSION FILTER ---
        std::vector<int> filteredIntervals;

        // ONLY apply the 7-button root filter if the scale actually has 7 notes!
        if (fullIntervals.size() == 7) {
            for (size_t i = 0; i < fullIntervals.size(); ++i) {
                if (allowedRoots[i]) {
                    filteredIntervals.push_back(fullIntervals[i]);
                }
            }
        }
        else {
            // For Chromatic (12), Pentatonic (5), and Custom...
            // DO NOT chop off notes! Ignore the 7-button filter entirely.
            filteredIntervals = fullIntervals;
        }

        // Safety Fallback: If they unchecked everything, just use the root note
        if (filteredIntervals.empty()) filteredIntervals = { 0 };

        // --- SECONDARY SAFETY CLAMP ---
        // Wrap the final return in a clamp just to be absolutely certain it never exceeds 0-127
        int finalNote = snapToScale(rawNote, rootNote, filteredIntervals);
        return juce::jlimit(0, 127, finalNote);
    }

    // --- NEW: Added getScaleIntervals here so MidiManager can use it! ---
    std::vector<int> getScaleIntervals(int scaleType) {
        switch (scaleType) {
        case 1: return { 0, 2, 4, 5, 7, 9, 11 };       // Major
        case 2: return { 0, 2, 3, 5, 7, 8, 10 };       // Minor
        case 3: return { 0, 2, 4, 7, 9 };              // Major Pentatonic 
        case 4: return { 0, 3, 5, 7, 10 };             // Minor Pentatonic
        case 5: return { 0, 3, 5, 6, 7, 10 };          // Blues
        case 6: return { 0, 2, 3, 5, 7, 9, 10 };       // Dorian
        case 7: return { 0, 2, 4, 5, 7, 9, 10 };       // Mixolydian
        case 8: return { 0, 2, 4, 6, 7, 9, 11 };       // Lydian
        case 9: return { 0, 1, 3, 5, 7, 8, 10 };       // Phrygian
        case 10: return { 0, 2, 3, 5, 7, 8, 11 };      // Harmonic Minor
        case 11: return { 0, 1, 3, 5, 6, 8, 10 };      // Locrian
        case 13: return customIntervals;
        case 12:
        default: {                                     // Chromatic (0)
            std::vector<int> chromatic;
            for (int i = 0; i < 12; ++i) chromatic.push_back(i);
            return chromatic;
        }
        }
    }

private:
    int snapToScale(int note, int rootNote, const std::vector<int>& intervals) {
        int closestNote = -1;
        int minDistance = 1000;

        for (int interval : intervals) {
            // Find the 0-11 pitch class for this interval
            int targetPitchClass = (rootNote + interval) % 12;

            // Find the raw distance from our current note's degree to the target
            int diff = targetPitchClass - (note % 12);

            // Wrap safely around the octave boundary! (This fixes the massive jumping)
            if (diff > 6) diff -= 12;
            else if (diff < -6) diff += 12;

            // Generate the actual candidate MIDI note and check its true distance
            int candidateNote = note + diff;
            int currentDistance = std::abs(note - candidateNote);

            if (currentDistance < minDistance) {
                minDistance = currentDistance;
                closestNote = candidateNote;
            }
        }

        return closestNote;
    }
};