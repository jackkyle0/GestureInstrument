#pragma once
#include <JuceHeader.h>
#include <atomic>
#include "../../Source/OSC/OSCManager.h" // Adjust path as needed

class OscManagerTests : public juce::UnitTest, private juce::OSCReceiver::ListenerWithOSCAddress<juce::OSCReceiver::RealtimeCallback> {
public:
    OscManagerTests() : juce::UnitTest("OSC Manager Network and Formatting Tests") {}

    // --- OSC Interception Memory ---
    juce::String lastReceivedAddress;
    float lastReceivedFloat = -999.0f;

    std::atomic<bool> messageReceived{ false };

    void oscMessageReceived(const juce::OSCMessage& message) override {
        lastReceivedAddress = message.getAddressPattern().toString();
        if (message.size() > 0 && message[0].isFloat32()) {
            lastReceivedFloat = message[0].getFloat32();
        }
        messageReceived.store(true);
    }

    void runTest() override {
        // --- Setup the Test Network ---
        int testPort = 9001;
        juce::OSCReceiver testReceiver;
        testReceiver.connect(testPort);

        // Tell the receiver to listen to these specific test addresses
        testReceiver.addListener(this, "/left/pitch");
        testReceiver.addListener(this, "/left/note");
        testReceiver.addListener(this, "/right/pitch");
        testReceiver.addListener(this, "/left/modulation");

        beginTest("1. Address Formatting and Float32 Verification");
        {
            OscManager osc;
            osc.connectSender("127.0.0.1", testPort);

            messageReceived = false;

            // Bypass the quantiser (Theremin mode, type 12)
            osc.routeMessage(GestureTarget::Pitch, 0.5f, "left", 0, 12, 1, MusicalRangeMode::OctaveRange, 60, 72);

            int timeout = 50;
            while (!messageReceived && timeout > 0) { juce::Thread::sleep(10); timeout--; }

            expect(messageReceived, "Failed to receive OSC UDP Packet over the local loopback.");
            expectEquals(lastReceivedAddress, juce::String("/left/pitch"), "OSC Address formatting is incorrect.");
            expect(std::abs(lastReceivedFloat - 66.0f) < 0.001f, "Payload float calculation (Theremin Mode) failed.");
        }

        beginTest("2. Network State Tracking (Spam Prevention)");
        {
            OscManager osc;
            osc.connectSender("127.0.0.1", testPort);
            messageReceived = false;

            osc.panicLeft(); // Sends 0.0f
            int timeout = 50;
            while (!messageReceived && timeout > 0) { juce::Thread::sleep(10); timeout--; }

            expectEquals(lastReceivedAddress, juce::String("/left/note"), "Panic address formatting incorrect.");
            expectEquals(lastReceivedFloat, 0.0f, "Panic payload must be strictly 0.0f.");

            // --- Spam Prevention Test ---
            HandData emptyHand;
            emptyHand.isPresent = true;

            // First call: Should send 1.0f
            messageReceived = false;
            osc.processHandData(emptyHand, emptyHand, 1.0f, 0, 1, 0, 1, 0, 1, GestureTarget::None, GestureTarget::None, GestureTarget::None, GestureTarget::None, GestureTarget::None, GestureTarget::None, GestureTarget::None, GestureTarget::None, GestureTarget::None, GestureTarget::None, GestureTarget::None, GestureTarget::None, GestureTarget::None, GestureTarget::None, GestureTarget::None, GestureTarget::None, GestureTarget::None, GestureTarget::None, GestureTarget::None, GestureTarget::None, GestureTarget::None, GestureTarget::None, 60, 1, 1, MusicalRangeMode::OctaveRange, 60, 72, false);

            timeout = 10;
            while (!messageReceived && timeout > 0) { juce::Thread::sleep(10); timeout--; }
            expect(messageReceived, "Failed to send initial default note state.");

            // Second call: Exact same state
            messageReceived = false;
            osc.processHandData(emptyHand, emptyHand, 1.0f, 0, 1, 0, 1, 0, 1, GestureTarget::None, GestureTarget::None, GestureTarget::None, GestureTarget::None, GestureTarget::None, GestureTarget::None, GestureTarget::None, GestureTarget::None, GestureTarget::None, GestureTarget::None, GestureTarget::None, GestureTarget::None, GestureTarget::None, GestureTarget::None, GestureTarget::None, GestureTarget::None, GestureTarget::None, GestureTarget::None, GestureTarget::None, GestureTarget::None, GestureTarget::None, GestureTarget::None, 60, 1, 1, MusicalRangeMode::OctaveRange, 60, 72, false);

            timeout = 10;
            while (!messageReceived && timeout > 0) { juce::Thread::sleep(10);  timeout--; }
            expect(!messageReceived, "Spam Prevention Failed! System sent duplicate identical state packets to PD.");
        }

        beginTest("3. Right Hand Address Separation");
        {
            OscManager osc;
            osc.connectSender("127.0.0.1", testPort);
            messageReceived = false;

            // Force a right hand message
            osc.routeMessage(GestureTarget::Pitch, 0.5f, "right", 0, 12, 1, MusicalRangeMode::OctaveRange, 60, 72);

            int timeout = 50;
            while (!messageReceived && timeout > 0) { juce::Thread::sleep(10); timeout--; }

            expect(messageReceived, "Failed to receive Right Hand OSC Packet.");
            expectEquals(lastReceivedAddress, juce::String("/right/pitch"), "Right hand address formatting failed to differentiate from left.");
        }
    }


       
};

static OscManagerTests oscManagerTestsInstance;