#pragma once

#include <JuceHeader.h>
#include "LeapC.h"
#include "HandData.h"

class LeapService {
public:
    LeapService()
    {
        LeapCreateConnection(nullptr, &connectionHandle);
        LeapOpenConnection(connectionHandle);
    }

    ~LeapService()
    {
        stop();
    }

    void stop();
    void pollHandData(HandData& leftHand, HandData& rightHand, bool& isSensorConnected);
    void convertLeapEventToHandData(const LEAP_TRACKING_EVENT* event, HandData& leftHand, HandData& rightHand);

private:
    LEAP_CONNECTION connectionHandle = nullptr;
};