#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "UI/HUDComponents.h"
#include "Helpers/MusicalRangeMode.h"

GestureInstrumentAudioProcessorEditor::GestureInstrumentAudioProcessorEditor(GestureInstrumentAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), settingsPage(p), staticDialsPage(p), hud(p), virtualCursor(p),
    xMinControl("Min Width", -350.0f, 0.0f, p.minWidthThreshold),
    xMaxControl("Max Width", 0.0f, 350.0f, p.maxWidthThreshold),
    yMinControl("Min Height", 50.0f, 275.0f, p.minHeightThreshold),
    yMaxControl("Max Height", 275.0f, 500.0f, p.maxHeightThreshold),
    zMinControl("Back Depth", -225.0f, 0.0f, p.minDepthThreshold),
    zMaxControl("Front Depth", 0.0f, 225.0f, p.maxDepthThreshold)
{
    addAndMakeVisible(hud);
    addAndMakeVisible(calibrationOverlay);
    calibrationOverlay.setVisible(false);



    addAndMakeVisible(calibrateButton);
    calibrateButton.onClick = [this] { startCalibration(); };

    addAndMakeVisible(editModeButton);
    editModeButton.setButtonText("Virtual Mouse");
    editModeButton.setClickingTogglesState(true);
    editModeButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
    editModeButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::orange);
    editModeButton.onClick = [this] {
        isEditMode = editModeButton.getToggleState();
        audioProcessor.muteOutput.store(isEditMode || isCalibrating);
        };

    addAndMakeVisible(connectionStatusLabel);
    connectionStatusLabel.setText("Checking Sensor...", juce::dontSendNotification);
    connectionStatusLabel.setJustificationType(juce::Justification::centredRight);
    connectionStatusLabel.setColour(juce::Label::textColourId, juce::Colours::grey);

    addAndMakeVisible(xMinControl);
    xMinControl.slider.onValueChange = [this] { audioProcessor.minWidthThreshold = (float)xMinControl.slider.getValue(); };
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

    addAndMakeVisible(staticDialsButton);
    staticDialsButton.onClick = [this] {
        staticDialsPage.setVisible(true);

        settingsButton.setVisible(false);
        staticDialsButton.setVisible(false);
        calibrateButton.setVisible(false);
        editModeButton.setVisible(false);
        connectionStatusLabel.setVisible(false);
        showNoteNamesButton.setVisible(false);

        rootSelector.setVisible(false);
        scaleSelector.setVisible(false);
        scaleLabel.setVisible(false);
        rangeModeSelector.setVisible(false);
        octaveSelector.setVisible(false);
        octaveLabel.setVisible(false);
        startNoteSelector.setVisible(false);
        endNoteSelector.setVisible(false);

        xMinControl.setVisible(false); xMaxControl.setVisible(false);
        yMinControl.setVisible(false); yMaxControl.setVisible(false);
        zMinControl.setVisible(false); zMaxControl.setVisible(false);
        };
    addChildComponent(staticDialsPage);
    staticDialsPage.setVisible(false);

    staticDialsPage.closeButton.onClick = [this] {
        staticDialsPage.setVisible(false);

        settingsButton.setVisible(true);
        staticDialsButton.setVisible(true);
        calibrateButton.setVisible(true);
        editModeButton.setVisible(true);
        connectionStatusLabel.setVisible(true);
        showNoteNamesButton.setVisible(true);

        rootSelector.setVisible(true);
        scaleSelector.setVisible(true);
        scaleLabel.setVisible(true);
        rangeModeSelector.setVisible(true);

        xMinControl.setVisible(true); xMaxControl.setVisible(true);
        yMinControl.setVisible(true); yMaxControl.setVisible(true);
        zMinControl.setVisible(true); zMaxControl.setVisible(true);

        rangeModeSelector.onChange();
        };

    addAndMakeVisible(settingsButton);

    settingsButton.onClick = [this] {
        settingsPage.setVisible(true);

        settingsButton.setVisible(false);
        staticDialsButton.setVisible(false);
        calibrateButton.setVisible(false);
        editModeButton.setVisible(false);
        connectionStatusLabel.setVisible(false);
        showNoteNamesButton.setVisible(false);

        rootSelector.setVisible(false);
        scaleSelector.setVisible(false);
        scaleLabel.setVisible(false);
        rangeModeSelector.setVisible(false);
        octaveSelector.setVisible(false);
        octaveLabel.setVisible(false);
        startNoteSelector.setVisible(false);
        endNoteSelector.setVisible(false);

        xMinControl.setVisible(false); xMaxControl.setVisible(false);
        yMinControl.setVisible(false); yMaxControl.setVisible(false);
        zMinControl.setVisible(false); zMaxControl.setVisible(false);
        };

    settingsPage.closeButton.onClick = [this] {
        settingsPage.setVisible(false);

        settingsButton.setVisible(true);
        staticDialsButton.setVisible(true);
        calibrateButton.setVisible(true);
        editModeButton.setVisible(true);
        connectionStatusLabel.setVisible(true);
        showNoteNamesButton.setVisible(true);

        rootSelector.setVisible(true);
        scaleSelector.setVisible(true);
        scaleLabel.setVisible(true);
        rangeModeSelector.setVisible(true);

        xMinControl.setVisible(true); xMaxControl.setVisible(true);
        yMinControl.setVisible(true); yMaxControl.setVisible(true);
        zMinControl.setVisible(true); zMaxControl.setVisible(true);

        rangeModeSelector.onChange();
        };

    settingsPage.onPresetLoaded = [this] {
        rangeModeSelector.onChange();
        resized();
        repaint();
        };

    addAndMakeVisible(rootSelector);
    rootSelector.addItemList({ "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" }, 1);
    rootSelector.setSelectedId(p.rootNote + 1);
    rootSelector.onChange = [this] { audioProcessor.rootNote = rootSelector.getSelectedId() - 1; };

    addAndMakeVisible(scaleSelector);
    scaleSelector.setText("Scale:", juce::dontSendNotification);
    scaleSelector.addItem("Chromatic", 1);
    scaleSelector.addItem("Major", 2);
    scaleSelector.addItem("Minor", 3);
    scaleSelector.addItem("Pentatonic", 4);
    scaleSelector.setSelectedId(p.scaleType + 1);
    scaleSelector.onChange = [this] { audioProcessor.scaleType = scaleSelector.getSelectedId() - 1; };

    addAndMakeVisible(scaleLabel);
    scaleLabel.setText("Key:", juce::dontSendNotification);
    scaleLabel.attachToComponent(&rootSelector, true);
    scaleLabel.setColour(juce::Label::textColourId, juce::Colours::white);

    addAndMakeVisible(rangeModeSelector);
    rangeModeSelector.addItem("Standard Range", 1);
    rangeModeSelector.addItem("Custom Range", 2);

    addAndMakeVisible(startNoteSelector);
    for (int i = 0; i <= 127; ++i) {
        juce::String noteName = juce::MidiMessage::getMidiNoteName(i, true, true, 3);
        startNoteSelector.addItem(noteName, i + 1);
    }
    startNoteSelector.setSelectedId(audioProcessor.startNote + 1, juce::dontSendNotification);
    startNoteSelector.onChange = [this] { audioProcessor.startNote = startNoteSelector.getSelectedId() - 1; };

    addAndMakeVisible(endNoteSelector);
    for (int i = 0; i <= 127; ++i) {
        juce::String noteName = juce::MidiMessage::getMidiNoteName(i, true, true, 3);
        endNoteSelector.addItem(noteName, i + 1);
    }
    endNoteSelector.setSelectedId(audioProcessor.endNote + 1, juce::dontSendNotification);
    endNoteSelector.onChange = [this] { audioProcessor.endNote = endNoteSelector.getSelectedId() - 1; };

    addAndMakeVisible(octaveSelector);
    octaveSelector.addItem("1 Octave", 1);
    octaveSelector.addItem("2 Octaves", 2);
    octaveSelector.addItem("3 Octaves", 3);
    octaveSelector.addItem("4 Octaves", 4);
    octaveSelector.setSelectedId(p.octaveRange > 0 ? p.octaveRange : 2, juce::dontSendNotification);
    octaveSelector.onChange = [this] { audioProcessor.octaveRange = octaveSelector.getSelectedId(); };

    addAndMakeVisible(octaveLabel);
    octaveLabel.attachToComponent(&octaveSelector, true);
    octaveLabel.setColour(juce::Label::textColourId, juce::Colours::white);

    rangeModeSelector.onChange = [this] {
        bool isCustom = rangeModeSelector.getSelectedId() == 2;
        audioProcessor.currentRangeMode = isCustom ? MusicalRangeMode::SpecificNotes : MusicalRangeMode::OctaveRange;
        octaveSelector.setVisible(!isCustom);
        octaveLabel.setVisible(!isCustom);
        startNoteSelector.setVisible(isCustom);
        endNoteSelector.setVisible(isCustom);
        resized();
        };

    rangeModeSelector.setSelectedId(audioProcessor.currentRangeMode == MusicalRangeMode::SpecificNotes ? 2 : 1, juce::NotificationType::sendNotificationSync);

    addAndMakeVisible(showNoteNamesButton);
    showNoteNamesButton.setToggleState(false, juce::dontSendNotification);
    showNoteNamesButton.setColour(juce::ToggleButton::textColourId, juce::Colours::white);
    showNoteNamesButton.onClick = [this] {
        audioProcessor.showNoteNames = showNoteNamesButton.getToggleState();
        repaint();
        };

    

    audioProcessor.oscManager.onStyleChanged = [this](float leftStyle, float rightStyle) {
        juce::MessageManager::callAsync([this, leftStyle, rightStyle]() {

            float maxStyle = std::max(leftStyle, rightStyle);

            if (std::abs(currentStyle - maxStyle) > 0.01f) {
                currentStyle = maxStyle;
                repaint();
            }
            });
        };

    setResizable(true, true);
    setResizeLimits(800, 600, 3000, 2000);
    setOpaque(true);

    openGLContext.attachTo(*this);
    openGLContext.setContinuousRepainting(false);

    setSize(1500, 700);
    startTimerHz(60);

    addChildComponent(settingsPage);
    settingsPage.setVisible(false);
    addAndMakeVisible(virtualCursor);
}

GestureInstrumentAudioProcessorEditor::~GestureInstrumentAudioProcessorEditor() {
    openGLContext.detach();
    stopTimer();
}

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

    if (menuGestureTimer > 0.0f && !menuGestureFired && !isCalibrating) {
        float requiredTime = (float)settingsPage.gestureTimerSlider.getValue();
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


    auto indicatorColor = juce::Colours::skyblue.interpolatedWith(juce::Colours::indianred, currentStyle);
    g.setColour(indicatorColor);

    auto area = getLocalBounds().removeFromTop(40).reduced(10);
    g.fillRoundedRectangle(area.toFloat(), 5.0f);

    g.setColour(juce::Colours::white);

    juce::String label = "INTENT: " + juce::String(currentStyle * 100.0f, 0) + "% AGGRESSIVE";
    g.drawText(label, area, juce::Justification::centred);

}

void GestureInstrumentAudioProcessorEditor::draw3DGrid(juce::Graphics& g) {
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
        return {
            juce::jlimit(audioProcessor.minWidthThreshold, audioProcessor.maxWidthThreshold, p.x),
            juce::jlimit(audioProcessor.minHeightThreshold, audioProcessor.maxHeightThreshold, p.y),
            juce::jlimit(audioProcessor.minDepthThreshold, audioProcessor.maxDepthThreshold, p.z)
        };
        };

    Point3D palm3D = clampPoint({ hand.currentHandPositionX, hand.currentHandPositionY, hand.currentHandPositionZ });
    auto palm2D = projectPoint(palm3D);
    float palmDepth = juce::jmap(palm3D.z, -200.0f, 200.0f, 0.6f, 1.4f);

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

void GestureInstrumentAudioProcessorEditor::resized() {
    calibrationOverlay.setBounds(getLocalBounds());
    settingsPage.setBounds(getLocalBounds());
    staticDialsPage.setBounds(getLocalBounds());
    virtualCursor.setBounds(getLocalBounds());

    int margin = 10;
    int topBarY = margin;
    int buttonW = 120;
    int controlHeight = 30;

    rootSelector.setBounds(margin + 40, topBarY, 60, controlHeight);
    scaleSelector.setBounds(rootSelector.getRight() + 10, topBarY, 120, controlHeight);

    int row2Y = topBarY + controlHeight + 5; 

    rangeModeSelector.setBounds(margin, row2Y, 140, controlHeight);

    int octX = rangeModeSelector.getRight() + 10;
    octaveSelector.setBounds(octX, row2Y, 100, controlHeight);
    startNoteSelector.setBounds(octX, row2Y, 85, controlHeight);
    endNoteSelector.setBounds(startNoteSelector.getRight() + 5, row2Y, 85, controlHeight);

    int centerX = getWidth() / 2;
    settingsButton.setBounds(centerX - 185, topBarY, buttonW, 30);

    calibrateButton.setBounds(centerX - 60, topBarY, buttonW, 30);
    editModeButton.setBounds(centerX + 65, topBarY, buttonW, 30);
    staticDialsButton.setBounds(centerX + 190, topBarY, buttonW, 30);


  

    int rightEdge = getWidth() - margin;
    connectionStatusLabel.setBounds(rightEdge - 200, topBarY, 200, 20);

    showNoteNamesButton.setBounds(rightEdge - 150, topBarY + 25, 150, 30);

    

    hud.setBounds(0, 110, getWidth(), 550);

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

void GestureInstrumentAudioProcessorEditor::timerCallback() {
    updateConnectionStatus();

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

    if (settingsPage.enableGestureSwitchButton.getToggleState() && !isCalibrating) {
        bool leftFist = audioProcessor.leftHand.isPresent && audioProcessor.leftHand.grabStrength > 0.85f;
        bool rightFist = audioProcessor.rightHand.isPresent && audioProcessor.rightHand.grabStrength > 0.85f;
        bool gestureTriggered = false;

        int mode = settingsPage.gestureTypeSelector.getSelectedId();

        if (mode == 1) gestureTriggered = leftFist && rightFist;
        else if (mode == 2) gestureTriggered = rightFist;
        else if (mode == 3) gestureTriggered = leftFist;

        float requiredHoldTime = (float)settingsPage.gestureTimerSlider.getValue();

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
}

void GestureInstrumentAudioProcessorEditor::CalibrationOverlay::paint(juce::Graphics& g) {
    g.fillAll(juce::Colours::black.withAlpha(0.85f));
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(28.0f, juce::Font::bold));
    g.drawText("Move hands to draw out playing area.", getLocalBounds().withY(-50), juce::Justification::centred);

    g.setColour(juce::Colours::yellow);
    g.setFont(juce::Font(20.0f, juce::Font::plain));
    g.drawText("Grab gesture to confirm.", getLocalBounds().withY(20), juce::Justification::centred);

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
    audioProcessor.muteOutput.store(true);
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