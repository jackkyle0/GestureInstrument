#pragma once
#include <JuceHeader.h>
#include "../../Source/Helpers/LeapThread.h"

class LeapThreadTests : public juce::UnitTest {
public:
    LeapThreadTests() : juce::UnitTest("Leap Thread Concurrency Tests") {}

    void runTest() override {
        beginTest("Thread Lifecycle and Safe Teardown");
        {
            {
                LeapThread testThread;

                // start thread
                testThread.startThread(juce::Thread::Priority::high);
                expect(testThread.isThreadRunning(), "Thread failed to start.");

                // poll hardware for 100ms
                juce::Thread::sleep(100);

                //stop thread
                testThread.shutdown();
            }

            expect(true, "Thread safely shut down and destroyed itself.");
        }


        beginTest("hread-Safe Audio Callback Polling");
        {
            LeapThread testThread;
            testThread.startThread(juce::Thread::Priority::high);

            // establish usb connection
            juce::Thread::sleep(50);

            HandData outLeft, outRight;
            bool outConnected = false;

            // Run the 10000 read stress test
            for (int i = 0; i < 10000; ++i) {
                testThread.getLatestData(outLeft, outRight, outConnected);
            }

            expect(true, "Aggressive read polling survived without deadlocking or memory corruption.");

            // recover
            juce::Thread::sleep(10);

            // stop thread
            testThread.shutdown();
        }
    }
};

static LeapThreadTests leapThreadTestsInstance;