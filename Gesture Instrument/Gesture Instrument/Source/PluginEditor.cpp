#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "UI/HUDComponents.h"

GestureInstrumentAudioProcessorEditor::GestureInstrumentAudioProcessorEditor(GestureInstrumentAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), settingsPage(p), hud(p), virtualCursor(p),
    xMinControl("X Min", -400.0f, 0.0f, p.minWidthThreshold),
    xMaxControl("X Max", 0.0f, 400.0f, p.maxWidthThreshold),
    yMinControl("Y Min", 0.0f, 250.0f, p.minHeightThreshold),
    yMaxControl("Y Max", 250.0f, 600.0f, p.maxHeightThreshold),
    zMinControl("Z Front", -300.0f, 0.0f, p.minDepthThreshold),
    zMaxControl("Z Back", 0.0f, 300.0f, p.maxDepthThreshold)
{
    // ... rest of constructor
    addAndMakeVisible(editModeButton);
    editModeButton.setButtonText("Virtual Mouse");
    editModeButton.setClickingTogglesState(true);
    editModeButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
    editModeButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::orange);

    editModeButton.onClick = [this] {
        isEditMode = editModeButton.getToggleState();
        // Mute audio if we are in Edit Mode OR Calibrating
        audioProcessor.muteOutput.store(isEditMode || isCalibrating);
        };


    addAndMakeVisible(hud);

    // In Constructor
    addAndMakeVisible(calibrationOverlay);
    calibrationOverlay.setVisible(false);

    addAndMakeVisible(calibrateButton);
    calibrateButton.onClick = [this] { startCalibration(); };

    // If you want a button to start it, add one to your UI or SettingsPage:
    // calibrateButton.onClick = [this] { startCalibration(); };

    // Output Mode
    addAndMakeVisible(modeSelector);
    modeSelector.addItem("OSC", 1);
    modeSelector.addItem("MIDI", 2);

    switch (audioProcessor.currentOutputMode) {
    case OutputMode::OSC_Only:  modeSelector.setSelectedId(1); break;
    case OutputMode::MIDI_Only: modeSelector.setSelectedId(2); break;
    }
    modeSelector.addListener(this);

    addAndMakeVisible(modeLabel);
    modeLabel.setText("Output Mode:", juce::dontSendNotification);
    modeLabel.attachToComponent(&modeSelector, true);

    // Connection Status
    addAndMakeVisible(connectionStatusLabel);
    connectionStatusLabel.setText("Checking Sensor...", juce::dontSendNotification);
    connectionStatusLabel.setJustificationType(juce::Justification::centredRight);
    connectionStatusLabel.setColour(juce::Label::textColourId, juce::Colours::grey);

    

    // Setup Callbacks
    addAndMakeVisible(xMinControl);
    xMinControl.slider.onValueChange = [this] { audioProcessor.minWidthThreshold= (float)xMinControl.slider.getValue(); };
    
    addAndMakeVisible(xMaxControl);
    xMaxControl.slider.onValueChange = [this] { audioProcessor.maxWidthThreshold = (float)xMaxControl.slider.getValue(); };

    addAndMakeVisible(yMinControl);
    yMinControl.slider.onValueChange = [this] { audioProcessor.minHeightThreshold = (float)yMinControl.slider.getValue(); };

    addAndMakeVisible(yMaxControl);
    yMaxControl.slider.onValueChange = [this] { audioProcessor.maxHeightThreshold = (float)yMaxControl.slider.getValue(); };

    addAndMakeVisible(zMinControl);
    zMinControl.slider.onValueChange = [this] { audioProcessor.minDepthThreshold = (float)zMinControl.slider.getValue(); };

    addAndMakeVisible(zMaxControl);
    zMaxControl.slider.onValueChange = [this] { audioProcessor.maxDepthThreshold = (float)zMaxControl.slider.getValue(); };

    // Settings Page+Button
    addAndMakeVisible(settingsButton);

    settingsButton.onClick = [this] {
        settingsPage.setVisible(true);
        settingsButton.setVisible(false);
        calibrateButton.setVisible(false); // Hide calibrate button

        // Hide top bar elements
        modeSelector.setVisible(false);
        modeLabel.setVisible(false);
        connectionStatusLabel.setVisible(false);

        rootSelector.setVisible(false);
        scaleSelector.setVisible(false);
        octaveSelector.setVisible(false);
        octaveLabel.setVisible(false);
        showNoteNamesButton.setVisible(false);

        xMinControl.setVisible(false);
        xMaxControl.setVisible(false);
        yMinControl.setVisible(false);
        yMaxControl.setVisible(false);
        zMinControl.setVisible(false);
        zMaxControl.setVisible(false);
        };

    addChildComponent(settingsPage);
    settingsPage.setVisible(false);

    settingsPage.closeButton.onClick = [this] {
        settingsPage.setVisible(false);
        settingsButton.setVisible(true);
        calibrateButton.setVisible(true); // Show calibrate button again

        // Show top bar elements
        modeSelector.setVisible(true);
        modeLabel.setVisible(true);
        connectionStatusLabel.setVisible(true);

        rootSelector.setVisible(true);
        scaleSelector.setVisible(true);
        octaveSelector.setVisible(true);
        octaveLabel.setVisible(true);
        showNoteNamesButton.setVisible(true);

        xMinControl.setVisible(true);
        xMaxControl.setVisible(true);
        yMinControl.setVisible(true);
        yMaxControl.setVisible(true);
        zMinControl.setVisible(true);
        zMaxControl.setVisible(true);
        };

    // Root Note Selector
    addAndMakeVisible(rootSelector);
    rootSelector.addItemList({ "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" }, 1);
    rootSelector.setSelectedId(p.rootNote + 1);
    rootSelector.onChange = [this] { audioProcessor.rootNote = rootSelector.getSelectedId() - 1; };

    // Scale Type Selector
    addAndMakeVisible(scaleSelector);
    scaleSelector.addItem("Chromatic", 1);
    scaleSelector.addItem("Major", 2);
    scaleSelector.addItem("Minor", 3);
    scaleSelector.addItem("Pentatonic", 4);
    scaleSelector.setSelectedId(p.scaleType + 1);
    scaleSelector.onChange = [this] { audioProcessor.scaleType = scaleSelector.getSelectedId() - 1; };

    // Label
    addAndMakeVisible(scaleLabel);
    scaleLabel.setText("Key / Scale:", juce::dontSendNotification);
    scaleLabel.attachToComponent(&rootSelector, true);
    scaleLabel.setColour(juce::Label::textColourId, juce::Colours::white);

    // Octave setup
    addAndMakeVisible(octaveSelector);
    octaveSelector.addItem("1 Octave", 1);
    octaveSelector.addItem("2 Octaves", 2);
    octaveSelector.addItem("3 Octaves", 3);
    octaveSelector.addItem("4 Octaves", 4);

    // Default to 2 if not set
    octaveSelector.setSelectedId(p.octaveRange > 0 ? p.octaveRange : 2, juce::dontSendNotification);

    octaveSelector.onChange = [this] {
        audioProcessor.octaveRange = octaveSelector.getSelectedId();
        };

    addAndMakeVisible(octaveLabel);
    octaveLabel.setText("Range:", juce::dontSendNotification);
    octaveLabel.attachToComponent(&octaveSelector, true);
    octaveLabel.setColour(juce::Label::textColourId, juce::Colours::white);

    addAndMakeVisible(showNoteNamesButton);
    showNoteNamesButton.setToggleState(false, juce::dontSendNotification); 
    showNoteNamesButton.setColour(juce::ToggleButton::textColourId, juce::Colours::white);

    showNoteNamesButton.onClick = [this] { repaint(); };

    // Graphics Setup
    setResizable(true, true);
    setResizeLimits(800, 600, 3000, 2000);
    setOpaque(true);

    openGLContext.attachTo(*this);
    openGLContext.setContinuousRepainting(false); // control the paint timing

    setSize(1500, 700);
    startTimerHz(60);

    addAndMakeVisible(virtualCursor);

    // Add to your constructor block:
    addAndMakeVisible(enableGestureSwitchButton);
    enableGestureSwitchButton.setToggleState(true, juce::dontSendNotification);
    enableGestureSwitchButton.setColour(juce::ToggleButton::textColourId, juce::Colours::white);

    addAndMakeVisible(gestureTimerSlider);
    gestureTimerSlider.setRange(0.5, 5.0, 0.1); // 0.5 seconds to 5 seconds
    gestureTimerSlider.setValue(1.5);
    gestureTimerSlider.setTextValueSuffix("s");

    addAndMakeVisible(gestureTypeSelector);
    gestureTypeSelector.addItem("Both Fists", 1);
    gestureTypeSelector.addItem("Right Fist", 2);
    gestureTypeSelector.addItem("Left Fist", 3);
    gestureTypeSelector.setSelectedId(1);
}

GestureInstrumentAudioProcessorEditor::~GestureInstrumentAudioProcessorEditor() {
    openGLContext.detach();
    stopTimer();
}

// ==============================================================================
// 3D Projection maths
juce::Point<float> GestureInstrumentAudioProcessorEditor::projectPoint(Point3D p) {
    float worldY = p.y - 250.0f;

    float adjustedZ = -p.z;

    float perspective = fov / (fov + adjustedZ + camDist);
    float zoom = ((float)getHeight() * 0.7f) / 500.0f;
    float finalScale = perspective * zoom * 2.5f;

    float x2d = p.x * finalScale + centerScreen.x;

    float y2d = -worldY * finalScale + centerScreen.y;

    return juce::Point<float>(x2d, y2d);
}

void GestureInstrumentAudioProcessorEditor::paint(juce::Graphics& g) {
    juce::ColourGradient bgGradient(
        juce::Colour::fromFloatRGBA(0.05f, 0.05f, 0.1f, 1.0f),
        (float)getWidth() / 2, (float)getHeight() / 2,
        juce::Colour::fromFloatRGBA(0.0f, 0.0f, 0.0f, 1.0f),
        0.0f, 0.0f, true);
    g.setGradientFill(bgGradient);
    g.fillAll();

    centerScreen = getLocalBounds().getCentre().toFloat();

    draw3DGrid(g);
    draw3DHand(g, audioProcessor.leftHand, juce::Colours::cyan);
    draw3DHand(g, audioProcessor.rightHand, juce::Colours::magenta);

    drawCalibrationBox3D(g);

    // --- DRAW GESTURE LOADING RING ---
    if (menuGestureTimer > 0.0f && !menuGestureFired && !isCalibrating) {
        float requiredTime = (float)gestureTimerSlider.getValue();
        float progress = menuGestureTimer / requiredTime;

        g.setColour(juce::Colours::orange);
        g.setFont(18.0f);
        g.drawText(isEditMode ? "Closing Virtual Mouse..." : "Opening Virtual Mouse...",
            (int)centerScreen.x - 150, (int)centerScreen.y - 60, 300, 30, juce::Justification::centred);

        float radius = 40.0f;
        juce::Path p;
        p.addCentredArc(centerScreen.x, centerScreen.y, radius, radius, 0.0f, 0.0f, juce::MathConstants<float>::twoPi * progress, true);
        g.strokePath(p, juce::PathStrokeType(4.0f));
    }
    
}

void GestureInstrumentAudioProcessorEditor::draw3DGrid(juce::Graphics& g) {
    // 1. Draw the static background "Room" (Faint white)
    float roomW = 400.0f;
    float roomH = 600.0f;
    float roomD = 300.0f;

    Point3D f1 = { -roomW, 0, -roomD }; Point3D f2 = { roomW, 0, -roomD };
    Point3D f3 = { roomW, 0, roomD };   Point3D f4 = { -roomW, 0, roomD };
    Point3D c1 = { -roomW, roomH, -roomD }; Point3D c2 = { roomW, roomH, -roomD };
    Point3D c3 = { roomW, roomH, roomD };   Point3D c4 = { -roomW, roomH, roomD };

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

    // 2. Draw the True Calibrated "Active Threshold" Box (Yellow)
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
}

void GestureInstrumentAudioProcessorEditor::draw3DHand(juce::Graphics& g, const HandData& hand, juce::Colour baseColour) {
    if (!hand.isPresent) return;

    // Project palm
    Point3D palm3D = { hand.currentHandPositionX, hand.currentHandPositionY, hand.currentHandPositionZ };
    auto palm2D = projectPoint(palm3D);

    float palmDepth = juce::jmap(hand.currentHandPositionZ, -200.0f, 200.0f, 0.6f, 1.4f);

    // Fist highlighter
    if (hand.grabStrength > 0.1f) {
        float grabIntensity = hand.grabStrength; // 0.0 to 1.0
        juce::Colour grabCol = baseColour.interpolatedWith(juce::Colours::orange, grabIntensity);

        g.setColour(grabCol.withAlpha(0.3f * grabIntensity));
        float radius = (40.0f * palmDepth) * (2.0f - grabIntensity);
        g.fillEllipse(palm2D.x - radius / 2, palm2D.y - radius / 2, radius, radius);

        g.setColour(grabCol);
        g.drawEllipse(palm2D.x - radius / 2, palm2D.y - radius / 2, radius, radius, 2.0f);
    }

    //Draw fingers
    for (int i = 0; i < 5; ++i) {
        const auto& f = hand.fingers[i];

        // Extention highlights
        float alpha = f.isExtended ? 0.9f : 0.2f;
        juce::Colour boneCol = baseColour.withAlpha(alpha);

        // Direct projection (No flippinggg)
        auto knuckle = projectPoint({ f.knuckleX, f.knuckleY, f.knuckleZ });
        auto joint1 = projectPoint({ f.joint2X,  f.joint2Y,  f.joint2Z }); 
        auto joint2 = projectPoint({ f.joint1X,  f.joint1Y,  f.joint1Z }); 
        auto tip = projectPoint({ f.tipX,     f.tipY,     f.tipZ });

        // Palm connection lines
        g.setColour(baseColour.withAlpha(0.2f));
        g.drawLine(juce::Line<float>(palm2D, knuckle), 2.0f * palmDepth);

        // Bones
        g.setColour(boneCol);
        g.drawLine(juce::Line<float>(knuckle, joint1), 3.0f * palmDepth);
        g.drawLine(juce::Line<float>(joint1, joint2), 2.5f * palmDepth);
        g.drawLine(juce::Line<float>(joint2, tip), 2.0f * palmDepth);

        // Joints
        float jointSize = 6.0f * palmDepth;
        if (f.isExtended) {
            g.setColour(juce::Colours::white.withAlpha(0.8f));
            g.fillEllipse(tip.x - jointSize / 2, tip.y - jointSize / 2, jointSize, jointSize);
        }

        // Knuckles
        g.setColour(baseColour.withAlpha(0.4f));
        g.fillEllipse(knuckle.x - jointSize, knuckle.y - jointSize, jointSize * 2, jointSize * 2);
    }

    // Pinch highlighter
    if (hand.isPinching && hand.fingers[0].isExtended == false) {
        Point3D thumbTip = { hand.fingers[0].tipX, hand.fingers[0].tipY, hand.fingers[0].tipZ };
        Point3D indexTip = { hand.fingers[1].tipX, hand.fingers[1].tipY, hand.fingers[1].tipZ };

        // calculate point between 
        Point3D pinchCenter = {
            (thumbTip.x + indexTip.x) / 2.0f,
            (thumbTip.y + indexTip.y) / 2.0f,
            (thumbTip.z + indexTip.z) / 2.0f
        };

        auto pinch2D = projectPoint(pinchCenter);

        // Draw the object
        g.setColour(juce::Colours::yellow);
        float sparkSize = 15.0f * palmDepth;
        g.fillEllipse(pinch2D.x - sparkSize / 2, pinch2D.y - sparkSize / 2, sparkSize, sparkSize);

        // Crosshair
        g.drawLine(pinch2D.x - sparkSize, pinch2D.y, pinch2D.x + sparkSize, pinch2D.y, 2.0f);
        g.drawLine(pinch2D.x, pinch2D.y - sparkSize, pinch2D.x, pinch2D.y + sparkSize, 2.0f);
    }
}


void GestureInstrumentAudioProcessorEditor::resized() {


    calibrationOverlay.setBounds(getLocalBounds());

    // Put it right next to the Calibrate button
    // --- ADD THIS LINE BACK IN ---
    settingsPage.setBounds(getLocalBounds());

    int margin = 10;
    int topBarY = margin;

    int buttonW = 120;
    int centerX = getLocalBounds().getCentreX();

    // Add this near the top of your resized() function:
    enableGestureSwitchButton.setBounds(margin, topBarY + 50, 150, 20);
    gestureTypeSelector.setBounds(margin, topBarY + 75, 100, 20);
    gestureTimerSlider.setBounds(margin + 110, topBarY + 75, 120, 20);


    settingsButton.setBounds(centerX - buttonW - 5, margin, buttonW, 30);
    virtualCursor.setBounds(getLocalBounds());
    calibrateButton.setBounds(centerX + 5, margin, buttonW, 30);
    editModeButton.setBounds(calibrateButton.getRight() + 5, margin, buttonW, 30);


    connectionStatusLabel.setBounds(getWidth() - 200 - margin, topBarY, 200, 20);
    modeSelector.setBounds(getWidth() - 160 - margin, topBarY + 30, 150, 30);
    showNoteNamesButton.setBounds(getWidth() - 160, getHeight() - 40, 150, 30);

    int controlHeight = 30;
    rootSelector.setBounds(margin + 80, topBarY + 10, 60, controlHeight);
    scaleSelector.setBounds(rootSelector.getRight() + 10, topBarY + 10, 120, controlHeight);
    octaveSelector.setBounds(scaleSelector.getRight() + 60, topBarY + 10, 100, controlHeight);

    hud.setBounds(0, 100, getWidth(), 250);



    auto bottomArea = getLocalBounds().removeFromBottom(90).reduced(20, 0);

    settingsButton.setBounds(bottomArea.removeFromLeft(100).reduced(0, 30));

    showNoteNamesButton.setBounds(bottomArea.removeFromRight(120).reduced(0, 30));

    int numSliders = 6;
    int sliderWidth = bottomArea.getWidth() / numSliders;

    // X Group
    xMinControl.setBounds(bottomArea.removeFromLeft(sliderWidth).reduced(5, 10));
    xMaxControl.setBounds(bottomArea.removeFromLeft(sliderWidth).reduced(5, 10));

    // Y Group
    yMinControl.setBounds(bottomArea.removeFromLeft(sliderWidth).reduced(5, 10));
    yMaxControl.setBounds(bottomArea.removeFromLeft(sliderWidth).reduced(5, 10));

    // Z Group
    zMinControl.setBounds(bottomArea.removeFromLeft(sliderWidth).reduced(5, 10));
    zMaxControl.setBounds(bottomArea.removeFromLeft(sliderWidth).reduced(5, 10));
}

void GestureInstrumentAudioProcessorEditor::timerCallback() {
    updateConnectionStatus();

    if (isCalibrating) {
        calibrationTimer += 1.0f / 60.0f;
        calibrationOverlay.setProgress(calibrationTimer / calibrationDuration);

        auto processHand = [&](const HandData& hand) {
            if (!hand.isPresent) return;
            // X Axis
            if (hand.currentHandPositionX < tempMinX) tempMinX = hand.currentHandPositionX;
            if (hand.currentHandPositionX > tempMaxX) tempMaxX = hand.currentHandPositionX;
            // Y Axis
            if (hand.currentHandPositionY < tempMinY) tempMinY = hand.currentHandPositionY;
            if (hand.currentHandPositionY > tempMaxY) tempMaxY = hand.currentHandPositionY;
            // Z Axis
            if (hand.currentHandPositionZ < tempMinZ) tempMinZ = hand.currentHandPositionZ;
            if (hand.currentHandPositionZ > tempMaxZ) tempMaxZ = hand.currentHandPositionZ;
            };

        processHand(audioProcessor.leftHand);
        processHand(audioProcessor.rightHand);

        // --- FIST TO STOP LOGIC ---
        bool leftFist = audioProcessor.leftHand.isPresent && audioProcessor.leftHand.grabStrength > 0.85f;
        bool rightFist = audioProcessor.rightHand.isPresent && audioProcessor.rightHand.grabStrength > 0.85f;

        if (leftFist || rightFist) {
            stopCalibration(true);
            return; // Exit the loop early
        }

        // Standard timeout
        if (calibrationTimer >= calibrationDuration) {
            stopCalibration(true);
        }
    }

    // --- GESTURE TO TOGGLE VIRTUAL MOUSE ---
    if (enableGestureSwitchButton.getToggleState() && !isCalibrating) {
        bool leftFist = audioProcessor.leftHand.isPresent && audioProcessor.leftHand.grabStrength > 0.85f;
        bool rightFist = audioProcessor.rightHand.isPresent && audioProcessor.rightHand.grabStrength > 0.85f;

        bool gestureTriggered = false;
        int mode = gestureTypeSelector.getSelectedId();

        if (mode == 1) gestureTriggered = leftFist && rightFist; // Both Hands
        else if (mode == 2) gestureTriggered = rightFist;        // Right Only
        else if (mode == 3) gestureTriggered = leftFist;         // Left Only

        float requiredHoldTime = (float)gestureTimerSlider.getValue();

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
        // Reset if the user turns the feature off entirely
        menuGestureTimer = 0.0f;
        menuGestureFired = false;
    }

    virtualCursor.updateCursorLogic(isEditMode);
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

void GestureInstrumentAudioProcessorEditor::comboBoxChanged(juce::ComboBox* box) {
    if (box == &modeSelector) {
        int id = modeSelector.getSelectedId();
        if (id == 1) audioProcessor.currentOutputMode = OutputMode::OSC_Only;
        if (id == 2) audioProcessor.currentOutputMode = OutputMode::MIDI_Only;
    }
}


void GestureInstrumentAudioProcessorEditor::CalibrationOverlay::paint(juce::Graphics& g) {
    g.fillAll(juce::Colours::black.withAlpha(0.7f)); // Slightly darker for readability

    // Draw Instructions
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(28.0f, juce::Font::bold));
    g.drawText("Move hands to the furthest edges of your reach.",
        getLocalBounds().withY(-50), juce::Justification::centred);

    g.setColour(juce::Colours::yellow);
    g.setFont(juce::Font(20.0f, juce::Font::plain));
    g.drawText("Make a FIST to lock in bounds and finish early.",
        getLocalBounds().withY(20), juce::Justification::centred);

    // Progress Bar at the bottom
    auto barArea = getLocalBounds().removeFromBottom(40).reduced(100, 15);
    g.setColour(juce::Colours::darkgrey);
    g.fillRoundedRectangle(barArea.toFloat(), 5.0f);
    g.setColour(juce::Colours::green);
    g.fillRoundedRectangle(barArea.removeFromLeft(barArea.getWidth() * progress).toFloat(), 5.0f);


}







void GestureInstrumentAudioProcessorEditor::CalibrationOverlay::setProgress(float p) {
    progress = p;
}

void GestureInstrumentAudioProcessorEditor::startCalibration() {
    isCalibrating = true;
    audioProcessor.muteOutput.store(true); // Mute immediately

    calibrationTimer = 0.0f;
    tempMinX = 1000.0f; tempMaxX = -1000.0f;
   

    tempMinY = 1000.0f; tempMaxY = -1000.0f;
    tempMinZ = 1000.0f; tempMaxZ = -1000.0f;

    calibrationOverlay.setVisible(true);
}

void GestureInstrumentAudioProcessorEditor::stopCalibration(bool success) {
    isCalibrating = false;

    audioProcessor.muteOutput.store(isEditMode);

    calibrationOverlay.setVisible(false);

    if (success) {
        // Push ALL true asymmetric bounds to the processor
        audioProcessor.minHeightThreshold = tempMinY;
        audioProcessor.maxHeightThreshold = tempMaxY;

        audioProcessor.minWidthThreshold = tempMinX;
        audioProcessor.maxWidthThreshold = tempMaxX;

        audioProcessor.minDepthThreshold = tempMinZ;
        audioProcessor.maxDepthThreshold = tempMaxZ;

        // Update all 6 sliders
        yMinControl.slider.setValue(tempMinY, juce::dontSendNotification);
        yMaxControl.slider.setValue(tempMaxY, juce::dontSendNotification);
        xMinControl.slider.setValue(tempMinX, juce::dontSendNotification);
        xMaxControl.slider.setValue(tempMaxX, juce::dontSendNotification);
        zMinControl.slider.setValue(tempMinZ, juce::dontSendNotification);
        zMaxControl.slider.setValue(tempMaxZ, juce::dontSendNotification);
    }
}

void GestureInstrumentAudioProcessorEditor::drawCalibrationBox3D(juce::Graphics& g) {
    if (!isCalibrating) return;

    // Safety check: Don't draw if hands haven't set bounds yet
    if (tempMinX == 1000.0f) return;

    // Define the 8 corners of the expanding calibration box
    Point3D f1 = { tempMinX, tempMinY, tempMinZ }; // Bottom-Left-Back
    Point3D f2 = { tempMaxX, tempMinY, tempMinZ }; // Bottom-Right-Back
    Point3D f3 = { tempMaxX, tempMinY, tempMaxZ }; // Bottom-Right-Front
    Point3D f4 = { tempMinX, tempMinY, tempMaxZ }; // Bottom-Left-Front

    Point3D c1 = { tempMinX, tempMaxY, tempMinZ }; // Top-Left-Back
    Point3D c2 = { tempMaxX, tempMaxY, tempMinZ }; // Top-Right-Back
    Point3D c3 = { tempMaxX, tempMaxY, tempMaxZ }; // Top-Right-Front
    Point3D c4 = { tempMinX, tempMaxY, tempMaxZ }; // Top-Left-Front

    // Project to 2D screen space
    auto pF1 = projectPoint(f1); auto pF2 = projectPoint(f2);
    auto pF3 = projectPoint(f3); auto pF4 = projectPoint(f4);
    auto pC1 = projectPoint(c1); auto pC2 = projectPoint(c2);
    auto pC3 = projectPoint(c3); auto pC4 = projectPoint(c4);

    // Draw the glowing cyan wireframe box
    g.setColour(juce::Colours::cyan.withAlpha(0.8f));
    float thick = 2.0f;

    // Floor
    g.drawLine(juce::Line<float>(pF1, pF2), thick); g.drawLine(juce::Line<float>(pF2, pF3), thick);
    g.drawLine(juce::Line<float>(pF3, pF4), thick); g.drawLine(juce::Line<float>(pF4, pF1), thick);

    // Ceiling
    g.drawLine(juce::Line<float>(pC1, pC2), thick); g.drawLine(juce::Line<float>(pC2, pC3), thick);
    g.drawLine(juce::Line<float>(pC3, pC4), thick); g.drawLine(juce::Line<float>(pC4, pC1), thick);

    // Walls
    g.drawLine(juce::Line<float>(pF1, pC1), thick); g.drawLine(juce::Line<float>(pF2, pC2), thick);
    g.drawLine(juce::Line<float>(pF3, pC3), thick); g.drawLine(juce::Line<float>(pF4, pC4), thick);
}

