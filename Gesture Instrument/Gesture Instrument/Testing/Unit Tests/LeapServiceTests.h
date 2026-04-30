#pragma once
#include <JuceHeader.h>
#include "../../Source/Helpers/LeapService.h" 

class LeapServiceTests : public juce::UnitTest {
public:
    LeapServiceTests() : juce::UnitTest("Leap SDK Service and Parsing Tests")
    {
    }

    void runTest() override {
        beginTest("1. Right Hand Data Extraction");
        {
            LeapService service;
            HandData leftHand, rightHand;

            // Build mock leap event
            LEAP_HAND mockHand = {};
            mockHand.type = eLeapHandType_Right; // Specify this is a right hand
            mockHand.palm.position.x = 120.5f;
            mockHand.palm.position.y = 300.0f;
            mockHand.palm.position.z = -50.2f;
            mockHand.grab_strength = 0.45f;
            mockHand.pinch_strength = 0.85f; // Greater than 0.8 to trigger isPinching

            LEAP_TRACKING_EVENT mockEvent = {};
            mockEvent.nHands = 1;
            mockEvent.pHands = &mockHand;

            // Execute
            service.convertLeapEventToHandData(&mockEvent, leftHand, rightHand);

            expect(rightHand.isPresent, "The Right Hand should be marked as present");
            expect(!leftHand.isPresent, "The Left Hand should be marked as absent");

            // Check spatial data
            expectEquals(rightHand.currentHandPositionX, 120.5f, "X coordinate extraction failed.");
            expectEquals(rightHand.currentHandPositionY, 300.0f, "Y coordinate extraction failed.");
            expectEquals(rightHand.currentHandPositionZ, -50.2f, "Z coordinate extraction failed.");

            // Check gestures
            expectEquals(rightHand.grabStrength, 0.45f, "Grab strength extraction failed");
            expect(rightHand.isPinching, "Pinch strength of 0.85 should trigger isPinching = true");
        }

        beginTest("2. Hardware Disconnect");
        {
            LeapService service;
            HandData leftHand, rightHand;

            // both hands visable
            LEAP_HAND dualHands[2] = {};
            dualHands[0].type = eLeapHandType_Left;
            dualHands[1].type = eLeapHandType_Right;

            LEAP_TRACKING_EVENT activeEvent = {};
            activeEvent.nHands = 2;
            activeEvent.pHands = dualHands;

            service.convertLeapEventToHandData(&activeEvent, leftHand, rightHand);
            expect(leftHand.isPresent && rightHand.isPresent, "Setup failed: Both hands should be active.");

            // move hands from sensor
            LEAP_TRACKING_EVENT emptyEvent = {};
            emptyEvent.nHands = 0; 
            emptyEvent.pHands = nullptr;

            service.convertLeapEventToHandData(&emptyEvent, leftHand, rightHand);

            expect(!leftHand.isPresent, "Left hand failed to clear presence on empty frame.");
            expect(!rightHand.isPresent, "Right hand failed to clear presence on empty frame.");
        }
    }
};

static LeapServiceTests leapServiceTestsInstance;