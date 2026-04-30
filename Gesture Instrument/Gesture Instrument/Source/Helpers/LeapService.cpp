#include "LeapService.h"

void LeapService::stop() {
    if (connectionHandle) {
        LeapCloseConnection(connectionHandle);
        LeapDestroyConnection(connectionHandle);
        connectionHandle = nullptr;
    }
}

void LeapService::pollHandData(HandData& leftHand, HandData& rightHand, bool& isConnected) {
    LEAP_CONNECTION_MESSAGE message;

    while (LeapPollConnection(connectionHandle, 0, &message) == eLeapRS_Success) {

        if (message.type == eLeapEventType_Tracking) {
            convertLeapEventToHandData(message.tracking_event, leftHand, rightHand);
            isConnected = true; // Hardware is actively tracking
        }
        else if (message.type == eLeapEventType_Device) {
            // Sensor was recognized
            LeapSetPolicyFlags(connectionHandle, eLeapPolicyFlag_BackgroundFrames, 0);
            isConnected = true;
        }
        else if (message.type == eLeapEventType_DeviceLost) {
            // Sensor was unplugged
            isConnected = false;
            leftHand.isPresent = false;
            rightHand.isPresent = false;
        }
        else if (message.type == eLeapEventType_Connection) {
            //Connected to the Leap sofwtare
            LeapSetPolicyFlags(connectionHandle, eLeapPolicyFlag_BackgroundFrames, 0);
        }
        else if (message.type == eLeapEventType_ConnectionLost) {
            // The background software stopped/crashed
            isConnected = false;
            leftHand.isPresent = false;
            rightHand.isPresent = false;
        }
    }
}

void LeapService::convertLeapEventToHandData(const LEAP_TRACKING_EVENT* event, HandData& leftHand, HandData& rightHand) {
    leftHand.isPresent = false;
    rightHand.isPresent = false;

    for (uint32_t i = 0; i < event->nHands; ++i) {
        const LEAP_HAND& hand = event->pHands[i];
        HandData* targetHand = (hand.type == eLeapHandType_Right) ? &rightHand : &leftHand;

        targetHand->isPresent = true;
        targetHand->currentHandPositionX = hand.palm.position.x;
        targetHand->currentHandPositionY = hand.palm.position.y;
        targetHand->currentHandPositionZ = hand.palm.position.z;
        targetHand->currentWristRotation = hand.palm.orientation.z;

        targetHand->grabStrength = hand.grab_strength;
        targetHand->pinchStrength = hand.pinch_strength;
        targetHand->isPinching = (hand.pinch_strength > 0.8f);

        for (int f = 0; f < 5; ++f) {
            const LEAP_DIGIT& digit = hand.digits[f];

            const LEAP_BONE& tip = digit.bones[3];
            targetHand->fingers[f].tipX = tip.next_joint.x;
            targetHand->fingers[f].tipY = tip.next_joint.y;
            targetHand->fingers[f].tipZ = tip.next_joint.z;

            const LEAP_BONE& joint1 = digit.bones[2];
            targetHand->fingers[f].joint1X = joint1.next_joint.x;
            targetHand->fingers[f].joint1Y = joint1.next_joint.y;
            targetHand->fingers[f].joint1Z = joint1.next_joint.z;

            const LEAP_BONE& joint2 = digit.bones[1];
            targetHand->fingers[f].joint2X = joint2.next_joint.x;
            targetHand->fingers[f].joint2Y = joint2.next_joint.y;
            targetHand->fingers[f].joint2Z = joint2.next_joint.z;

            const LEAP_BONE& knuckle = digit.bones[0];
            targetHand->fingers[f].knuckleX = knuckle.next_joint.x;
            targetHand->fingers[f].knuckleY = knuckle.next_joint.y;
            targetHand->fingers[f].knuckleZ = knuckle.next_joint.z;

            targetHand->fingers[f].isExtended = digit.is_extended;
        }
    }
}