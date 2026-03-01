#pragma once

#include <JuceHeader.h>
#include <onnxruntime_cxx_api.h>

class AdaptiveEngine {
public:
    AdaptiveEngine()
    {
        try {
            env = Ort::Env(ORT_LOGGING_LEVEL_WARNING, "AdaptiveEngine");
            memoryInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);

            Ort::SessionOptions sessionOptions;
            sessionOptions.SetIntraOpNumThreads(1);

            // load from binary 
            session = std::make_unique<Ort::Session>(env, BinaryData::gesture_model_packaged_onnx, BinaryData::gesture_model_packaged_onnxSize, sessionOptions);

            juce::Logger::writeToLog("Model loaded successfully");
        }
        catch (const Ort::Exception& e) {
            juce::Logger::writeToLog("ONNX Load Error: " + juce::String(e.what()));
        }
    }

    float predictStyle(float l_speed, float l_jitter, float r_speed, float r_jitter) {
        if (!session) {
            return 0.0f;
        }

        std::array<float, 4> inputValues = { l_speed, l_jitter, r_speed, r_jitter };
        std::array<int64_t, 2> inputShape = { 1, 4 };

        Ort::Value inputTensor = Ort::Value::CreateTensor<float>(
            memoryInfo, inputValues.data(), inputValues.size(), inputShape.data(), inputShape.size());

        const char* inputNames[] = { "input" };
        const char* outputNames[] = { "output" };

        try {
            auto outputTensors = session->Run(Ort::RunOptions{ nullptr }, inputNames, &inputTensor, 1, outputNames, 1);
            float* floatArray = outputTensors.front().GetTensorMutableData<float>();

            // Convert pytorch into values
            float maxLogit = std::max(floatArray[0], floatArray[1]);
            float expG = std::exp(floatArray[0] - maxLogit);
            float expA = std::exp(floatArray[1] - maxLogit);
            float aggressiveProbability = expA / (expG + expA);

            // Debug
            juce::Logger::writeToLog("AI Prob: " + juce::String(aggressiveProbability) +
                " | L_Spd: " + juce::String(l_speed) +
                " | R_Spd: " + juce::String(r_speed));

            return aggressiveProbability;
        }
        catch (const Ort::Exception& e) {
            juce::Logger::writeToLog("Error: " + juce::String(e.what()));
            return 0.0f;
        }
    }

private:
    Ort::Env env{ nullptr };
    std::unique_ptr<Ort::Session> session;
    Ort::MemoryInfo memoryInfo{ nullptr };
};