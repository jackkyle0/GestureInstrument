#pragma once

#include <JuceHeader.h>
#include <deque>
#include <numeric>
#include <cmath>

class BiometricSpatialTracker {
public:
    BiometricSpatialTracker(int windowSize, float absMin, float absMax, float minRange)
        : maxHistory(windowSize), absoluteMin(absMin), absoluteMax(absMax), minimumRange(minRange) {}

    void addDataPoint(float position) {
        history.push_back(position);
        if (history.size() > maxHistory) {
            history.pop_front();
        }
    }

    struct SpatialLimits {
        float minBound;
        float maxBound;
        float center;
    };

    SpatialLimits getLearnedBoundaries(float standardDeviationMultiplier = 2.0f) {
        if (history.size() < 60) {
            return { absoluteMin, absoluteMax, (absoluteMin + absoluteMax) / 2.0f };
        }

        float sum = std::accumulate(history.begin(), history.end(), 0.0f);
        float mean = sum / history.size();

        float varianceSum = 0.0f;
        for (float val : history) {
            varianceSum += (val - mean) * (val - mean);
        }
        float variance = varianceSum / history.size();

        float standardDeviation = std::sqrt(variance);

        float effectiveRange = std::max(standardDeviation * standardDeviationMultiplier, minimumRange / 2.0f);

        float minBound = mean - effectiveRange;
        float maxBound = mean + effectiveRange;

        minBound = juce::jlimit(absoluteMin, absoluteMax - minimumRange, minBound);
        maxBound = juce::jlimit(absoluteMin + minimumRange, absoluteMax, maxBound);

        return { minBound, maxBound, mean };
    }

private:
    std::deque<float> history;
    size_t maxHistory;
    float absoluteMin;
    float absoluteMax;
    float minimumRange;
};