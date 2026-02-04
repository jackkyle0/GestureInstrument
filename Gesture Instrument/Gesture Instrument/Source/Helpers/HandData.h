#pragma once

struct FingerData
{
    int type = 0;
    float fingerPositionX = 0.0f;
    float fingerPositionY = 0.0f;
    float fingerPositionZ = 0.0f;
    bool isExtended = false;
};

struct HandData
{
    float currentHandPositionX = 0.0f;
    float currentHandPositionY = 0.0f;
    float currentHandPositionZ = 0.0f;
    float currentWristRotation = 0.0f;
    float grabStrength = 0.0f;
    float pinchStrength = 0.0f;
    bool isPinching = false;
    bool isPresent = false;
    FingerData fingers[5];
};
