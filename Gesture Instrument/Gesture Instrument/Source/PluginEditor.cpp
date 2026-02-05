#include "PluginProcessor.h"
#include "PluginEditor.h"

GestureInstrumentAudioProcessorEditor::GestureInstrumentAudioProcessorEditor(GestureInstrumentAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), settingsPage(p),
    xMinControl("X Min", 1.0f, 150.0f, p.minWidthThreshold),
    xMaxControl("X Max", 150.0f, 350.0f, p.maxWidthThreshold),
    yMinControl("Y Min", 0.0f, 250.0f, p.minHeightThreshold),
    yMaxControl("Y Max", 250.0f, 600.0f, p.maxHeightThreshold),
    zMinControl("Z Front", -300.0f, 0.0f, p.minDepthThreshold),
    zMaxControl("Z Back", 0.0f, 300.0f, p.maxDepthThreshold)
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



    int hudThick = 30;  
    int margin = 10;

    float sens = audioProcessor.sensitivityLevel;
    float range = 200.0f / sens;

    //Left hand
    float lValX = -1.0f, lValY = -1.0f;
    if (audioProcessor.leftHand.isPresent) {
        lValX = juce::jlimit(0.0f, 1.0f, juce::jmap(audioProcessor.leftHand.currentHandPositionX, -range, range, 0.0f, 1.0f));
        lValY = juce::jlimit(0.0f, 1.0f, juce::jmap(audioProcessor.leftHand.currentHandPositionY, audioProcessor.minHeightThreshold, audioProcessor.maxHeightThreshold, 0.0f, 1.0f));
    }
    //Right hand
    float rValX = -1.0f, rValY = -1.0f;
    if (audioProcessor.rightHand.isPresent) {
        rValX = juce::jlimit(0.0f, 1.0f, juce::jmap(audioProcessor.rightHand.currentHandPositionX, -range, range, 0.0f, 1.0f));
        rValY = juce::jlimit(0.0f, 1.0f, juce::jmap(audioProcessor.rightHand.currentHandPositionY, audioProcessor.minHeightThreshold, audioProcessor.maxHeightThreshold, 0.0f, 1.0f));
    }

    // Left hand drawing
    juce::Rectangle<int> leftXRect(margin, 100, 300, hudThick);
    drawParameterHUD(g, leftXRect, audioProcessor.leftXTarget, lValX, false, "L-X", juce::Colours::cyan);

    juce::Rectangle<int> leftYRect(margin, 150, hudThick, 300);
    drawParameterHUD(g, leftYRect, audioProcessor.leftYTarget, lValY, true, "L-Y", juce::Colours::cyan);


    // Right hand drawing
    juce::Rectangle<int> rightXRect(getWidth() - 300 - margin, 100, 300, hudThick);
    drawParameterHUD(g, rightXRect, audioProcessor.rightXTarget, rValX, false, "R-X", juce::Colours::magenta);

    juce::Rectangle<int> rightYRect(getWidth() - margin - hudThick, 150, hudThick, 300);
    drawParameterHUD(g, rightYRect, audioProcessor.rightYTarget, rValY, true, "R-Y", juce::Colours::magenta);
}


void GestureInstrumentAudioProcessorEditor::draw3DGrid(juce::Graphics& g) {
    float w = audioProcessor.maxWidthThreshold; 
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
    int margin = 10;
    int topBarY = margin;

    int buttonW = 120;
    settingsButton.setBounds(getLocalBounds().getCentreX() - (buttonW / 2), margin, buttonW, 30);
    settingsPage.setBounds(getLocalBounds());

    connectionStatusLabel.setBounds(getWidth() - 200 - margin, topBarY, 200, 20);
    modeSelector.setBounds(getWidth() - 160 - margin, topBarY + 30, 150, 30);
    showNoteNamesButton.setBounds(getWidth() - 160, getHeight() - 40, 150, 30);

    int controlHeight = 30;
    rootSelector.setBounds(margin + 80, topBarY + 10, 60, controlHeight);
    scaleSelector.setBounds(rootSelector.getRight() + 10, topBarY + 10, 120, controlHeight);
    octaveSelector.setBounds(scaleSelector.getRight() + 60, topBarY + 10, 100, controlHeight);



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

void GestureInstrumentAudioProcessorEditor::drawPitchBlocks(juce::Graphics& g, juce::Rectangle<int> bounds, float pitchVal, juce::String handName, juce::Colour color) {
    g.setColour(juce::Colours::black.withAlpha(0.6f));
    g.fillRect(bounds);
    g.setColour(juce::Colours::grey.withAlpha(0.5f));
    g.drawRect(bounds, 1.0f);

    if (pitchVal < 0.0f) {
        g.setColour(juce::Colours::grey);
        g.setFont(14.0f);
        g.drawText(handName, bounds, juce::Justification::centred);
        return;
    }

    int numBlocks = juce::jmax(12, audioProcessor.octaveRange * 12);

    float blockWidth = (float)bounds.getWidth() / (float)numBlocks;

    int activeIndex = juce::jlimit(0, numBlocks - 1, (int)(pitchVal * numBlocks));

    for (int i = 0; i < numBlocks; ++i) {
        juce::Rectangle<float> blockRect(
            bounds.getX() + (i * blockWidth),
            (float)bounds.getY(),
            blockWidth,
            (float)bounds.getHeight()
        );

        blockRect.reduce(1.0f, 2.0f);

        if (i == activeIndex) {
            g.setColour(color);
            g.fillRect(blockRect);

            g.setColour(juce::Colours::white.withAlpha(0.5f));
            g.drawRect(blockRect, 2.0f);

            g.setColour(juce::Colours::black);
            g.setFont(juce::Font(14.0f, juce::Font::bold));

            if (showNoteNamesButton.getToggleState()) {
                int root = audioProcessor.rootNote;       // Get Root from processor
                int octave = i / 12;                      // Calculate Octave
                int chromaticInterval = i % 12;           // Calculate Semitone (0-11)

                int noteNum = 48 + (octave * 12) + root + chromaticInterval;

                juce::String noteName = juce::MidiMessage::getMidiNoteName(noteNum, true, true, 3);
                g.drawText(noteName, blockRect, juce::Justification::centred);
            }
            else {
                g.drawText(handName, blockRect, juce::Justification::centred);
            }
        }
        else {
            g.setColour(juce::Colours::white.withAlpha(0.05f));
            g.fillRect(blockRect);
        }
    }

}

std::vector<int> GestureInstrumentAudioProcessorEditor::getScaleIntervals(int scaleType) {
    switch (scaleType) {
    case 1: return { 0, 2, 4, 5, 7, 9, 11 };       // Major (7 notes)
    case 2: return { 0, 2, 3, 5, 7, 8, 10 };       // Minor (7 notes)
    case 3: return { 0, 2, 4, 7, 9 };              // Pentatonic (5 notes)
    default: {                                     // Chromatic (12 notes)
        std::vector<int> chromatic;
        for (int i = 0; i < 12; ++i) chromatic.push_back(i);
        return chromatic;
    }
    }
}

void GestureInstrumentAudioProcessorEditor::drawParameterHUD(juce::Graphics& g, juce::Rectangle<int> bounds, GestureTarget target, float value, bool isVertical, juce::String label, juce::Colour color) {

    // Draw Background
    g.setColour(juce::Colours::black.withAlpha(0.5f));
    g.fillRect(bounds);
    g.setColour(juce::Colours::white.withAlpha(0.1f));
    g.drawRect(bounds);

    g.setColour(juce::Colours::white.withAlpha(0.6f));
    g.setFont(10.0f);
    if (isVertical) {
        g.drawText(label, bounds.removeFromTop(15), juce::Justification::centred);
    }
    else {
        g.drawText(label, bounds.removeFromLeft(30), juce::Justification::centredLeft);
    }

    if (value < 0.0f) return; 

    switch (target) {
    case GestureTarget::Pitch:
        drawScaleBlocks(g, bounds, value, isVertical, color);
        break;

    case GestureTarget::Sustain:
    case GestureTarget::Portamento:
    case GestureTarget::NoteTrigger:
        drawBooleanBox(g, bounds, value, color);
        break;

    case GestureTarget::None:
        break;

    default:
        drawFaderBar(g, bounds, value, isVertical, color);
        break;
    }
}

void GestureInstrumentAudioProcessorEditor::drawScaleBlocks(juce::Graphics& g, juce::Rectangle<int> bounds, float value, bool isVertical, juce::Colour color) {

    std::vector<int> intervals = getScaleIntervals(audioProcessor.scaleType);
    int notesPerOctave = (int)intervals.size();

    int totalBlocks = audioProcessor.octaveRange * notesPerOctave;

    float blockSize = isVertical ?
        (float)bounds.getHeight() / totalBlocks :
        (float)bounds.getWidth() / totalBlocks;

    int activeIndex = juce::jlimit(0, totalBlocks - 1, (int)(value * totalBlocks));

    for (int i = 0; i < totalBlocks; ++i) {

        int octave = i / notesPerOctave;
        int scaleIndex = i % notesPerOctave;
        int chromaticInterval = intervals[scaleIndex];
        int root = audioProcessor.rootNote;

        int noteNum = 48 + (octave * 12) + root + chromaticInterval;
        

        juce::Rectangle<float> blockRect;

        if (isVertical) {
            float yPos = bounds.getBottom() - ((i + 1) * blockSize);
            blockRect = juce::Rectangle<float>((float)bounds.getX(), yPos, (float)bounds.getWidth(), blockSize);
        }
        else {
            float xPos = bounds.getX() + (i * blockSize);
            blockRect = juce::Rectangle<float>(xPos, (float)bounds.getY(), blockSize, (float)bounds.getHeight());
        }

        blockRect.reduce(1.0f, 1.0f);

        if (i == activeIndex) {
            g.setColour(color);
            g.fillRect(blockRect);

            g.setColour(juce::Colours::white.withAlpha(0.5f));
            g.drawRect(blockRect, 2.0f);

            if (showNoteNamesButton.getToggleState()) {
                g.setColour(juce::Colours::black);
                g.setFont(isVertical ? 10.0f : 14.0f);

                juce::String noteName = juce::MidiMessage::getMidiNoteName(noteNum, true, true, 3);
                g.drawText(noteName, blockRect, juce::Justification::centred);
            }
        }
        else {
            g.setColour(juce::Colours::white.withAlpha(0.1f));
            g.fillRect(blockRect);
        }
    }
}

void GestureInstrumentAudioProcessorEditor::drawFaderBar(juce::Graphics& g, juce::Rectangle<int> bounds, float value, bool isVertical, juce::Colour color) {
    // Background track
    g.setColour(juce::Colours::white.withAlpha(0.05f));
    g.fillRect(bounds);

    if (isVertical) {
        int fillH = (int)(bounds.getHeight() * value);
        int y = bounds.getBottom() - fillH;
        g.setColour(color);
        g.fillRect(bounds.getX(), y, bounds.getWidth(), fillH);
    }
    else {
        int fillW = (int)(bounds.getWidth() * value);
        g.setColour(color);
        g.fillRect(bounds.getX(), bounds.getY(), fillW, bounds.getHeight());
    }
}

void GestureInstrumentAudioProcessorEditor::drawBooleanBox(juce::Graphics& g, juce::Rectangle<int> bounds, float value, juce::Colour color) {
    bool isOn = value > 0.5f;

    auto box = bounds.withSizeKeepingCentre(20, 20);

    if (isOn) {
        g.setColour(color);
        g.fillRect(box);
        g.setColour(juce::Colours::white);
        g.drawText("ON", box, juce::Justification::centred);
    }
    else {
        g.setColour(juce::Colours::white.withAlpha(0.2f));
        g.drawRect(box, 2.0f);
        g.setFont(10.0f);
        g.drawText("OFF", box, juce::Justification::centred);
    }
}
