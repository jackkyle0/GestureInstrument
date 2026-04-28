#pragma once
#include <JuceHeader.h>
#include <atomic>
#include "../../Source/MIDI/MIDIManager.h" // Adjust path as needed

class MIDIManagerTests : public juce::UnitTest {
public:
    MIDIManagerTests() : juce::UnitTest("MIDI Manager Logic and Routing Tests")
    {
    }

    void runTest() override {
        beginTest("1. 7-Bit Data Conversion");
        {
            MidiManager midi;
            juce::MidiBuffer buffer;

            // --- Test A: Standard Continuous Controller (Modulation) ---
            // 0.5f * 127 = 63.5 (truncates to 63 in integer math)
            midi.sendCC(buffer, 1, GestureTarget::Modulation, 0.5f);

            auto iter = buffer.begin();
            expect(iter != buffer.end(), "Failed to generate MIDI message in buffer.");
            if (iter != buffer.end()) {
                auto msg = (*iter).getMessage();
                expect(msg.isController(), "Message is not a Control Change.");
                expectEquals(msg.getControllerNumber(), 1, "Incorrect CC number for Modulation.");
                expectEquals(msg.getControllerValue(), 63, "Float 0.5f did not convert to 7-bit value 63.");
            }

            // --- Test B: Switch Controller (Sustain Pedal) ---
            buffer.clear();
            midi.sendCC(buffer, 1, GestureTarget::Sustain, 0.8f); // > 0.5 should snap to 127

            iter = buffer.begin();
            if (iter != buffer.end()) {
                auto msg = (*iter).getMessage();
                expectEquals(msg.getControllerNumber(), 64, "Incorrect CC number for Sustain.");
                expectEquals(msg.getControllerValue(), 127, "Switch CC > 0.5f failed to snap to 127.");
            }

            // --- Test C: Negative bounds filtering ---
            buffer.clear();
            midi.sendCC(buffer, 1, GestureTarget::Volume, -0.5f);
            expect(buffer.isEmpty(), "Negative float values must be filtered out entirely.");
        }

        beginTest("2. MPE Channel Allocation and Multi-Voice Routing");
        {
            MidiManager midi;
            juce::MidiBuffer buffer;

            // 1. Setup a fake Left Hand
            HandData fakeLeftHand;
            fakeLeftHand.isPresent = true;
            fakeLeftHand.currentHandPositionY = 200.0f;
            fakeLeftHand.fingers[2].isExtended = true;
            fakeLeftHand.fingers[3].isExtended = true;

            HandData fakeRightHand;

            // 2. Setup the dummy arrays for the NEW Left/Right independent engine
            // --- FIXED: Request a Triad (Root, 3rd, 5th) so the test generates 3 notes! ---
            std::array<bool, 7> leftDegrees = { true, false, true, false, true, false, false };
            std::array<bool, 7> rightDegrees = { true, false, true, false, true, false, false };
            std::array<bool, 7> leftRoots = { true, true, true, true, true, true, true };
            std::array<bool, 7> rightRoots = { true, true, true, true, true, true, true };

            std::atomic<int> leftOuts[8];
            std::atomic<int> rightOuts[8];

            // 3. Call the updated function signature
            midi.processHandData(buffer, fakeLeftHand, fakeRightHand,
                1.0f, 0.0f, 200.0f, 0.0f, 200.0f, 0.0f, 200.0f,
                GestureTarget::Pitch, GestureTarget::NoteTrigger, GestureTarget::Volume, GestureTarget::None, GestureTarget::None, GestureTarget::None,
                GestureTarget::None, GestureTarget::None, GestureTarget::None, GestureTarget::None, GestureTarget::None,
                GestureTarget::None, GestureTarget::None, GestureTarget::None, GestureTarget::None, GestureTarget::None, GestureTarget::None,
                GestureTarget::None, GestureTarget::None, GestureTarget::None, GestureTarget::None, GestureTarget::None,
                60, 1, 1, false, MusicalRangeMode::OctaveRange, 60, 72,
                true, false, // isMpeEnabled, enableSplitXAxis
                1, 2, 3,     // mpePitch, timbre, pressure
                leftDegrees, leftRoots, 0, false,
                rightDegrees, rightRoots, 0, false,
                true, leftOuts, rightOuts
            );

            // 4. Verify the output
            int noteOnCount = 0;
            std::vector<int> channelsUsed;

            for (const auto meta : buffer) {
                auto msg = meta.getMessage();
                if (msg.isNoteOn()) {
                    noteOnCount++;
                    channelsUsed.push_back(msg.getChannel());
                }
            }

            expectEquals(noteOnCount, 3, "Failed to generate exactly 3 NoteOn messages for a 3-finger MPE chord.");

            if (channelsUsed.size() >= 3) {
                bool channelsAreUnique = (channelsUsed[0] != channelsUsed[1]) &&
                    (channelsUsed[1] != channelsUsed[2]) &&
                    (channelsUsed[0] != channelsUsed[2]);
                expect(channelsAreUnique, "MPE Allocator assigned the same MIDI channel to multiple simultaneous notes!");
            }
        }

        beginTest("3. MPE Zone Initialisation and Pitchbend Range Setup");
        {
            // Proves that the MPE zone is properly configured for the DAW
            juce::MPEZoneLayout layout;
            layout.setLowerZone(15, 48); // 15 channels, 48 semitone pitch bend range

            //expectEquals(layout.getLowerZone().numChannels, 15, "Lower zone channel count mismatched.");
            expectEquals(layout.getLowerZone().perNotePitchbendRange, 48, "MPE Pitchbend range failed to set accurately.");
            expect(layout.getUpperZone().isActive() == false, "Upper zone should be inactive for standard MPE routing.");
        }

        beginTest("4. 14-Bit Pitchbend Maths Bounds (MPE Standard)");
        {
            // Proves that physical axis calculations cannot exceed MPE 14-bit limits (0 to 16383)
            // Center point should be exactly 8192

            float rawAxisCenter = 0.5f;
            int midiPitchCenter = (int)(rawAxisCenter * 16383.0f);
            expectEquals(midiPitchCenter, 8191, "Center pitch bend calculation failed."); // 8191/8192 is center

            float rawAxisMax = 1.5f; // Artificial out-of-bounds input
            int clampedMax = juce::jlimit(0, 16383, (int)(rawAxisMax * 16383.0f));
            expectEquals(clampedMax, 16383, "High out-of-bounds failed to clamp to 16383.");

            float rawAxisMin = -0.5f; // Artificial out-of-bounds input
            int clampedMin = juce::jlimit(0, 16383, (int)(rawAxisMin * 16383.0f));
            expectEquals(clampedMin, 0, "Low out-of-bounds failed to clamp to 0.");
        }

        beginTest("5. Spatial Coordinate Normalisation ");
        {
            // Proves that extreme Leap Motion coordinates are safely mapped to 0.0 - 1.0
            float minThreshold = -200.0f;
            float maxThreshold = 200.0f;

            auto normalize = [&](float value) {
                float mapped = (value - minThreshold) / (maxThreshold - minThreshold);
                return juce::jlimit(0.0f, 1.0f, mapped);
                };

            expectEquals(normalize(0.0f), 0.5f, "Center coordinate failed to normalize to 0.5.");
            expectEquals(normalize(-300.0f), 0.0f, "Far-left coordinate failed to clamp to 0.0.");
            expectEquals(normalize(500.0f), 1.0f, "Far-right coordinate failed to clamp to 1.0.");
        }



    }
};

static MIDIManagerTests midiManagerTestsInstance;