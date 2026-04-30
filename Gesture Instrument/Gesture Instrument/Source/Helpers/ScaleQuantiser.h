#pragma once

#include <JuceHeader.h>
#include <vector>
#include <array>

class ScaleQuantiser {
public:
    ScaleQuantiser() {}

    std::vector<int> customIntervals = { 0, 2, 4, 7, 9 };

    int getQuantisedNote(float normalizedPosition, int rootNote, int scaleType) {
        std::array<bool, 7> allAllowed = { true, true, true, true, true, true, true };
        return getQuantisedNote(normalizedPosition, rootNote, scaleType, allAllowed);
    }

    int getQuantisedNote(float normalizedPosition, int rootNote, int scaleType, const std::array<bool, 7>& allowedRoots) {
        normalizedPosition = juce::jlimit(0.0f, 1.0f, normalizedPosition);
        int rawNote = static_cast<int>(normalizedPosition * 127.0f);

        std::vector<int> fullIntervals = getScaleIntervals(scaleType);
        std::vector<int> filteredIntervals;

        if (fullIntervals.size() == 7) {
            for (size_t i = 0; i < fullIntervals.size(); ++i) {
                if (allowedRoots[i]) {
                    filteredIntervals.push_back(fullIntervals[i]);
                }
            }
        }
        else {
            filteredIntervals = fullIntervals;
        }

        if (filteredIntervals.empty()) {
            filteredIntervals = { 0 };
        }

        int finalNote = snapToScale(rawNote, rootNote, filteredIntervals);
        return juce::jlimit(0, 127, finalNote);
    }

    std::vector<int> getScaleIntervals(int scaleType) {
        switch (scaleType) {
        case 1:  return { 0, 2, 4, 5, 7, 9, 11 };        // Major
        case 2:  return { 0, 2, 3, 5, 7, 8, 10 };        // Minor
        case 3:  return { 0, 2, 4, 7, 9 };               // Major Pentatonic 
        case 4:  return { 0, 3, 5, 7, 10 };              // Minor Pentatonic
        case 5:  return { 0, 3, 5, 6, 7, 10 };           // Blues
        case 6:  return { 0, 2, 3, 5, 7, 9, 10 };        // Dorian
        case 7:  return { 0, 2, 4, 5, 7, 9, 10 };        // Mixolydian
        case 8:  return { 0, 2, 4, 6, 7, 9, 11 };        // Lydian
        case 9:  return { 0, 1, 3, 5, 7, 8, 10 };        // Phrygian
        case 10: return { 0, 2, 3, 5, 7, 8, 11 };        // Harmonic Minor
        case 11: return { 0, 1, 3, 5, 6, 8, 10 };        // Locrian
        case 13: return customIntervals;                 // Custom
        case 12:                                         // Unquantised
        default: {                                       // Chromatic (0)
            std::vector<int> chromatic;
            for (int i = 0; i < 12; ++i) chromatic.push_back(i);
            return chromatic;
        }
        }
    }

private:
    int snapToScale(int rawNote, int rootNote, const std::vector<int>& intervals) {
        int closestNote = -1;
        int minDistance = 1000;

        for (int interval : intervals) {
            int targetPitchClass = (rootNote + interval) % 12;
            int diff = targetPitchClass - (rawNote % 12);

            // Wrap safely around the octave boundary to find the true shortest distance
            if (diff > 6) diff -= 12;
            else if (diff < -6) diff += 12;

            int candidateNote = rawNote + diff;
            int currentDistance = std::abs(rawNote - candidateNote);

            if (currentDistance < minDistance) {
                minDistance = currentDistance;
                closestNote = candidateNote;
            }
        }

        return closestNote;
    }
};