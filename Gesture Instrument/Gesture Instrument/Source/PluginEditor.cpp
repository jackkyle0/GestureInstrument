#include "PluginProcessor.h"
#include "PluginEditor.h"

GestureInstrumentAudioProcessorEditor::GestureInstrumentAudioProcessorEditor(GestureInstrumentAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), settingsPage(p)
{

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

    // Settings Page+Button
    addAndMakeVisible(settingsButton);
    settingsButton.onClick = [this] {
        settingsPage.setVisible(true);
        settingsButton.setVisible(false);
        rootSelector.setVisible(false);
        scaleSelector.setVisible(false);
        };

    addChildComponent(settingsPage);
    settingsPage.setVisible(false);

    settingsPage.closeButton.onClick = [this] {
        settingsPage.setVisible(false);
        settingsButton.setVisible(true);
        rootSelector.setVisible(true);
        scaleSelector.setVisible(true);
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


    // Graphics Setup
    setResizable(true, true);
    setResizeLimits(800, 600, 3000, 2000);
    setOpaque(true);

    openGLContext.attachTo(*this);
    openGLContext.setContinuousRepainting(false); // control the paint timing

    setSize(1200, 800);
    startTimerHz(60);
}

GestureInstrumentAudioProcessorEditor::~GestureInstrumentAudioProcessorEditor() {
    openGLContext.detach();
    stopTimer();
}

// ==============================================================================
// 3D Projection maths
juce::Point<float> GestureInstrumentAudioProcessorEditor::projectPoint(Point3D p) {
    // Centre Y
    float worldY = p.y - 250.0f;

    // Invert Z axis
    float adjustedZ = -p.z;

    // Perpective
    float perspective = fov / (fov + adjustedZ + camDist);
    float zoom = ((float)getHeight() * 0.7f) / 500.0f;
    float finalScale = perspective * zoom * 2.5f;

    // X axis
    float x2d = p.x * finalScale + centerScreen.x;

    float y2d = -worldY * finalScale + centerScreen.y;

    return juce::Point<float>(x2d, y2d);
}

// Painting
void GestureInstrumentAudioProcessorEditor::paint(juce::Graphics& g) {
        // Background
    juce::ColourGradient bgGradient(
        juce::Colour::fromFloatRGBA(0.05f, 0.05f, 0.1f, 1.0f),
        (float)getWidth() / 2, (float)getHeight() / 2,
        juce::Colour::fromFloatRGBA(0.0f, 0.0f, 0.0f, 1.0f),
        0.0f, 0.0f, true);
    g.setGradientFill(bgGradient);
    g.fillAll();

    // Update center point 
    centerScreen = getLocalBounds().getCentre().toFloat();

    draw3DGrid(g);

    draw3DHand(g, audioProcessor.leftHand, juce::Colours::cyan);
    draw3DHand(g, audioProcessor.rightHand, juce::Colours::magenta);
}

void GestureInstrumentAudioProcessorEditor::draw3DGrid(juce::Graphics& g) {
    // Dimensions adjusted to match Leap Motion range 
    float w = 300.0f; 
    float h = 500.0f; 
    float d = 200.0f; 

    // Define Corners
    Point3D f1 = { -w, 0, -d }; Point3D f2 = { w, 0, -d }; // Floor Back
    Point3D f3 = { w, 0, d };   Point3D f4 = { -w, 0, d }; // Floor Front

    Point3D c1 = { -w, h, -d }; Point3D c2 = { w, h, -d }; // Ceiling Back
    Point3D c3 = { w, h, d };   Point3D c4 = { -w, h, d }; // Ceiling Front

    // Project to 2D
    auto pF1 = projectPoint(f1); auto pF2 = projectPoint(f2);
    auto pF3 = projectPoint(f3); auto pF4 = projectPoint(f4);
    auto pC1 = projectPoint(c1); auto pC2 = projectPoint(c2);
    auto pC3 = projectPoint(c3); auto pC4 = projectPoint(c4);

    // Draw grid lines
    g.setColour(juce::Colours::white.withAlpha(0.08f)); 

    // Floor and Ceiling
    g.drawLine(juce::Line<float>(pF1, pF2)); g.drawLine(juce::Line<float>(pF2, pF3));
    g.drawLine(juce::Line<float>(pF3, pF4)); g.drawLine(juce::Line<float>(pF4, pF1));
    g.drawLine(juce::Line<float>(pC1, pC2)); g.drawLine(juce::Line<float>(pC2, pC3));
    g.drawLine(juce::Line<float>(pC3, pC4)); g.drawLine(juce::Line<float>(pC4, pC1));

    // Pillars
    g.drawLine(juce::Line<float>(pF1, pC1)); g.drawLine(juce::Line<float>(pF2, pC2));
    g.drawLine(juce::Line<float>(pF3, pC3)); g.drawLine(juce::Line<float>(pF4, pC4));

    // Hight for active areas
    float minH = audioProcessor.minHeightThreshold;
    float maxH = audioProcessor.maxHeightThreshold;

    // Min Threshold line 
    g.setColour(juce::Colours::yellow.withAlpha(0.4f)); 
    auto tMin1 = projectPoint({ -w, minH, -d }); auto tMin2 = projectPoint({ w, minH, -d });
    auto tMin3 = projectPoint({ w, minH, d });   auto tMin4 = projectPoint({ -w, minH, d });

    // Draw thick lines for visibility
    float lineThick = 2.5f;
    g.drawLine(juce::Line<float>(tMin1, tMin2), lineThick); g.drawLine(juce::Line<float>(tMin2, tMin3), lineThick);
    g.drawLine(juce::Line<float>(tMin3, tMin4), lineThick); g.drawLine(juce::Line<float>(tMin4, tMin1), lineThick);

    // Max Threshold line
    g.setColour(juce::Colours::yellow.withAlpha(0.4f));
    auto tMax1 = projectPoint({ -w, maxH, -d }); auto tMax2 = projectPoint({ w, maxH, -d });
    auto tMax3 = projectPoint({ w, maxH, d });   auto tMax4 = projectPoint({ -w, maxH, d });

    g.drawLine(juce::Line<float>(tMax1, tMax2), lineThick); g.drawLine(juce::Line<float>(tMax2, tMax3), lineThick);
    g.drawLine(juce::Line<float>(tMax3, tMax4), lineThick); g.drawLine(juce::Line<float>(tMax4, tMax1), lineThick);
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
        auto joint1 = projectPoint({ f.joint2X,  f.joint2Y,  f.joint2Z }); // PIP
        auto joint2 = projectPoint({ f.joint1X,  f.joint1Y,  f.joint1Z }); // DIP
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
    int margin = 10;
    int topBarY = margin;

    // Settings Button in Bottom Center
    int buttonW = 120;
    settingsButton.setBounds(getLocalBounds().getCentreX() - (buttonW / 2), margin, buttonW, 30);
    settingsPage.setBounds(getLocalBounds());

    // Connection Status in Top Right
    connectionStatusLabel.setBounds(getWidth() - 200 - margin, topBarY, 200, 20);

    // Output Mode in Top Right, below status
    modeSelector.setBounds(getWidth() - 160 - margin, topBarY + 30, 150, 30);

    // Scale Controls in Top Left
    int controlHeight = 30;
    rootSelector.setBounds(margin + 80, topBarY + 10, 60, controlHeight);
    scaleSelector.setBounds(rootSelector.getRight() + 10, topBarY + 10, 120, controlHeight);
}

void GestureInstrumentAudioProcessorEditor::timerCallback() {
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

void GestureInstrumentAudioProcessorEditor::comboBoxChanged(juce::ComboBox* box) {
    if (box == &modeSelector) {
        int id = modeSelector.getSelectedId();
        if (id == 1) audioProcessor.currentOutputMode = OutputMode::OSC_Only;
        if (id == 2) audioProcessor.currentOutputMode = OutputMode::MIDI_Only;
    }
}