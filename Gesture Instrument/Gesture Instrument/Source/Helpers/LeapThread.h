#pragma once

#include <JuceHeader.h>
#include "LeapService.h"
#include "HandData.h"

class LeapThread : public juce::Thread {
public:
    LeapThread() : juce::Thread("Leap Polling Thread")
    {
    }

    ~LeapThread() override {
        shutdown(); // Call our custom shutdown when destroyed
    }

    void shutdown() {
        signalThreadShouldExit(); // 1. Signal the loop to break
        leapService.stop();       // 2. Abort the USB poll so the thread unblocks
        stopThread(3000);         // 3. Give it plenty of time to  die
    }

    void run() override {
        HandData persistentLeft;
        HandData persistentRight;
        bool persistentConnected = false;

        while (!threadShouldExit()) {
            // leapService.pollHandData will ONLY overwrite these if the USB queue has new data
            leapService.pollHandData(persistentLeft, persistentRight, persistentConnected);

            {
                juce::ScopedLock sl(dataLock);
                sharedLeft = persistentLeft;
                sharedRight = persistentRight;
                sharedConnected = persistentConnected;
            }

            wait(5); // polling rate
        }
    }

    void getLatestData(HandData& outLeft, HandData& outRight, bool& outConnected) {
        const juce::ScopedTryLock sl(dataLock);

        if (sl.isLocked()) {
            lastLeft = sharedLeft;
            lastRight = sharedRight;
            lastConnected = sharedConnected;
        }

        // Output whatever the last successful read was
        outLeft = lastLeft;
        outRight = lastRight;
        outConnected = lastConnected;
    }

private:
    LeapService leapService;
    juce::CriticalSection dataLock;

    HandData sharedLeft;
    HandData sharedRight;
    bool sharedConnected = false;

    HandData lastLeft;
    HandData lastRight;
    bool lastConnected = false;
};