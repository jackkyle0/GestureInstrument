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
        stopThread(1000);
    }

    void run() override {
        HandData persistentLeft;
        HandData persistentRight;
        bool persistentConnected = false;

        while (!threadShouldExit()) {
            // leapService.pollHandData will ONLY overwrite these if the USB queue has new data
            leapService.pollHandData(persistentLeft, persistentRight, persistentConnected);

            // Lock and copy the persistent data to the shared memory 
            {
                juce::ScopedLock sl(dataLock);
                sharedLeft = persistentLeft;
                sharedRight = persistentRight;
                sharedConnected = persistentConnected;
            }

            sleep(5); // polling rate
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