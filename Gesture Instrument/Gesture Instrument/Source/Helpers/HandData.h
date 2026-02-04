#pragma once

struct FingerData
{
    int type = 0;
    float tipX = 0.0f, tipY = 0.0f, tipZ = 0.0f;           
    float joint1X = 0.0f, joint1Y = 0.0f, joint1Z = 0.0f; 
    float joint2X = 0.0f, joint2Y = 0.0f, joint2Z = 0.0f; 
    float knuckleX = 0.0f, knuckleY = 0.0f, knuckleZ = 0.0f; 

    // I might not need these ***
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