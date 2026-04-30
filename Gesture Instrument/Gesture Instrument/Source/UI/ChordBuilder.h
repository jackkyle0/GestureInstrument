#pragma once

#include <JuceHeader.h>
#include "../PluginProcessor.h"

class ChordBuilder : public juce::Component {
public:
    ChordBuilder(GestureInstrumentAudioProcessor& p);
    ~ChordBuilder() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void refreshUI();

    juce::TextButton closeButton{ "Close" };

private:
    GestureInstrumentAudioProcessor& audioProcessor;

    juce::ToggleButton enableEngineBtn{ "Enable Chord Builder" };
    juce::Label titleLabel{ "", "CHORD BUILDER" };

    // Left hand
    juce::Label leftHandLabel{ "", "LEFT HAND" };
    juce::Label leftVoicingLabel{ "", "CHORD SHAPE" };
    juce::Label leftProgLabel{ "", "PROGRESSION" };
    juce::Label leftInvLabel{ "", "BASS AND INVERSIONS" };

    juce::TextButton leftDegreeButtons[7];
    juce::TextButton leftRootButtons[7];

    juce::TextButton leftTriadBtn, leftSeventhBtn, leftNinthBtn, leftSus2Btn, leftSus4Btn, leftAdd6Btn;
    juce::TextButton leftAllRootsBtn;
    juce::TextButton leftInvRootBtn, leftInvFirstBtn, leftInvSecondBtn, leftDropBassBtn;

    // Right hand
    juce::Label rightHandLabel{ "", "RIGHT HAND" };
    juce::Label rightVoicingLabel{ "", "CHORD SHAPE" };
    juce::Label rightProgLabel{ "", "PROGRESSION" };
    juce::Label rightInvLabel{ "", "BASS AND INVERSIONS" };

    juce::TextButton rightDegreeButtons[7];
    juce::TextButton rightRootButtons[7];

    juce::TextButton rightTriadBtn, rightSeventhBtn, rightNinthBtn, rightSus2Btn, rightSus4Btn, rightAdd6Btn;
    juce::TextButton rightAllRootsBtn;
    juce::TextButton rightInvRootBtn, rightInvFirstBtn, rightInvSecondBtn, rightDropBassBtn;

    // Array and helpers
    juce::String degreeNames[7] = { "1", "2", "3", "4", "5", "6", "7" };
    juce::String rootNames[7] = { "I", "ii", "iii", "IV", "V", "vi", "vii" };

    void updateProcessorFromButtons();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChordBuilder)
};