#pragma once
#include <JuceHeader.h>
#include <vector>

class ScaleQuantiser {
public:
    ScaleQuantiser() {}

    int getQuantisedNote(float value, int rootNote, int scaleType) {
        int rawNote = (int)(value * 127.0f);
        std::vector<int> intervals;

        switch (scaleType) {
        case 1: intervals = { 0, 2, 4, 5, 7, 9, 11 }; break;
        case 2: intervals = { 0, 2, 3, 5, 7, 8, 10 }; break;
        case 3: intervals = { 0, 2, 4, 7, 9 }; break;
        default: return rawNote;
        }

        return snapToScale(rawNote, rootNote, intervals);
    }

private:
    int snapToScale(int note, int rootNote, const std::vector<int>& intervals) {
        int octave = note / 12;
        int degree = note % 12;

        int closestDegree = -1;
        int minimumDistance = 100;

        for (int interval : intervals) {
            int targetDegree = (rootNote + interval) % 12;
            int distance = std::abs(degree - targetDegree);

            if (distance > 6) {
                distance = 12 - distance;
            }

            if (distance < minimumDistance) {
                minimumDistance = distance;
                closestDegree = targetDegree;
            }
        }

        return (octave * 12) + closestDegree;
    }
};