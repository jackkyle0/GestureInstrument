#pragma once

#include <JuceHeader.h>
#include "LeapC.h"
#include "Helpers/HandData.h"

class LeapService 
{
public:
    LeapService();
    ~LeapService();

    void pollHandData(HandData& leftHand, HandData& rightHand, bool& isSensorConnected);

private:
    LEAP_CONNECTION connectionHandle;

    void convertLeapEventToHandData(const LEAP_TRACKING_EVENT* event, HandData& left, HandData& right);

};