#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "UI/HUDComponents.h"
#include "Helpers/MusicalRangeMode.h"

// ==============================================================================
// 1. SETUP & TEARDOWN (Constructor / Destructor)
// ==============================================================================

GestureInstrumentAudioProcessorEditor::GestureInstrumentAudioProcessorEditor(GestureInstrumentAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), settingsPage(p), staticDialsPage(p), hud(p), virtualCursor(p), chordBuilderPage(p),
    xMinControl("Min Width", -350.0f, 0.0f, p.minWidthThreshold),
    xMaxControl("Max Width", 0.0f, 350.0f, p.maxWidthThreshold),
    yMinControl("Min Height", 50.0f, 275.0f, p.minHeightThreshold),
    yMaxControl("Max Height", 275.0f, 500.0f, p.maxHeightThreshold),
    zMinControl("Back Depth", -225.0f, 0.0f, p.minDepthThreshold),
    zMaxControl("Front Depth", 0.0f, 225.0f, p.maxDepthThreshold)
{
    // ======================================================
    // CORE VISUALS & ENGINE
    // ======================================================
    addAndMakeVisible(hud);
    addAndMakeVisible(virtualCursor);
    virtualCursor.setAlwaysOnTop(true);

    openGLContext.attachTo(*this);
    openGLContext.setContinuousRepainting(false);

    audioProcessor.oscManager.onStyleChanged = [this](float leftStyle, float rightStyle) {
        juce::MessageManager::callAsync([this, leftStyle, rightStyle]() {
            float maxStyle = std::max(leftStyle, rightStyle);
            if (std::abs(currentStyle - maxStyle) > 0.01f) {
                currentStyle = maxStyle;
                hud.currentStyle = maxStyle;
                repaint();
            }
            });
        };

    // ======================================================
    // TOP BAR: HARDWARE & VIEW CONTROLS
    // ======================================================
    addAndMakeVisible(connectionStatusLabel);
    connectionStatusLabel.setText("Checking Sensor...", juce::dontSendNotification);
    connectionStatusLabel.setJustificationType(juce::Justification::centredRight);
    connectionStatusLabel.setColour(juce::Label::textColourId, juce::Colours::grey);

    addAndMakeVisible(maximizeButton);
    maximizeButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
    maximizeButton.onClick = [this] {
        if (auto* wrapperWindow = getTopLevelComponent()) {
            if (!isMaximized) {
                previousSize = getLocalBounds();
                previousPosition = wrapperWindow->getPosition();
                auto monitorArea = getParentMonitorArea();
                wrapperWindow->setTopLeftPosition(monitorArea.getPosition());
                setSize(monitorArea.getWidth(), monitorArea.getHeight() - 50);
                maximizeButton.setButtonText("Restore");
                isMaximized = true;
                // Right after you set isMaximized = true/false:
                audioProcessor.isWindowMaximized.store(isMaximized);
            }
            else {
                setSize(previousSize.getWidth(), previousSize.getHeight());
                wrapperWindow->setTopLeftPosition(previousPosition);
                maximizeButton.setButtonText("Maximize");
                isMaximized = false;
                // Right after you set isMaximized = true/false:
                audioProcessor.isWindowMaximized.store(isMaximized);
            }
        }
        };

    addAndMakeVisible(showNoteNamesButton);
    showNoteNamesButton.setButtonText("Show Notes");
    showNoteNamesButton.setToggleState(audioProcessor.showNoteNames, juce::dontSendNotification);

    showNoteNamesButton.setColour(juce::ToggleButton::textColourId, juce::Colours::white);
    showNoteNamesButton.onClick = [this] {
        audioProcessor.showNoteNames = showNoteNamesButton.getToggleState();
        repaint();
        };

    // ======================================================
    // TOP BAR: MUSICAL SETTINGS (KEY & SCALE)
    // ======================================================
    addAndMakeVisible(keyLabel);
    keyLabel.setText("Key:", juce::dontSendNotification);
    keyLabel.setColour(juce::Label::textColourId, juce::Colours::white);

    addAndMakeVisible(rootSelector);
    rootSelector.addItemList({ "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" }, 1);
    rootSelector.setSelectedId(p.rootNote + 1);
    rootSelector.onChange = [this] { audioProcessor.rootNote = rootSelector.getSelectedId() - 1; };

    addAndMakeVisible(scaleLabel);
    scaleLabel.setText("Scale:", juce::dontSendNotification);
    scaleLabel.setColour(juce::Label::textColourId, juce::Colours::white);

    addAndMakeVisible(scaleSelector);
    scaleSelector.addItem("Chromatic", 1);
    scaleSelector.addItem("Major", 2);
    scaleSelector.addItem("Minor", 3);
    scaleSelector.addItem("Major Pentatonic", 4);
    scaleSelector.addItem("Minor Pentatonic", 5);
    scaleSelector.addItem("Blues", 6);
    scaleSelector.addItem("Dorian", 7);
    scaleSelector.addItem("Mixolydian", 8);
    scaleSelector.addItem("Lydian", 9);
    scaleSelector.addItem("Phrygian", 10);
    scaleSelector.addItem("Harmonic Minor", 11);
    scaleSelector.addItem("Locrian", 12);
    scaleSelector.addItem("Unquantised", 13);
    scaleSelector.addItem("Custom", 14);
    scaleSelector.setSelectedId(p.scaleType + 1);
    scaleSelector.onChange = [this] {
        audioProcessor.scaleType = scaleSelector.getSelectedId() - 1;
        customScaleUI.setVisible(scaleSelector.getSelectedId() == 14);
        repaint();
        };

    addAndMakeVisible(customScaleUI);
    customScaleUI.setCustomScale(audioProcessor.customScaleIntervals);
    customScaleUI.onScaleChanged = [this](std::vector<int> newScale) {
        audioProcessor.customScaleIntervals = newScale;
        audioProcessor.oscManager.updateCustomScale(newScale);
        repaint();
        };

    // ======================================================
    // TOP BAR: MUSICAL SETTINGS (RANGE)
    // ======================================================
    addAndMakeVisible(octaveLabel); // Functioning as the Range Mode label
    octaveLabel.setText("Range:", juce::dontSendNotification);
    octaveLabel.setColour(juce::Label::textColourId, juce::Colours::white);

    addAndMakeVisible(rangeModeSelector);
    rangeModeSelector.addItem("Standard Range", 1);
    rangeModeSelector.addItem("Custom Range", 2);


    addAndMakeVisible(baseOctaveLabel);
    baseOctaveLabel.setText("Start:", juce::dontSendNotification);
    baseOctaveLabel.setColour(juce::Label::textColourId, juce::Colours::white);

    addAndMakeVisible(baseOctaveSelector);
    baseOctaveSelector.addItem("Sub (0)", 2);
    baseOctaveSelector.addItem("Low (1)", 3);
    baseOctaveSelector.addItem("Mid-Low (2)", 4);
    baseOctaveSelector.addItem("Standard (3)", 5);
    baseOctaveSelector.addItem("Mid-High (4)", 6);
    baseOctaveSelector.addItem("High (5)", 7);
    baseOctaveSelector.addItem("Very High (6)", 8);
    baseOctaveSelector.setSelectedId((p.startNote / 12) + 1, juce::dontSendNotification);
    baseOctaveSelector.onChange = [this] {
        audioProcessor.startNote = (baseOctaveSelector.getSelectedId() - 1) * 12;
        startNoteSelector.setSelectedId(audioProcessor.startNote + 1, juce::dontSendNotification);
        repaint();
        };

    addAndMakeVisible(octaveSelector);
    octaveSelector.addItem("1 Octave", 1);
    octaveSelector.addItem("2 Octaves", 2);
    octaveSelector.addItem("3 Octaves", 3);
    octaveSelector.addItem("4 Octaves", 4);
    octaveSelector.addItem("5 Octaves", 5);
    octaveSelector.addItem("6 Octaves", 6);
    octaveSelector.setSelectedId(p.octaveRange > 0 ? p.octaveRange : 2, juce::dontSendNotification);
    octaveSelector.onChange = [this] { audioProcessor.octaveRange = octaveSelector.getSelectedId(); };

    addAndMakeVisible(startNoteSelector);
    for (int i = 0; i <= 127; ++i) {
        startNoteSelector.addItem(juce::MidiMessage::getMidiNoteName(i, true, true, 3), i + 1);
    }
    startNoteSelector.setSelectedId(audioProcessor.startNote + 1, juce::dontSendNotification);
    startNoteSelector.onChange = [this] {
        audioProcessor.startNote = startNoteSelector.getSelectedId() - 1;
        baseOctaveSelector.setSelectedId((audioProcessor.startNote / 12) + 1, juce::dontSendNotification);
        repaint();
        };

    addAndMakeVisible(endNoteSelector);
    for (int i = 0; i <= 127; ++i) {
        endNoteSelector.addItem(juce::MidiMessage::getMidiNoteName(i, true, true, 3), i + 1);
    }
    endNoteSelector.setSelectedId(audioProcessor.endNote + 1, juce::dontSendNotification);
    endNoteSelector.onChange = [this] { audioProcessor.endNote = endNoteSelector.getSelectedId() - 1; };

    // Range Master Logic
    rangeModeSelector.onChange = [this] {
        bool isCustom = rangeModeSelector.getSelectedId() == 2;
        audioProcessor.currentRangeMode = isCustom ? MusicalRangeMode::SpecificNotes : MusicalRangeMode::OctaveRange;

        // Note: octaveLabel is now the main row label, so it STAYS visible!
        baseOctaveLabel.setVisible(!isCustom); // <-- ADD THIS LINE
        baseOctaveSelector.setVisible(!isCustom);
        octaveSelector.setVisible(!isCustom);
        startNoteSelector.setVisible(isCustom);
        endNoteSelector.setVisible(isCustom);

        resized();
        repaint();
        };
    rangeModeSelector.setSelectedId(audioProcessor.currentRangeMode == MusicalRangeMode::SpecificNotes ? 2 : 1, juce::NotificationType::sendNotificationSync);

    // ======================================================
    // BOTTOM BAR: PARAMETER CONTROLS
    // ======================================================
    addAndMakeVisible(xMinControl); xMinControl.slider.onValueChange = [this] { audioProcessor.minWidthThreshold = (float)xMinControl.slider.getValue(); };
    addAndMakeVisible(xMaxControl); xMaxControl.slider.onValueChange = [this] { audioProcessor.maxWidthThreshold = (float)xMaxControl.slider.getValue(); };
    addAndMakeVisible(yMinControl); yMinControl.slider.onValueChange = [this] { audioProcessor.minHeightThreshold = (float)yMinControl.slider.getValue(); };
    addAndMakeVisible(yMaxControl); yMaxControl.slider.onValueChange = [this] { audioProcessor.maxHeightThreshold = (float)yMaxControl.slider.getValue(); };
    addAndMakeVisible(zMinControl); zMinControl.slider.onValueChange = [this] { audioProcessor.minDepthThreshold = (float)zMinControl.slider.getValue(); };
    addAndMakeVisible(zMaxControl); zMaxControl.slider.onValueChange = [this] { audioProcessor.maxDepthThreshold = (float)zMaxControl.slider.getValue(); };

    addAndMakeVisible(muteButton);
    muteButton.setToggleState(audioProcessor.globalMute.load(), juce::dontSendNotification);
    muteButton.setColour(juce::ToggleButton::textColourId, juce::Colours::white); // Match the Show Notes color!

    muteButton.onClick = [this] {
        audioProcessor.globalMute.store(muteButton.getToggleState());
        };

    // ======================================================
    // OVERLAYS & PAGES
    // ======================================================
    addAndMakeVisible(calibrationOverlay);
    calibrationOverlay.setVisible(false);
    calibrationOverlay.onCancel = [this] { stopCalibration(false); };

    addAndMakeVisible(calibrateButton);
    calibrateButton.setButtonText("Calibrate");
    calibrateButton.onClick = [this] { startCalibration(); };

    addAndMakeVisible(editModeButton);
    editModeButton.setButtonText("Virtual Mouse");
    editModeButton.setClickingTogglesState(true);
    editModeButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
    editModeButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::orange);
    editModeButton.onClick = [this] {
        isEditMode = editModeButton.getToggleState();
        audioProcessor.isVirtualMouse.store(isEditMode);
        };

    addChildComponent(staticDialsPage);
    staticDialsPage.setVisible(false);
    addAndMakeVisible(staticDialsButton);
    staticDialsButton.setButtonText("Static Dials");

    // --- DELETE THESE TWO LINES ---
    // addChildComponent(settingsPage);
    // settingsPage.setVisible(false);

    // --- ADD THIS INSTEAD ---
    settingsViewport.setViewedComponent(&settingsPage, false);
    settingsViewport.setScrollBarsShown(true, false); // Vertical YES, Horizontal NO
    addChildComponent(settingsViewport);
    settingsViewport.setVisible(false);

    addAndMakeVisible(settingsButton);
    settingsButton.setButtonText("Settings");
    

    addChildComponent(chordBuilderPage);
    chordBuilderPage.setVisible(false);
    addAndMakeVisible(chordBuilderButton);
    chordBuilderButton.setButtonText("Chord Builder");

    // Unified Menu Hiding Logic
    auto hideMainMenu = [this]() {
        settingsButton.setVisible(false);
        staticDialsButton.setVisible(false);
        calibrateButton.setVisible(false);
        editModeButton.setVisible(false);
        connectionStatusLabel.setVisible(false);
        showNoteNamesButton.setVisible(false);
        rootSelector.setVisible(false);
        scaleSelector.setVisible(false);
        keyLabel.setVisible(false);
        scaleLabel.setVisible(false);
        rangeModeSelector.setVisible(false);
        baseOctaveLabel.setVisible(false); // <-- ADD THIS LINE
        octaveSelector.setVisible(false);
        octaveLabel.setVisible(false);
        startNoteSelector.setVisible(false);
        endNoteSelector.setVisible(false);
        xMinControl.setVisible(false); xMaxControl.setVisible(false);
        yMinControl.setVisible(false); yMaxControl.setVisible(false);
        zMinControl.setVisible(false); zMaxControl.setVisible(false);
        maximizeButton.setVisible(false);
        baseOctaveSelector.setVisible(false);
        customScaleUI.setVisible(false);
        chordBuilderButton.setVisible(false);
        };

    auto showMainMenu = [this]() {
        settingsButton.setVisible(true);
        staticDialsButton.setVisible(true);
        calibrateButton.setVisible(true);
        editModeButton.setVisible(true);
        connectionStatusLabel.setVisible(true);
        showNoteNamesButton.setVisible(true);
        rootSelector.setVisible(true);
        scaleSelector.setVisible(true);
        keyLabel.setVisible(true);
        scaleLabel.setVisible(true);
        rangeModeSelector.setVisible(true);
        octaveLabel.setVisible(true);
        xMinControl.setVisible(true); xMaxControl.setVisible(true);
        yMinControl.setVisible(true); yMaxControl.setVisible(true);
        zMinControl.setVisible(true); zMaxControl.setVisible(true);
        maximizeButton.setVisible(true);
        scaleSelector.onChange();
        chordBuilderButton.setVisible(true);
        rangeModeSelector.onChange(); // Recalculates Custom UI & Range drop-downs automatically!
        };

    staticDialsButton.onClick = [this, hideMainMenu] { staticDialsPage.setVisible(true); hideMainMenu(); };
    staticDialsPage.closeButton.onClick = [this, showMainMenu] { staticDialsPage.setVisible(false); showMainMenu(); };
    // Change your settings button clicks to this:
    settingsButton.onClick = [this, hideMainMenu] { settingsViewport.setVisible(true); hideMainMenu(); };
    settingsPage.closeButton.onClick = [this, showMainMenu] { settingsViewport.setVisible(false); showMainMenu(); };
    // Change this:
    chordBuilderButton.onClick = [this, hideMainMenu] { chordBuilderPage.setVisible(true); hideMainMenu(); };

    // To this:
    chordBuilderButton.onClick = [this, hideMainMenu] {
        chordBuilderPage.refreshUI(); // <-- Forces the UI to unlock and sync data!
        chordBuilderPage.setVisible(true);
        hideMainMenu();
        };
    chordBuilderPage.closeButton.onClick = [this, showMainMenu] { chordBuilderPage.setVisible(false); showMainMenu(); };
    settingsPage.onPresetLoaded = [this] {
        // 1. Force the UI dropdowns to match the loaded XML data
        rootSelector.setSelectedId(audioProcessor.rootNote + 1, juce::dontSendNotification);
        scaleSelector.setSelectedId(audioProcessor.scaleType + 1, juce::dontSendNotification);

        rangeModeSelector.setSelectedId(audioProcessor.currentRangeMode == MusicalRangeMode::SpecificNotes ? 2 : 1, juce::dontSendNotification);

        baseOctaveSelector.setSelectedId((audioProcessor.startNote / 12) + 1, juce::dontSendNotification);
        octaveSelector.setSelectedId(audioProcessor.octaveRange > 0 ? audioProcessor.octaveRange : 2, juce::dontSendNotification);

        startNoteSelector.setSelectedId(audioProcessor.startNote + 1, juce::dontSendNotification);
        endNoteSelector.setSelectedId(audioProcessor.endNote + 1, juce::dontSendNotification);

        // 2. Trigger the logic updates so visibility rules apply
        scaleSelector.onChange();
        rangeModeSelector.onChange();

        // 3. Refresh sub-menus and toggles
        settingsPage.refreshUI();
        chordBuilderPage.refreshUI(); // <-- ADD THIS LINE HERE!
        customScaleUI.setCustomScale(audioProcessor.customScaleIntervals);

        showNoteNamesButton.setToggleState(audioProcessor.showNoteNames, juce::dontSendNotification);
        muteButton.setToggleState(audioProcessor.globalMute.load(), juce::dontSendNotification);
        // Check if the loaded preset wants the window maximized, and if we aren't already:
        if (audioProcessor.isWindowMaximized.load() != isMaximized) {
            maximizeButton.triggerClick();
        }
        resized();
        repaint();
        };

    // ======================================================
    // FINAL WINDOW SETUP
    // ======================================================
    setResizable(true, true);
    setResizeLimits(800, 600, 3000, 2000);
    setOpaque(true);
    setSize(1500, 700);
    startTimerHz(60);
}

GestureInstrumentAudioProcessorEditor::~GestureInstrumentAudioProcessorEditor() {
    openGLContext.detach();
    stopTimer();
}

// ==============================================================================
// 2. MAIN RENDERING & LAYOUT (Paint & Resized)
// ==============================================================================

void GestureInstrumentAudioProcessorEditor::paint(juce::Graphics& g) {
    juce::ColourGradient bgGradient(
        juce::Colour::fromFloatRGBA(0.05f, 0.05f, 0.1f, 1.0f),
        (float)getWidth() / 2, (float)getHeight() / 2,
        juce::Colour::fromFloatRGBA(0.0f, 0.0f, 0.0f, 1.0f),
        0.0f, 0.0f, true);
    g.setGradientFill(bgGradient);
    g.fillAll();

    centerScreen = getLocalBounds().getCentre().toFloat();

    // 3D Scene Only! Everything else is handled by HUDComponents.
    draw3DGrid(g);
    draw3DHand(g, audioProcessor.leftHand, juce::Colours::cyan);
    draw3DHand(g, audioProcessor.rightHand, juce::Colours::magenta);
    drawCalibrationBox3D(g);
}

void GestureInstrumentAudioProcessorEditor::resized() {
    calibrationOverlay.setBounds(getLocalBounds());
    settingsViewport.setBounds(getLocalBounds());
    staticDialsPage.setBounds(getLocalBounds());
    virtualCursor.setBounds(getLocalBounds());
    chordBuilderPage.setBounds(getLocalBounds());

    settingsPage.setBounds(0, 0, getWidth(), 900);

    int margin = 10;
    int topBarY = margin;
    int buttonW = 120;
    int controlHeight = 30;

    // ======================================================
    // ROW 1: KEY & SCALE
    // ======================================================
    keyLabel.setBounds(margin, topBarY, 35, controlHeight);
    rootSelector.setBounds(keyLabel.getRight() + 5, topBarY, 60, controlHeight);

    scaleLabel.setBounds(rootSelector.getRight() + 20, topBarY, 45, controlHeight);
    scaleSelector.setBounds(scaleLabel.getRight() + 5, topBarY, 130, controlHeight);

    customScaleUI.setBounds((getWidth() / 2) - 200, topBarY + 40, 400, controlHeight);
    // ======================================================
    // ROW 2: RANGE TOOLS
    // ======================================================
    int row2Y = topBarY + controlHeight + 5;

    // Label is now properly placed right in front of the Range Mode
    octaveLabel.setBounds(margin, row2Y, 55, controlHeight);
    rangeModeSelector.setBounds(octaveLabel.getRight() + 5, row2Y, 140, controlHeight);

    int currentX = rangeModeSelector.getRight() + 20;

    if (audioProcessor.currentRangeMode == MusicalRangeMode::OctaveRange) {
        // Position the new label
        baseOctaveLabel.setBounds(currentX, row2Y, 40, controlHeight);

        // Push the selector to the right of the label and make it 110px wide
        baseOctaveSelector.setBounds(baseOctaveLabel.getRight() + 5, row2Y, 110, controlHeight);

        octaveSelector.setBounds(baseOctaveSelector.getRight() + 10, row2Y, 100, controlHeight);
    }
    else {
        startNoteSelector.setBounds(currentX, row2Y, 85, controlHeight);
        endNoteSelector.setBounds(startNoteSelector.getRight() + 10, row2Y, 85, controlHeight);
    }

    // ======================================================
    // TOP BUTTONS (CENTERED & RIGHT)
    // ======================================================
    int centerX = getWidth() / 2;
    settingsButton.setBounds(centerX - 255, topBarY, buttonW, 30);
    calibrateButton.setBounds(centerX - 125, topBarY, buttonW, 30);
    editModeButton.setBounds(centerX + 5, topBarY, buttonW, 30);
    staticDialsButton.setBounds(centerX + 135, topBarY, buttonW, 30);
    chordBuilderButton.setBounds(centerX + 260, topBarY, buttonW, 30);


    int rightEdge = getWidth() - margin;
    maximizeButton.setBounds(rightEdge - 100, topBarY, 100, 30);
    connectionStatusLabel.setBounds(maximizeButton.getX() - 210, topBarY, 200, 20);
    showNoteNamesButton.setBounds(rightEdge - 100, topBarY + 35, 100, 30);
    muteButton.setBounds(rightEdge - 100, showNoteNamesButton.getBottom() + 5, 100, 30);

    // ======================================================
    // MAIN 3D AREA & BOTTOM BAR
    // ======================================================
    hud.setBounds(0, 110, getWidth(), getHeight() - 240);

    auto bottomArea = getLocalBounds().removeFromBottom(120).reduced(20, 10);
    int colWidth = bottomArea.getWidth() / 3;

    auto xCol = bottomArea.removeFromLeft(colWidth).reduced(15, 0);
    auto yCol = bottomArea.removeFromLeft(colWidth).reduced(15, 0);
    auto zCol = bottomArea.removeFromLeft(colWidth).reduced(15, 0);

    int halfHeight = xCol.getHeight() / 2;
    xMinControl.setBounds(xCol.removeFromTop(halfHeight).reduced(0, 2));
    xMaxControl.setBounds(xCol.reduced(0, 2));
    yMinControl.setBounds(yCol.removeFromTop(halfHeight).reduced(0, 2));
    yMaxControl.setBounds(yCol.reduced(0, 2));
    zMinControl.setBounds(zCol.removeFromTop(halfHeight).reduced(0, 2));
    zMaxControl.setBounds(zCol.reduced(0, 2));

}

// ==============================================================================
// 3. CORE LOGIC & TIMERS
// ==============================================================================

void GestureInstrumentAudioProcessorEditor::timerCallback() {
    if (isCalibrating) {
        calibrationTimer += 1.0f / 60.0f;
        calibrationOverlay.setProgress(calibrationTimer / calibrationDuration);

        auto processHand = [&](const HandData& hand) {
            if (!hand.isPresent) return;

            if (hand.currentHandPositionX < tempMinX) tempMinX = hand.currentHandPositionX;
            if (hand.currentHandPositionX > tempMaxX) tempMaxX = hand.currentHandPositionX;

            if (hand.currentHandPositionY < tempMinY) tempMinY = hand.currentHandPositionY;
            if (hand.currentHandPositionY > tempMaxY) tempMaxY = hand.currentHandPositionY;

            if (hand.currentHandPositionZ < tempMinZ) tempMinZ = hand.currentHandPositionZ;
            if (hand.currentHandPositionZ > tempMaxZ) tempMaxZ = hand.currentHandPositionZ;

            tempMinX = juce::jmax(-350.0f, tempMinX);
            tempMaxX = juce::jmin(350.0f, tempMaxX);
            tempMinY = juce::jmax(50.0f, tempMinY);
            tempMaxY = juce::jmin(500.0f, tempMaxY);
            tempMinZ = juce::jmax(-225.0f, tempMinZ);
            tempMaxZ = juce::jmin(225.0f, tempMaxZ);
            };

        processHand(audioProcessor.leftHand);
        processHand(audioProcessor.rightHand);

        bool leftFist = audioProcessor.leftHand.isPresent && audioProcessor.leftHand.grabStrength > 0.85f;
        bool rightFist = audioProcessor.rightHand.isPresent && audioProcessor.rightHand.grabStrength > 0.85f;

        if (leftFist || rightFist) {
            stopCalibration(true);
            return;
        }

        if (calibrationTimer >= calibrationDuration) {
            stopCalibration(true);
        }
    }

    // Now reading from your atomic bool!
    if (audioProcessor.isGestureToMouseEnabled.load() && !isCalibrating) {
        bool leftFist = audioProcessor.leftHand.isPresent && audioProcessor.leftHand.grabStrength > 0.85f;
        bool rightFist = audioProcessor.rightHand.isPresent && audioProcessor.rightHand.grabStrength > 0.85f;
        bool gestureTriggered = false;

        // Now reading from your processor int!
        int mode = audioProcessor.virtualMouseGestureType;
        if (mode == 1) gestureTriggered = leftFist && rightFist;
        else if (mode == 2) gestureTriggered = rightFist;
        else if (mode == 3) gestureTriggered = leftFist;

        // Now reading from your processor float!
        float requiredHoldTime = audioProcessor.virtualMouseHoldTime;

        if (gestureTriggered && !menuGestureFired) {
            menuGestureTimer += 1.0f / 60.0f;
            if (menuGestureTimer >= requiredHoldTime) {
                editModeButton.triggerClick();
                menuGestureFired = true;
                menuGestureTimer = 0.0f;
            }
        }
        else if (!gestureTriggered) {
            menuGestureTimer = 0.0f;
            menuGestureFired = false;
        }
    }
    else {
        menuGestureTimer = 0.0f;
        menuGestureFired = false;
    }

    hud.isEditMode = isEditMode;
    hud.menuGestureTimer = menuGestureTimer;

    // Sync the HUD visual timer as well!
    hud.requiredHoldTime = audioProcessor.virtualMouseHoldTime;

    hud.isCalibrating = isCalibrating;
    hud.menuGestureFired = menuGestureFired;
    chordBuilderButton.setEnabled(audioProcessor.currentOutputMode != OutputMode::OSC_Only);

    virtualCursor.updateCursorLogic(isEditMode);
    updateConnectionStatus();
    repaint();
}

void GestureInstrumentAudioProcessorEditor::updateConnectionStatus() {
    bool connected = audioProcessor.isSensorConnected;
    if (connected) {
        connectionStatusLabel.setText("Sensor Connected", juce::dontSendNotification);
        connectionStatusLabel.setColour(juce::Label::textColourId, juce::Colours::green);
    }
    else {
        connectionStatusLabel.setText("Sensor Disconnected", juce::dontSendNotification);
        connectionStatusLabel.setColour(juce::Label::textColourId, juce::Colours::red);
    }
}

// ==============================================================================
// 4. CALIBRATION LOGIC & OVERLAY
// ==============================================================================

void GestureInstrumentAudioProcessorEditor::CalibrationOverlay::paint(juce::Graphics& g) {
    g.fillAll(juce::Colours::black.withAlpha(0.4f));
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(28.0f, juce::Font::bold));
    g.drawText("Move hands to draw out playing area.", getLocalBounds().withY(-50), juce::Justification::centred);

    auto barArea = getLocalBounds().removeFromBottom(40).reduced(100, 15);
    g.setColour(juce::Colours::darkgrey);
    g.fillRoundedRectangle(barArea.toFloat(), 5.0f);
    g.setColour(juce::Colours::green);
    g.fillRoundedRectangle(barArea.removeFromLeft(barArea.getWidth() * progress).toFloat(), 5.0f);

    g.setColour(juce::Colours::yellow);
    g.setFont(juce::Font(20.0f, juce::Font::plain));
    g.drawText("Grab gesture to confirm.", getLocalBounds().withY(20), juce::Justification::centred);
}

void GestureInstrumentAudioProcessorEditor::CalibrationOverlay::setProgress(float p) {
    progress = p;
    repaint();
}

void GestureInstrumentAudioProcessorEditor::startCalibration() {
    isCalibrating = true;
    audioProcessor.isCalibrating.store(true);
    calibrationTimer = 0.0f;
    tempMinX = 1000.0f; tempMaxX = -1000.0f;
    tempMinY = 1000.0f; tempMaxY = -1000.0f;
    tempMinZ = 1000.0f; tempMaxZ = -1000.0f;
    calibrationOverlay.setVisible(true);

    calibrationOverlay.toFront(true);
    virtualCursor.toFront(true);
}

void GestureInstrumentAudioProcessorEditor::stopCalibration(bool success) {
    isCalibrating = false;
    audioProcessor.isCalibrating.store(false);
    calibrationOverlay.setVisible(false);

    if (success) {
        audioProcessor.minWidthThreshold = juce::jlimit(-350.0f, 0.0f, tempMinX);
        audioProcessor.maxWidthThreshold = juce::jlimit(0.0f, 350.0f, tempMaxX);

        audioProcessor.minHeightThreshold = juce::jlimit(50.0f, 275.0f, tempMinY);
        audioProcessor.maxHeightThreshold = juce::jlimit(275.0f, 500.0f, tempMaxY);

        audioProcessor.minDepthThreshold = juce::jlimit(-225.0f, 0.0f, tempMinZ);
        audioProcessor.maxDepthThreshold = juce::jlimit(0.0f, 225.0f, tempMaxZ);

        yMinControl.slider.setValue(audioProcessor.minHeightThreshold, juce::dontSendNotification);
        yMaxControl.slider.setValue(audioProcessor.maxHeightThreshold, juce::dontSendNotification);
        xMinControl.slider.setValue(audioProcessor.minWidthThreshold, juce::dontSendNotification);
        xMaxControl.slider.setValue(audioProcessor.maxWidthThreshold, juce::dontSendNotification);
        zMinControl.slider.setValue(audioProcessor.minDepthThreshold, juce::dontSendNotification);
        zMaxControl.slider.setValue(audioProcessor.maxDepthThreshold, juce::dontSendNotification);
    }
}

// ==============================================================================
// 5. 3D GRAPHICS & PROJECTION
// ==============================================================================

juce::Point<float> GestureInstrumentAudioProcessorEditor::projectPoint(Point3D p) {
    float worldY = p.y - 250.0f;
    float adjustedZ = -p.z;

    float perspective = fov / (fov + adjustedZ + camDist);
    float zoom = ((float)getHeight() * 0.55f) / 500.0f;
    float finalScale = perspective * zoom * 2.5f;

    float x2d = p.x * finalScale + centerScreen.x;
    float y2d = -worldY * finalScale + centerScreen.y;

    return juce::Point<float>(x2d, y2d);
}

void GestureInstrumentAudioProcessorEditor::drawCalibrationBox3D(juce::Graphics& g) {
    if (!isCalibrating || tempMinX == 1000.0f) return;

    Point3D f1 = { tempMinX, tempMinY, tempMinZ };
    Point3D f2 = { tempMaxX, tempMinY, tempMinZ };
    Point3D f3 = { tempMaxX, tempMinY, tempMaxZ };
    Point3D f4 = { tempMinX, tempMinY, tempMaxZ };

    Point3D c1 = { tempMinX, tempMaxY, tempMinZ };
    Point3D c2 = { tempMaxX, tempMaxY, tempMinZ };
    Point3D c3 = { tempMaxX, tempMaxY, tempMaxZ };
    Point3D c4 = { tempMinX, tempMaxY, tempMaxZ };

    auto pF1 = projectPoint(f1); auto pF2 = projectPoint(f2);
    auto pF3 = projectPoint(f3); auto pF4 = projectPoint(f4);
    auto pC1 = projectPoint(c1); auto pC2 = projectPoint(c2);
    auto pC3 = projectPoint(c3); auto pC4 = projectPoint(c4);

    g.setColour(juce::Colours::cyan.withAlpha(0.8f));
    float thick = 2.0f;

    g.drawLine(juce::Line<float>(pF1, pF2), thick); g.drawLine(juce::Line<float>(pF2, pF3), thick);
    g.drawLine(juce::Line<float>(pF3, pF4), thick); g.drawLine(juce::Line<float>(pF4, pF1), thick);

    g.drawLine(juce::Line<float>(pC1, pC2), thick); g.drawLine(juce::Line<float>(pC2, pC3), thick);
    g.drawLine(juce::Line<float>(pC3, pC4), thick); g.drawLine(juce::Line<float>(pC4, pC1), thick);

    g.drawLine(juce::Line<float>(pF1, pC1), thick); g.drawLine(juce::Line<float>(pF2, pC2), thick);
    g.drawLine(juce::Line<float>(pF3, pC3), thick); g.drawLine(juce::Line<float>(pF4, pC4), thick);
}

void GestureInstrumentAudioProcessorEditor::draw3DGrid(juce::Graphics& g) {
    float minRoomX = -350.0f; float maxRoomX = 350.0f;
    float minRoomY = 50.0f;   float maxRoomY = 500.0f;
    float minRoomZ = -225.0f; float maxRoomZ = 225.0f;

    Point3D f1 = { minRoomX, minRoomY, minRoomZ };
    Point3D f2 = { maxRoomX, minRoomY, minRoomZ };
    Point3D f3 = { maxRoomX, minRoomY, maxRoomZ };
    Point3D f4 = { minRoomX, minRoomY, maxRoomZ };

    Point3D c1 = { minRoomX, maxRoomY, minRoomZ };
    Point3D c2 = { maxRoomX, maxRoomY, minRoomZ };
    Point3D c3 = { maxRoomX, maxRoomY, maxRoomZ };
    Point3D c4 = { minRoomX, maxRoomY, maxRoomZ };

    auto pF1 = projectPoint(f1); auto pF2 = projectPoint(f2);
    auto pF3 = projectPoint(f3); auto pF4 = projectPoint(f4);
    auto pC1 = projectPoint(c1); auto pC2 = projectPoint(c2);
    auto pC3 = projectPoint(c3); auto pC4 = projectPoint(c4);

    g.setColour(juce::Colours::white.withAlpha(0.08f));

    g.drawLine(juce::Line<float>(pF1, pF2)); g.drawLine(juce::Line<float>(pF2, pF3));
    g.drawLine(juce::Line<float>(pF3, pF4)); g.drawLine(juce::Line<float>(pF4, pF1));
    g.drawLine(juce::Line<float>(pC1, pC2)); g.drawLine(juce::Line<float>(pC2, pC3));
    g.drawLine(juce::Line<float>(pC3, pC4)); g.drawLine(juce::Line<float>(pC4, pC1));
    g.drawLine(juce::Line<float>(pF1, pC1)); g.drawLine(juce::Line<float>(pF2, pC2));
    g.drawLine(juce::Line<float>(pF3, pC3)); g.drawLine(juce::Line<float>(pF4, pC4));

    float minX = audioProcessor.minWidthThreshold;
    float maxX = audioProcessor.maxWidthThreshold;
    float minY = audioProcessor.minHeightThreshold;
    float maxY = audioProcessor.maxHeightThreshold;
    float minZ = audioProcessor.minDepthThreshold;
    float maxZ = audioProcessor.maxDepthThreshold;

    Point3D tF1 = { minX, minY, minZ }; Point3D tF2 = { maxX, minY, minZ };
    Point3D tF3 = { maxX, minY, maxZ }; Point3D tF4 = { minX, minY, maxZ };
    Point3D tC1 = { minX, maxY, minZ }; Point3D tC2 = { maxX, maxY, minZ };
    Point3D tC3 = { maxX, maxY, maxZ }; Point3D tC4 = { minX, maxY, maxZ };

    auto ptF1 = projectPoint(tF1); auto ptF2 = projectPoint(tF2);
    auto ptF3 = projectPoint(tF3); auto ptF4 = projectPoint(tF4);
    auto ptC1 = projectPoint(tC1); auto ptC2 = projectPoint(tC2);
    auto ptC3 = projectPoint(tC3); auto ptC4 = projectPoint(tC4);

    g.setColour(juce::Colours::yellow.withAlpha(0.6f));
    float thick = 2.5f;

    g.drawLine(juce::Line<float>(ptF1, ptF2), thick); g.drawLine(juce::Line<float>(ptF2, ptF3), thick);
    g.drawLine(juce::Line<float>(ptF3, ptF4), thick); g.drawLine(juce::Line<float>(ptF4, ptF1), thick);
    g.drawLine(juce::Line<float>(ptC1, ptC2), thick); g.drawLine(juce::Line<float>(ptC2, ptC3), thick);
    g.drawLine(juce::Line<float>(ptC3, ptC4), thick); g.drawLine(juce::Line<float>(ptC4, ptC1), thick);
    g.drawLine(juce::Line<float>(ptF1, ptC1), thick); g.drawLine(juce::Line<float>(ptF2, ptC2), thick);
    g.drawLine(juce::Line<float>(ptF3, ptC3), thick); g.drawLine(juce::Line<float>(ptF4, ptC4), thick);

    // --- DYNAMIC SPLIT X-AXIS LINE ---
    if (audioProcessor.enableSplitXAxis.load()) {
        // Find the exact middle between the left and right thresholds
        float centerX = (minX + maxX) / 2.0f;

        // Create the 4 points of the dividing wall
        Point3D divF1 = { centerX, minY, minZ };
        Point3D divF2 = { centerX, minY, maxZ };
        Point3D divC1 = { centerX, maxY, minZ };
        Point3D divC2 = { centerX, maxY, maxZ };

        // Project them to 2D
        auto pDivF1 = projectPoint(divF1); auto pDivF2 = projectPoint(divF2);
        auto pDivC1 = projectPoint(divC1); auto pDivC2 = projectPoint(divC2);

        // Draw a glowing orange line down the middle of the box
        g.setColour(juce::Colours::orange.withAlpha(0.8f));
        g.drawLine(juce::Line<float>(pDivF1, pDivF2), 2.5f); // Floor line
        g.drawLine(juce::Line<float>(pDivC1, pDivC2), 2.5f); // Ceiling line
        g.drawLine(juce::Line<float>(pDivF1, pDivC1), 2.5f); // Front vertical line
        g.drawLine(juce::Line<float>(pDivF2, pDivC2), 2.5f); // Back vertical line
    }
}

void GestureInstrumentAudioProcessorEditor::draw3DHand(juce::Graphics& g, const HandData& hand, juce::Colour baseColour) {
    if (!hand.isPresent) return;

    auto rotateLocal = [&](float x, float y, float z) -> Point3D {
        float dy = y - hand.currentHandPositionY;
        float dz = z - hand.currentHandPositionZ;
        float angle = juce::MathConstants<float>::pi * 0.20f;
        float cosA = std::cos(angle);
        float sinA = std::sin(angle);
        float rotY = dy * cosA - dz * sinA;
        float rotZ = dy * sinA + dz * cosA;
        return { x, hand.currentHandPositionY + rotY, hand.currentHandPositionZ + rotZ };
        };

    auto clampPoint = [&](Point3D p) -> Point3D {
        if (isCalibrating) return p;
        return {
            juce::jlimit(audioProcessor.minWidthThreshold, audioProcessor.maxWidthThreshold, p.x),
            juce::jlimit(audioProcessor.minHeightThreshold, audioProcessor.maxHeightThreshold, p.y),
            juce::jlimit(audioProcessor.minDepthThreshold, audioProcessor.maxDepthThreshold, p.z)
        };
        };

    Point3D palm3D = clampPoint({ hand.currentHandPositionX, hand.currentHandPositionY, hand.currentHandPositionZ });
    auto palm2D = projectPoint(palm3D);
    float palmDepth = juce::jmap(palm3D.z, -200.0f, 200.0f, 0.6f, 1.4f);

    // --- NEW: DYNAMIC 3D FLOOR SHADOW ---
    // 1. Cast a point straight down to the physical "floor"
    // --- NEW: DYNAMIC 3D FLOOR SHADOW ---
    if (audioProcessor.showFloorShadow) {
        Point3D shadow3D = { palm3D.x, audioProcessor.minHeightThreshold, palm3D.z };
        auto shadow2D = projectPoint(shadow3D);

        float heightRatio = juce::jmap(palm3D.y, audioProcessor.minHeightThreshold, audioProcessor.maxHeightThreshold, 0.0f, 1.0f);
        heightRatio = juce::jlimit(0.0f, 1.0f, heightRatio);

        float shadowScale = juce::jmap(shadow3D.z, -200.0f, 200.0f, 0.6f, 1.4f);
        float shadowW = (50.0f + (heightRatio * 70.0f)) * shadowScale;
        float shadowH = shadowW * 0.35f;
        float shadowAlpha = 0.5f - (heightRatio * 0.4f);

        // 1. Create a 2D Path representing the exact shape of your physical floor
        juce::Path floorPath;
        floorPath.startNewSubPath(projectPoint({ audioProcessor.minWidthThreshold, audioProcessor.minHeightThreshold, audioProcessor.minDepthThreshold }));
        floorPath.lineTo(projectPoint({ audioProcessor.maxWidthThreshold, audioProcessor.minHeightThreshold, audioProcessor.minDepthThreshold }));
        floorPath.lineTo(projectPoint({ audioProcessor.maxWidthThreshold, audioProcessor.minHeightThreshold, audioProcessor.maxDepthThreshold }));
        floorPath.lineTo(projectPoint({ audioProcessor.minWidthThreshold, audioProcessor.minHeightThreshold, audioProcessor.maxDepthThreshold }));
        floorPath.closeSubPath();

        // 2. Save the graphics state, clip to the floor, draw the shadow, and restore!
        g.saveState();
        g.reduceClipRegion(floorPath); // This acts as our invisible stencil!

        g.setColour(baseColour.darker(0.8f).withAlpha(shadowAlpha));
        g.fillEllipse(shadow2D.x - (shadowW / 2.0f), shadow2D.y - (shadowH / 2.0f), shadowW, shadowH);

        g.restoreState(); // Removes the stencil so the rest of the hand draws normally
    }
    // ------------------------------------

    // --- NEW: DYNAMIC 3D BACK-WALL SHADOW ---
    // 1. Cast a point straight back to the physical "back wall" (minDepthThreshold)
    if (audioProcessor.showWallShadow) {
        Point3D wallShadow3D = { palm3D.x, palm3D.y, audioProcessor.minDepthThreshold };
        auto wallShadow2D = projectPoint(wallShadow3D);

        // 2. Calculate how far the hand is from the back wall (0.0 to 1.0)
        float depthRatio = juce::jmap(palm3D.z, audioProcessor.minDepthThreshold, audioProcessor.maxDepthThreshold, 0.0f, 1.0f);
        depthRatio = juce::jlimit(0.0f, 1.0f, depthRatio);

        // 3. Shadow gets larger and more transparent the further you pull your hand away from the wall
        float backWallScale = juce::jmap(audioProcessor.minDepthThreshold, -200.0f, 200.0f, 0.6f, 1.4f);
        float wallShadowSize = (50.0f + (depthRatio * 70.0f)) * backWallScale;
        float wallShadowAlpha = 0.4f - (depthRatio * 0.35f); // Fades from 0.4 opacity to near invisible

        // 4. Create a 2D Path representing the exact shape of the BACK WALL
        juce::Path wallPath;
        wallPath.startNewSubPath(projectPoint({ audioProcessor.minWidthThreshold, audioProcessor.minHeightThreshold, audioProcessor.minDepthThreshold }));
        wallPath.lineTo(projectPoint({ audioProcessor.maxWidthThreshold, audioProcessor.minHeightThreshold, audioProcessor.minDepthThreshold }));
        wallPath.lineTo(projectPoint({ audioProcessor.maxWidthThreshold, audioProcessor.maxHeightThreshold, audioProcessor.minDepthThreshold }));
        wallPath.lineTo(projectPoint({ audioProcessor.minWidthThreshold, audioProcessor.maxHeightThreshold, audioProcessor.minDepthThreshold }));
        wallPath.closeSubPath();

        // 5. Save state, clip to the back wall, draw shadow, and restore!
        g.saveState();
        g.reduceClipRegion(wallPath);

        g.setColour(baseColour.darker(0.8f).withAlpha(wallShadowAlpha));

        // We don't squash this ellipse because we are looking directly at the back wall!
        g.fillEllipse(wallShadow2D.x - (wallShadowSize / 2.0f), wallShadow2D.y - (wallShadowSize / 2.0f), wallShadowSize, wallShadowSize);

        g.restoreState();
    }


    // ------------------------------------

    if (hand.grabStrength > 0.1f) {
        float grabIntensity = hand.grabStrength;
        juce::Colour grabCol = baseColour.interpolatedWith(juce::Colours::orange, grabIntensity);

        g.setColour(grabCol.withAlpha(0.3f * grabIntensity));
        float radius = (40.0f * palmDepth) * (2.0f - grabIntensity);
        g.fillEllipse(palm2D.x - radius / 2, palm2D.y - radius / 2, radius, radius);

        g.setColour(grabCol);
        g.drawEllipse(palm2D.x - radius / 2, palm2D.y - radius / 2, radius, radius, 2.0f);
    }

    for (int i = 0; i < 5; ++i) {
        const auto& f = hand.fingers[i];

        float alpha = f.isExtended ? 0.9f : 0.2f;
        juce::Colour boneCol = baseColour.withAlpha(alpha);

        auto knuckle = projectPoint(clampPoint(rotateLocal(f.knuckleX, f.knuckleY, f.knuckleZ)));
        auto joint1 = projectPoint(clampPoint(rotateLocal(f.joint2X, f.joint2Y, f.joint2Z)));
        auto joint2 = projectPoint(clampPoint(rotateLocal(f.joint1X, f.joint1Y, f.joint1Z)));
        auto tip = projectPoint(clampPoint(rotateLocal(f.tipX, f.tipY, f.tipZ)));

        g.setColour(baseColour.withAlpha(0.2f));
        g.drawLine(juce::Line<float>(palm2D, knuckle), 2.0f * palmDepth);

        g.setColour(boneCol);
        g.drawLine(juce::Line<float>(knuckle, joint1), 3.0f * palmDepth);
        g.drawLine(juce::Line<float>(joint1, joint2), 2.5f * palmDepth);
        g.drawLine(juce::Line<float>(joint2, tip), 2.0f * palmDepth);

        float jointSize = 6.0f * palmDepth;
        if (f.isExtended) {
            g.setColour(juce::Colours::white.withAlpha(0.8f));
            g.fillEllipse(tip.x - jointSize / 2, tip.y - jointSize / 2, jointSize, jointSize);
        }

        g.setColour(baseColour.withAlpha(0.4f));
        g.fillEllipse(knuckle.x - jointSize, knuckle.y - jointSize, jointSize * 2, jointSize * 2);
    }

    if (hand.isPinching && hand.fingers[0].isExtended == false) {
        Point3D pinchCenter = {
            (hand.fingers[0].tipX + hand.fingers[1].tipX) / 2.0f,
            (hand.fingers[0].tipY + hand.fingers[1].tipY) / 2.0f,
            (hand.fingers[0].tipZ + hand.fingers[1].tipZ) / 2.0f
        };

        auto pinch2D = projectPoint(clampPoint(rotateLocal(pinchCenter.x, pinchCenter.y, pinchCenter.z)));

        g.setColour(juce::Colours::yellow);
        float sparkSize = 15.0f * palmDepth;
        g.fillEllipse(pinch2D.x - sparkSize / 2, pinch2D.y - sparkSize / 2, sparkSize, sparkSize);

        g.drawLine(pinch2D.x - sparkSize, pinch2D.y, pinch2D.x + sparkSize, pinch2D.y, 2.0f);
        g.drawLine(pinch2D.x, pinch2D.y - sparkSize, pinch2D.x, pinch2D.y + sparkSize, 2.0f);
    }
}

// ==============================================================================
// 6. INPUT HANDLING
// ==============================================================================

bool GestureInstrumentAudioProcessorEditor::keyPressed(const juce::KeyPress& key) {
    if (key == juce::KeyPress::spaceKey) {
        muteButton.triggerClick(); // <-- REMOVED .button
        return true;
    }
    if (key == juce::KeyPress::escapeKey) {
        if (isCalibrating) {
            stopCalibration(false);
            return true;
        }
    }
    return false;
}