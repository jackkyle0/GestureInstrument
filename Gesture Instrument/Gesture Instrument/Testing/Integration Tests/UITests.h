#pragma once
#include <JuceHeader.h>
#include "../../Source/PluginProcessor.h"
#include "../../Source/UI/ChordBuilder.h" 

class ChordBuilderUITest : public juce::UnitTest {
public:
    ChordBuilderUITest() : juce::UnitTest("Chord Builder UI Interaction Test")
    {
    }

    void runTest() override {
        beginTest("Simulating Human User Clicking the 'JAZZ' Preset");

        // 1. BOOT UP THE ENGINE 
        GestureInstrumentAudioProcessor processor;

        // (Removed the manual MessageManager creation here!)

        // --- ARTIFICIAL SCOPE BLOCK ---
        {
            ChordBuilder editor(processor);

            // 2. FORCE A BLANK SLATE
            processor.rootII.store(false);
            processor.rootV.store(false);
            processor.rootI.store(false);
            processor.rootVI.store(true);

            expect(processor.rootII.load() == false, "Blank slate failed.");

            // 3. THE GHOST CLICK (Synchronous!)
            editor.jazzButton.onClick();

            // 4. ASSERT THE RESULTS
            expect(processor.rootII.load() == true, "UI Click Failed: Jazz preset did not activate the ii chord.");
            expect(processor.rootV.load() == true, "UI Click Failed: Jazz preset did not activate the V chord.");
            expect(processor.rootI.load() == true, "UI Click Failed: Jazz preset did not activate the I chord.");
            expect(processor.rootVI.load() == false, "UI Click Failed: Jazz preset left the vi chord active.");

        } // <-- The editor safely shuts down its LookAndFeel here!

        // (Removed the MessageManager::deleteInstance() here! Leave the host alone!)
    }
};

static ChordBuilderUITest uiTestInstance;