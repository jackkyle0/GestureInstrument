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

        // Grab/Pinch Strength (0.0 to 1.0)
        dataToUpdate->grabStrength = hand.grab_strength;
        dataToUpdate->pinchStrength = hand.pinch_strength;
        dataToUpdate->isPinching = (hand.pinch_strength > 0.8f);

        //Finger Logic
        for (int f = 0; f < 5; ++f) {
            const LEAP_DIGIT& digit = hand.digits[f];

            // Tip
            const LEAP_BONE& tip = digit.bones[3];
            dataToUpdate->fingers[f].tipX = tip.next_joint.x;
            dataToUpdate->fingers[f].tipY = tip.next_joint.y;
            dataToUpdate->fingers[f].tipZ = tip.next_joint.z;

            // Joint 1 
            const LEAP_BONE& joint1 = digit.bones[2];
            dataToUpdate->fingers[f].joint1X = joint1.next_joint.x;
            dataToUpdate->fingers[f].joint1Y = joint1.next_joint.y;
            dataToUpdate->fingers[f].joint1Z = joint1.next_joint.z;

            // Joint 2
            const LEAP_BONE& joint2 = digit.bones[1];
            dataToUpdate->fingers[f].joint2X = joint2.next_joint.x;
            dataToUpdate->fingers[f].joint2Y = joint2.next_joint.y;
            dataToUpdate->fingers[f].joint2Z = joint2.next_joint.z;

            // Knuckle
            const LEAP_BONE& knuckle = digit.bones[0];
            dataToUpdate->fingers[f].knuckleX = knuckle.next_joint.x;
            dataToUpdate->fingers[f].knuckleY = knuckle.next_joint.y;
            dataToUpdate->fingers[f].knuckleZ = knuckle.next_joint.z;

            // I might need to delete these ***
            dataToUpdate->fingers[f].fingerPositionX = tip.next_joint.x;
            dataToUpdate->fingers[f].fingerPositionY = tip.next_joint.y;
            dataToUpdate->fingers[f].fingerPositionZ = tip.next_joint.z;
            dataToUpdate->fingers[f].isExtended = digit.is_extended;
        }
    }
}