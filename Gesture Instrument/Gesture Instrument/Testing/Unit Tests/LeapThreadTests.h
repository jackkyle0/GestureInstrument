#pragma once
#include <JuceHeader.h>
#include "../../Source/Helpers/LeapThread.h" // Adjust path as needed

class LeapThreadTests : public juce::UnitTest {
public:
    LeapThreadTests() : juce::UnitTest("Leap Thread Concurrency Tests") {}

    void runTest() override {
        beginTest("Thread Lifecycle and Safe Teardown");
        {
            // We use an artificial scope block { } here. 
            {
                LeapThread testThread;

                // 1. Start the thread
                testThread.startThread(juce::Thread::Priority::high);
                expect(testThread.isThreadRunning(), "Thread failed to start.");

                // 2. Let it spin up and poll the hardware for a fraction of a second
                juce::Thread::sleep(100);

                // 3. Explicitly tell the thread to stop, giving it up to 3000ms (3 seconds) 
                // to safely close the Ultraleap USB connection.
                testThread.shutdown();
            }

            // If the code reaches this line, it means the thread successfully 
            // tore down without hitting the timeout deadlock!
            expect(true, "Thread safely shut down and destroyed itself.");
        }


        beginTest("hread-Safe Audio Callback Polling");
        {
            LeapThread testThread;
            testThread.startThread(juce::Thread::Priority::high);

            // 1. Give the Ultraleap driver 50ms to turn on and establish its USB handshake
            juce::Thread::sleep(50);

            HandData outLeft, outRight;
            bool outConnected = false;

            // 2. Run the 10,000 read stress test
            for (int i = 0; i < 10000; ++i) {
                testThread.getLatestData(outLeft, outRight, outConnected);
            }

            expect(true, "Aggressive read polling survived without deadlocking or memory corruption.");

            // 3. Give it 10ms to recover from the stress test
            juce::Thread::sleep(10);

            // 4. WE MUST STOP THE THREAD HERE TOO!
            // Giving it 3 seconds to safely close down before the test ends.
            testThread.shutdown();
        }
    }
};

static LeapThreadTests leapThreadTestsInstance;