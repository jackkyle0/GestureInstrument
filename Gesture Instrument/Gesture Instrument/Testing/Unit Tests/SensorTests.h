#pragma once
#include <JuceHeader.h>

class SensorMathTests : public juce::UnitTest {
public:
    SensorMathTests() : juce::UnitTest("Sensor Tests")
    {
    }

    void runTest() override {
        beginTest("Test");

        float testVal = 1.0f;

        expect(testVal == 1.0f, "Fail");
    }
};

static SensorMathTests sensorMathTestsInstance;