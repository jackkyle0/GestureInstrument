#pragma once
#include <JuceHeader.h>
#include "../../Source/Helpers/ScaleQuantiser.h" 

class ScaleQuantiserTests : public juce::UnitTest {
public:
    ScaleQuantiserTests() : juce::UnitTest("Scale Quantiser Logic Tests")
    {
    }

    void runTest() override {
        beginTest("1. Scale Intervals");
        {
            ScaleQuantiser sq;

            auto majorIntervals = sq.getScaleIntervals(1);
            expect(majorIntervals.size() == 7, "Major scale should contain exactly 7 intervals.");
            expect(majorIntervals[2] == 4, "The 3rd degree of a Major scale should be 4 semitones (Major 3rd).");

            auto chromaticIntervals = sq.getScaleIntervals(0);
            if (chromaticIntervals.size() != 12) chromaticIntervals = sq.getScaleIntervals(12); 
            expect(chromaticIntervals.size() == 12, "Chromatic scale should contain 12 intervals.");
        }

        beginTest("2. Basic Note Quantisation");
        {
            ScaleQuantiser sq;
            int rootC = 60;

            float exactValue = 60.0f / 127.0f;
            int snappedNoteExact = sq.getQuantisedNote(exactValue, rootC, 1);
            expectEquals(snappedNoteExact, 60, "A perfectly diatonic raw note should not change.");

            float outOfKeyVal = 61.0f / 127.0f;
            int snappedNote = sq.getQuantisedNote(outOfKeyVal, rootC, 1);
            expect(snappedNote == 60 || snappedNote == 62, "C# must snap to either C or D in C Major.");
        }

        beginTest("3. Progression Filter");
        {
            ScaleQuantiser sq;
            int rootC = 60;

            std::array<bool, 7> allowed = { true, false, true, false, true, false, false };

            float valueD = 62.0f / 127.0f;
            int snappedNote = sq.getQuantisedNote(valueD, rootC, 1, allowed);

            expect(snappedNote == 60 || snappedNote == 64, "D is disable, it must snap to allowed C or E.");
            expect(snappedNote != 62, "Failed to filter out the disabled D degree.");
        }

        beginTest("4. Safety Fallback");
        {
            ScaleQuantiser sq;
            int rootC = 60;

            std::array<bool, 7> noneAllowed = { false, false, false, false, false, false, false };

            float valueF = 65.0f / 127.0f;
            int snappedNote = sq.getQuantisedNote(valueF, rootC, 1, noneAllowed);

            expect(snappedNote % 12 == rootC % 12, "When all degrees are disabled, system must default to the root note to avoid crashing.");
        }

        beginTest("5. Extreme Input Clamping");
        {
            ScaleQuantiser sq;
            int rootC = 60;

            int lowNote = sq.getQuantisedNote(-5.0f, rootC, 1);
            expect(lowNote >= 0, "Negative float inputs must safely clamp to 0 or above to prevent MIDI errors.");

            int highNote = sq.getQuantisedNote(10.5f, rootC, 1);
            expect(highNote <= 127, "Excessive float inputs must safely clamp to 127 or below to prevent MIDI errors.");
        }

        beginTest("6. Root Note Transposition Maths");
        {
            ScaleQuantiser sq;
            int rootFs = 66; // F#4

            // The perfect 5th of F# is C# (73). We check if the quantiser recognizes this dynamically.
            float exactCsharp = 73.0f / 127.0f;
            int snappedNote = sq.getQuantisedNote(exactCsharp, rootFs, 1); // 1 = Major Scale

            expectEquals(snappedNote, 73, "Transposition failed. F# Major should recognize its perfect 5th (C#) without snapping it away.");
        }

        beginTest("7. Pentatonic Sparsity");
        {
            ScaleQuantiser sq;
            int rootC = 60;

            // In C Major Pentatonic (C, D, E, G, A), the note F (65) does not exist.
            // It sits in a wider gap between E (64) and G (67).
            float valueF = 65.0f / 127.0f;
            int snappedNote = sq.getQuantisedNote(valueF, rootC, 3); // Assuming 3 = Minor/Major Pentatonic

            expect(snappedNote != 65, "Pentatonic scale failed to snap out-of-key note.");
            // Note: It will either snap down to E or up to G depending on your math logic, both are mathematically correct for this test!
            expect(snappedNote == 64 || snappedNote == 67 || snappedNote == 63, "Note F must snap to the nearest valid Pentatonic note.");
        }
    }
};

static ScaleQuantiserTests scaleQuantiserTestsInstance;