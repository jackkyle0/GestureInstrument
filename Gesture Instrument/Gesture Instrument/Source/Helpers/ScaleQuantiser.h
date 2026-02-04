#pragma once
#include <JuceHeader.h>
#include <vector>

class ScaleQuantiser
{
public:
    ScaleQuantiser() {}

    int getQuantisedNote(float value, int rootNote, int scaleType)
    {
        int rawNote = (int)(value * 127.0f);

        std::vector<int> intervals;

        switch (scaleType) {
        case 1: intervals = { 0, 2, 4, 5, 7, 9, 11 }; break;       
        case 2: intervals = { 0, 2, 3, 5, 7, 8, 10 }; break;      
        case 3: intervals = { 0, 2, 4, 7, 9 }; break;              
        default: return rawNote; // Chromatic
        }

        return snapToScale(rawNote, rootNote, intervals);
    }

private:
    int snapToScale(int note, int root, const std::vector<int>& intervals)
    {
        int octave = note / 12;
        int degree = note % 12;

        int closest = -1;
        int minDist = 100;

        for (int interval : intervals) {
            int target = (root + interval) % 12;
            int dist = std::abs(degree - target);

            if (dist > 6) dist = 12 - dist;

            if (dist < minDist) {
                minDist = dist;
                closest = target;
            }
        }

        return (octave * 12) + closest;
    }
};