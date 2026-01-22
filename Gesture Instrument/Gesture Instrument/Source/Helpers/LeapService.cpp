#include "LeapService.h"

LeapService::LeapService() {
    LeapCreateConnection(nullptr, &connectionHandle);
    if (LeapOpenConnection(connectionHandle) == eLeapRS_Success) {
        // Connection started
    }
}

LeapService::~LeapService() {
    if (connectionHandle) {
        LeapCloseConnection(connectionHandle);
        LeapDestroyConnection(connectionHandle);
    }
}

void LeapService::pollHandData(HandData& leftHand, HandData& rightHand, bool& isConnected) {
    LEAP_CONNECTION_MESSAGE message;

    while (LeapPollConnection(connectionHandle, 0, &message) == eLeapRS_Success)
    {
        if (message.type == eLeapEventType_Tracking) {
            convertLeapEventToHandData(message.tracking_event, leftHand, rightHand);
        }
        else if (message.type == eLeapEventType_DeviceLost) {
            isConnected = false;
        }
        else if (message.type == eLeapEventType_Device) {
            isConnected = true;
        }
    }
}
void LeapService::convertLeapEventToHandData(const LEAP_TRACKING_EVENT* event, HandData& leftHand, HandData& rightHand) {
        leftHand.isPresent = false;
        rightHand.isPresent = false;

        for (uint32_t i = 0; i < event->nHands; ++i) {
            const LEAP_HAND& hand = event->pHands[i];

            HandData* dataToUpdate = (hand.type == eLeapHandType_Right) ? &rightHand : &leftHand;

            dataToUpdate->isPresent = true;
            dataToUpdate->currentHandPositionX = hand.palm.position.x;
            dataToUpdate->currentHandPositionY = hand.palm.position.y;
            dataToUpdate->currentHandPositionZ = hand.palm.position.z;
            dataToUpdate->currentWristRotation = hand.palm.orientation.z;
            dataToUpdate->isPinching = (hand.pinch_strength > 0.8f);

            for (int f = 0; f < 5; ++f) {
                const LEAP_DIGIT& digit = hand.digits[f];
                dataToUpdate->fingers[f].type = f;
                dataToUpdate->fingers[f].fingerPositionX = digit.distal.next_joint.x;
                dataToUpdate->fingers[f].fingerPositionY = digit.distal.next_joint.y;
                dataToUpdate->fingers[f].fingerPositionZ = digit.distal.next_joint.z;
                dataToUpdate->fingers[f].isExtended = digit.is_extended;
            }
        }
    }
