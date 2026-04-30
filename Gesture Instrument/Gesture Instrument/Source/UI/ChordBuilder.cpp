#include "ChordBuilder.h"

ChordBuilder::ChordBuilder(GestureInstrumentAudioProcessor& p)
    : audioProcessor(p)
{
    setOpaque(false);

    addAndMakeVisible(enableEngineBtn);
    enableEngineBtn.setTooltip("Toggle the Diatonic Chord Engine on or off.");
    enableEngineBtn.setColour(juce::ToggleButton::tickColourId, juce::Colours::orange);
    enableEngineBtn.onClick = [this] { audioProcessor.chordEngineEnabled.store(enableEngineBtn.getToggleState()); };

    addAndMakeVisible(titleLabel);
    titleLabel.setFont(juce::Font(24.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centred);

    auto setupHeaderLabel = [](juce::Label& l, juce::Colour c) {
        l.setFont(juce::Font(18.0f, juce::Font::bold));
        l.setJustificationType(juce::Justification::centred);
        l.setColour(juce::Label::textColourId, c);
        };

    auto setupSectionLabel = [](juce::Label& l) {
        l.setFont(juce::Font(12.0f, juce::Font::bold));
        l.setJustificationType(juce::Justification::centred);
        l.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.4f));
        };

    setupHeaderLabel(leftHandLabel, juce::Colours::cyan);
    setupHeaderLabel(rightHandLabel, juce::Colours::magenta);

    setupSectionLabel(leftVoicingLabel); setupSectionLabel(rightVoicingLabel);
    setupSectionLabel(leftProgLabel);    setupSectionLabel(rightProgLabel);
    setupSectionLabel(leftInvLabel);     setupSectionLabel(rightInvLabel);

    addAndMakeVisible(leftHandLabel);    addAndMakeVisible(rightHandLabel);
    addAndMakeVisible(leftVoicingLabel); addAndMakeVisible(rightVoicingLabel);
    addAndMakeVisible(leftProgLabel);    addAndMakeVisible(rightProgLabel);
    addAndMakeVisible(leftInvLabel);     addAndMakeVisible(rightInvLabel);

    juce::String degTips[7] = { "Root", "2nd", "3rd", "4th", "5th", "6th", "7th" };
    juce::String rootTips[7] = { "Tonic (I)", "Supertonic (ii)", "Mediant (iii)", "Subdominant (IV)", "Dominant (V)", "Submediant (vi)", "Leading (vii)" };

    // Setup pad
    auto setupPad = [this](juce::TextButton* b, juce::TextButton* arrayPtr, juce::Colour col, juce::String tip, bool isDegreePad) {
        addAndMakeVisible(*b);
        b->setClickingTogglesState(true);
        b->setTooltip(tip);
        b->setColour(juce::TextButton::buttonOnColourId, col.withAlpha(0.6f));

        b->onClick = [this, b, arrayPtr, isDegreePad] {
            if (isDegreePad && b->getToggleState()) {
                int activeCount = 0;
                for (int j = 0; j < 7; ++j) {
                    if (arrayPtr[j].getToggleState()) activeCount++;
                }
                if (activeCount > 5) {
                    b->setToggleState(false, juce::dontSendNotification);
                }
            }
            updateProcessorFromButtons();
            };
        };

    for (int i = 0; i < 7; ++i) {
        setupPad(&leftDegreeButtons[i], leftDegreeButtons, juce::Colours::orange, degTips[i], true);
        leftDegreeButtons[i].setButtonText(degreeNames[i]);
        setupPad(&rightDegreeButtons[i], rightDegreeButtons, juce::Colours::orange, degTips[i], true);
        rightDegreeButtons[i].setButtonText(degreeNames[i]);

        setupPad(&leftRootButtons[i], leftRootButtons, juce::Colours::cyan, rootTips[i], false);
        leftRootButtons[i].setButtonText(rootNames[i]);
        setupPad(&rightRootButtons[i], rightRootButtons, juce::Colours::magenta, rootTips[i], false);
        rightRootButtons[i].setButtonText(rootNames[i]);
    }

    // Inversions
    auto setupInv = [this](juce::TextButton& b, int group, juce::String txt, bool isLeft, juce::Colour c) {
        addAndMakeVisible(b);
        b.setButtonText(txt);
        b.setClickingTogglesState(true);
        if (group > 0) b.setRadioGroupId(group);
        b.setColour(juce::TextButton::buttonOnColourId, c.withAlpha(0.6f));

        b.onClick = [this, &b, isLeft] {
            if (b.getButtonText() == "-12st") {
                if (isLeft) audioProcessor.leftDropBass.store(b.getToggleState());
                else audioProcessor.rightDropBass.store(b.getToggleState());
            }
            else {
                int mode = (b.getButtonText() == "Root") ? 0 : (b.getButtonText() == "3rd" ? 1 : 2);
                if (isLeft) audioProcessor.leftChordInversionMode.store(mode);
                else audioProcessor.rightChordInversionMode.store(mode);
            }
            };
        };

    setupInv(leftInvRootBtn, 101, "Root", true, juce::Colours::cyan);
    setupInv(leftInvFirstBtn, 101, "3rd", true, juce::Colours::cyan);
    setupInv(leftInvSecondBtn, 101, "5th", true, juce::Colours::cyan);
    setupInv(leftDropBassBtn, 0, "-12st", true, juce::Colours::cyan);

    setupInv(rightInvRootBtn, 201, "Root", false, juce::Colours::magenta);
    setupInv(rightInvFirstBtn, 201, "3rd", false, juce::Colours::magenta);
    setupInv(rightInvSecondBtn, 201, "5th", false, juce::Colours::magenta);
    setupInv(rightDropBassBtn, 0, "-12st", false, juce::Colours::magenta);

    // Setup preset
    auto setupPreset = [this](juce::TextButton& btn, juce::TextButton* targetArray, std::vector<int> activeIndices, juce::String txt) {
        addAndMakeVisible(btn);
        btn.setButtonText(txt);
        btn.onClick = [this, targetArray, activeIndices] {
            for (int i = 0; i < 7; ++i) {
                bool turnOn = std::find(activeIndices.begin(), activeIndices.end(), i) != activeIndices.end();
                targetArray[i].setToggleState(turnOn, juce::dontSendNotification);
            }
            updateProcessorFromButtons();
            };
        };

    setupPreset(leftTriadBtn, leftDegreeButtons, { 0, 2, 4 }, "1-3-5");
    setupPreset(leftSeventhBtn, leftDegreeButtons, { 0, 2, 4, 6 }, "1-3-5-7");
    setupPreset(leftNinthBtn, leftDegreeButtons, { 0, 1, 2, 4, 6 }, "1-2-3-5-7");
    setupPreset(leftSus2Btn, leftDegreeButtons, { 0, 1, 4 }, "1-2-5");
    setupPreset(leftSus4Btn, leftDegreeButtons, { 0, 3, 4 }, "1-4-5");
    setupPreset(leftAdd6Btn, leftDegreeButtons, { 0, 2, 4, 5 }, "1-3-5-6");
    setupPreset(leftAllRootsBtn, leftRootButtons, { 0, 1, 2, 3, 4, 5, 6 }, "Allow All");

    setupPreset(rightTriadBtn, rightDegreeButtons, { 0, 2, 4 }, "1-3-5");
    setupPreset(rightSeventhBtn, rightDegreeButtons, { 0, 2, 4, 6 }, "1-3-5-7");
    setupPreset(rightNinthBtn, rightDegreeButtons, { 0, 1, 2, 4, 6 }, "1-2-3-5-7");
    setupPreset(rightSus2Btn, rightDegreeButtons, { 0, 1, 4 }, "1-2-5");
    setupPreset(rightSus4Btn, rightDegreeButtons, { 0, 3, 4 }, "1-4-5");
    setupPreset(rightAdd6Btn, rightDegreeButtons, { 0, 2, 4, 5 }, "1-3-5-6");
    setupPreset(rightAllRootsBtn, rightRootButtons, { 0, 1, 2, 3, 4, 5, 6 }, "Allow All");

    addAndMakeVisible(closeButton);
    closeButton.setColour(juce::TextButton::buttonColourId, juce::Colours::white.withAlpha(0.1f));

    refreshUI();
}

ChordBuilder::~ChordBuilder() {}

void ChordBuilder::paint(juce::Graphics& g) {
    auto area = getLocalBounds().toFloat();
    g.fillAll(juce::Colours::black.withAlpha(0.85f));
    g.setColour(juce::Colours::white.withAlpha(0.1f));
    g.drawRect(area, 1.0f);

    
    g.drawLine(area.getWidth() / 2.0f, 60.0f, area.getWidth() / 2.0f, area.getHeight() - 80.0f, 1.0f);

    if (audioProcessor.currentOutputMode == OutputMode::OSC_Only) {
        g.fillAll(juce::Colours::black.withAlpha(0.7f));
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(24.0f, juce::Font::bold));
        g.drawText("CHORD BUILDER UNAVAILABLE", area.translated(0, -15), juce::Justification::centred);

        g.setFont(juce::Font(16.0f, juce::Font::plain));
        g.setColour(juce::Colours::grey);
        g.drawText("Chord Builder is currently only supported in MIDI Mode.", area.translated(0, 15), juce::Justification::centred);
    }
}

void ChordBuilder::resized() {
    auto bounds = getLocalBounds().reduced(20, 20);

    // Header Area
    auto header = bounds.removeFromTop(40);
    enableEngineBtn.setBounds(header.removeFromRight(120).reduced(0, 5));
    titleLabel.setBounds(header);

    bounds.removeFromTop(10);

    // Split Screen
    auto leftArea = bounds.removeFromLeft(bounds.getWidth() / 2).reduced(15, 0);
    auto rightArea = bounds.reduced(15, 0);

    auto centerRow = [&](juce::Rectangle<int> row, int itemW, int itemH, int count, juce::TextButton** items, int space) {
        int totalW = (count * itemW) + ((count - 1) * space);
        auto xArea = row.withSizeKeepingCentre(totalW, itemH);
        for (int i = 0; i < count; ++i) {
            items[i]->setBounds(xArea.removeFromLeft(itemW));
            xArea.removeFromLeft(space);
        }
        };

    auto layoutHand = [&](juce::Rectangle<int>& area, juce::Label& title, juce::Label& voic,
        juce::TextButton* vp1[], juce::TextButton* dPads[],
        juce::Label& prog, juce::TextButton* pp1[], juce::TextButton* rPads[],
        juce::Label& inv, juce::TextButton* iBtns[]) {

            title.setBounds(area.removeFromTop(30));
            area.removeFromTop(10);

            voic.setBounds(area.removeFromTop(20));
            centerRow(area.removeFromTop(22), 50, 22, 6, vp1, 5);
            area.removeFromTop(8);
            centerRow(area.removeFromTop(35), 35, 35, 7, dPads, 5);
            area.removeFromTop(20);

            prog.setBounds(area.removeFromTop(20));
            centerRow(area.removeFromTop(22), 120, 22, 1, pp1, 5);
            area.removeFromTop(8);
            centerRow(area.removeFromTop(35), 35, 35, 7, rPads, 5);
            area.removeFromTop(20);

            inv.setBounds(area.removeFromTop(20));
            centerRow(area.removeFromTop(30), 60, 30, 4, iBtns, 8);
        };

    // Arrays for layout mapping
    juce::TextButton* lVp[] = { &leftTriadBtn, &leftSeventhBtn, &leftNinthBtn, &leftSus2Btn, &leftSus4Btn, &leftAdd6Btn };
    juce::TextButton* lDp[7]; for (int i = 0; i < 7; ++i) lDp[i] = &leftDegreeButtons[i];
    juce::TextButton* lPp[] = { &leftAllRootsBtn };
    juce::TextButton* lRp[7]; for (int i = 0; i < 7; ++i) lRp[i] = &leftRootButtons[i];
    juce::TextButton* lIp[] = { &leftInvRootBtn, &leftInvFirstBtn, &leftInvSecondBtn, &leftDropBassBtn };

    juce::TextButton* rVp[] = { &rightTriadBtn, &rightSeventhBtn, &rightNinthBtn, &rightSus2Btn, &rightSus4Btn, &rightAdd6Btn };
    juce::TextButton* rDp[7]; for (int i = 0; i < 7; ++i) rDp[i] = &rightDegreeButtons[i];
    juce::TextButton* rPp[] = { &rightAllRootsBtn };
    juce::TextButton* rRp[7]; for (int i = 0; i < 7; ++i) rRp[i] = &rightRootButtons[i];
    juce::TextButton* rIp[] = { &rightInvRootBtn, &rightInvFirstBtn, &rightInvSecondBtn, &rightDropBassBtn };

    layoutHand(leftArea, leftHandLabel, leftVoicingLabel, lVp, lDp, leftProgLabel, lPp, lRp, leftInvLabel, lIp);
    layoutHand(rightArea, rightHandLabel, rightVoicingLabel, rVp, rDp, rightProgLabel, rPp, rRp, rightInvLabel, rIp);

    closeButton.setBounds(getLocalBounds().removeFromBottom(60).withSizeKeepingCentre(120, 35));
}

void ChordBuilder::updateProcessorFromButtons() {
    audioProcessor.leftChordDegree1.store(leftDegreeButtons[0].getToggleState());
    audioProcessor.leftChordDegree2.store(leftDegreeButtons[1].getToggleState());
    audioProcessor.leftChordDegree3.store(leftDegreeButtons[2].getToggleState());
    audioProcessor.leftChordDegree4.store(leftDegreeButtons[3].getToggleState());
    audioProcessor.leftChordDegree5.store(leftDegreeButtons[4].getToggleState());
    audioProcessor.leftChordDegree6.store(leftDegreeButtons[5].getToggleState());
    audioProcessor.leftChordDegree7.store(leftDegreeButtons[6].getToggleState());

    audioProcessor.leftRootI.store(leftRootButtons[0].getToggleState());
    audioProcessor.leftRootII.store(leftRootButtons[1].getToggleState());
    audioProcessor.leftRootIII.store(leftRootButtons[2].getToggleState());
    audioProcessor.leftRootIV.store(leftRootButtons[3].getToggleState());
    audioProcessor.leftRootV.store(leftRootButtons[4].getToggleState());
    audioProcessor.leftRootVI.store(leftRootButtons[5].getToggleState());
    audioProcessor.leftRootVII.store(leftRootButtons[6].getToggleState());

    audioProcessor.rightChordDegree1.store(rightDegreeButtons[0].getToggleState());
    audioProcessor.rightChordDegree2.store(rightDegreeButtons[1].getToggleState());
    audioProcessor.rightChordDegree3.store(rightDegreeButtons[2].getToggleState());
    audioProcessor.rightChordDegree4.store(rightDegreeButtons[3].getToggleState());
    audioProcessor.rightChordDegree5.store(rightDegreeButtons[4].getToggleState());
    audioProcessor.rightChordDegree6.store(rightDegreeButtons[5].getToggleState());
    audioProcessor.rightChordDegree7.store(rightDegreeButtons[6].getToggleState());

    audioProcessor.rightRootI.store(rightRootButtons[0].getToggleState());
    audioProcessor.rightRootII.store(rightRootButtons[1].getToggleState());
    audioProcessor.rightRootIII.store(rightRootButtons[2].getToggleState());
    audioProcessor.rightRootIV.store(rightRootButtons[3].getToggleState());
    audioProcessor.rightRootV.store(rightRootButtons[4].getToggleState());
    audioProcessor.rightRootVI.store(rightRootButtons[5].getToggleState());
    audioProcessor.rightRootVII.store(rightRootButtons[6].getToggleState());
}

void ChordBuilder::refreshUI() {
    bool isMidiMode = (audioProcessor.currentOutputMode != OutputMode::OSC_Only);

    for (auto* child : getChildren()) {
        if (child != &closeButton) {
            child->setEnabled(isMidiMode);
        }
    }

    for (int i = 0; i < 7; ++i) {
        bool leftDegreeOn = false, leftRootOn = false;
        if (i == 0) { leftDegreeOn = audioProcessor.leftChordDegree1.load(); leftRootOn = audioProcessor.leftRootI.load(); }
        else if (i == 1) { leftDegreeOn = audioProcessor.leftChordDegree2.load(); leftRootOn = audioProcessor.leftRootII.load(); }
        else if (i == 2) { leftDegreeOn = audioProcessor.leftChordDegree3.load(); leftRootOn = audioProcessor.leftRootIII.load(); }
        else if (i == 3) { leftDegreeOn = audioProcessor.leftChordDegree4.load(); leftRootOn = audioProcessor.leftRootIV.load(); }
        else if (i == 4) { leftDegreeOn = audioProcessor.leftChordDegree5.load(); leftRootOn = audioProcessor.leftRootV.load(); }
        else if (i == 5) { leftDegreeOn = audioProcessor.leftChordDegree6.load(); leftRootOn = audioProcessor.leftRootVI.load(); }
        else if (i == 6) { leftDegreeOn = audioProcessor.leftChordDegree7.load(); leftRootOn = audioProcessor.leftRootVII.load(); }

        leftDegreeButtons[i].setToggleState(leftDegreeOn, juce::dontSendNotification);
        leftRootButtons[i].setToggleState(leftRootOn, juce::dontSendNotification);

        bool rightDegreeOn = false, rightRootOn = false;
        if (i == 0) { rightDegreeOn = audioProcessor.rightChordDegree1.load(); rightRootOn = audioProcessor.rightRootI.load(); }
        else if (i == 1) { rightDegreeOn = audioProcessor.rightChordDegree2.load(); rightRootOn = audioProcessor.rightRootII.load(); }
        else if (i == 2) { rightDegreeOn = audioProcessor.rightChordDegree3.load(); rightRootOn = audioProcessor.rightRootIII.load(); }
        else if (i == 3) { rightDegreeOn = audioProcessor.rightChordDegree4.load(); rightRootOn = audioProcessor.rightRootIV.load(); }
        else if (i == 4) { rightDegreeOn = audioProcessor.rightChordDegree5.load(); rightRootOn = audioProcessor.rightRootV.load(); }
        else if (i == 5) { rightDegreeOn = audioProcessor.rightChordDegree6.load(); rightRootOn = audioProcessor.rightRootVI.load(); }
        else if (i == 6) { rightDegreeOn = audioProcessor.rightChordDegree7.load(); rightRootOn = audioProcessor.rightRootVII.load(); }

        rightDegreeButtons[i].setToggleState(rightDegreeOn, juce::dontSendNotification);
        rightRootButtons[i].setToggleState(rightRootOn, juce::dontSendNotification);
    }

    enableEngineBtn.setToggleState(audioProcessor.chordEngineEnabled.load(), juce::dontSendNotification);

    int leftInv = audioProcessor.leftChordInversionMode.load();
    leftInvRootBtn.setToggleState(leftInv == 0, juce::dontSendNotification);
    leftInvFirstBtn.setToggleState(leftInv == 1, juce::dontSendNotification);
    leftInvSecondBtn.setToggleState(leftInv == 2, juce::dontSendNotification);
    leftDropBassBtn.setToggleState(audioProcessor.leftDropBass.load(), juce::dontSendNotification);

    int rightInv = audioProcessor.rightChordInversionMode.load();
    rightInvRootBtn.setToggleState(rightInv == 0, juce::dontSendNotification);
    rightInvFirstBtn.setToggleState(rightInv == 1, juce::dontSendNotification);
    rightInvSecondBtn.setToggleState(rightInv == 2, juce::dontSendNotification);
    rightDropBassBtn.setToggleState(audioProcessor.rightDropBass.load(), juce::dontSendNotification);
}