#pragma once

#include <JuceHeader.h>
#include "LeapService.h"
#include "HandData.h"

class LeapThread : public juce::Thread {
public:
    LeapThread()
        : juce::Thread("Leap Polling Thread")
    {
    }

    ~LeapThread() override
    {
        shutdown();
    }

    void shutdown() {
        signalThreadShouldExit();
        stopThread(3000);   
        leapService.stop(); // Safe to destroy the connection 
    }

    void run() override {
        HandData persistentLeft;
        HandData persistentRight;
        bool persistentConnected = false;

        while (!threadShouldExit()) {
            leapService.pollHandData(persistentLeft, persistentRight, persistentConnected);

            {
                juce::ScopedLock sl(dataLock);
                sharedLeft = persistentLeft;
                sharedRight = persistentRight;
                sharedConnected = persistentConnected;
            }

            wait(5);
        }
    }

    void getLatestData(HandData& outLeft, HandData& outRight, bool& outConnected) {
        const juce::ScopedTryLock sl(dataLock);

        if (sl.isLocked()) {
            lastLeft = sharedLeft;
            lastRight = sharedRight;
            lastConnected = sharedConnected;
        }

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