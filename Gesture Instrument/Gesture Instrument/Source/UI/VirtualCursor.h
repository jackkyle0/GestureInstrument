#pragma once
#include <JuceHeader.h>
#include "../PluginProcessor.h"

class VirtualCursor : public juce::Component {
public:
    VirtualCursor(GestureInstrumentAudioProcessor& p)
        : audioProcessor(p)
    {
        setInterceptsMouseClicks(false, false);
    }

    void paint(juce::Graphics& g) override {
        if (!isActive) return;

        juce::Colour cursorCol = juce::Colours::white;
        if (isPinching) cursorCol = juce::Colours::yellow;
        else if (isHoveringClickable) cursorCol = juce::Colours::green;

        // Use the smoothed coordinates for drawing!
        g.setColour(cursorCol);
        g.fillEllipse(smoothedX - 10.0f, smoothedY - 10.0f, 20.0f, 20.0f);

        float safeAlpha = juce::jlimit(0.0f, 1.0f, currentPinchStrength + 0.2f);
        float ringSize = 20.0f + (40.0f * (1.0f - currentPinchStrength));
        g.setColour(cursorCol.withAlpha(safeAlpha));
        g.drawEllipse(smoothedX - ringSize / 2.0f, smoothedY - ringSize / 2.0f, ringSize, ringSize, 2.0f);
    }

    void updateCursorLogic(bool editModeActive) {
        if (!editModeActive || !audioProcessor.rightHand.isPresent) {
            isActive = false;
            isPinching = false;
            wasPinching = false;
            isHoveringClickable = false;
            draggedSlider = nullptr;
            repaint();
            return;
        }

        isActive = true;
        currentPinchStrength = audioProcessor.rightHand.pinchStrength;

        // 1. Pinch Hysteresis (Prevents accidental double-clicks from hand shaking)
        if (currentPinchStrength > 0.8f) {
            isPinching = true;
        }
        else if (currentPinchStrength < 0.5f) {
            isPinching = false; // Only release the click if they OPEN their hand significantly
        }

        // 2. Calculate Target Coordinates with SENSITIVITY MULTIPLIER
        // First, map the hand to a 0.0 to 1.0 percentage
        float normX = juce::jmap(audioProcessor.rightHand.currentHandPositionX, audioProcessor.minWidthThreshold, audioProcessor.maxWidthThreshold, 0.0f, 1.0f);
        float normY = juce::jmap(audioProcessor.rightHand.currentHandPositionY, audioProcessor.minHeightThreshold, audioProcessor.maxHeightThreshold, 1.0f, 0.0f); // Y is inverted

        // Apply a 1.3x sensitivity multiplier (expands the reach from the center outward)
        // You can tweak this number! 1.5f requires even less physical movement.
        float sensitivity = 1.0f;
        normX = 0.5f + ((normX - 0.5f) * sensitivity);
        normY = 0.5f + ((normY - 0.5f) * sensitivity);

        // Convert to pixels and strictly clamp it so the real mouse doesn't get pushed off-screen
        float targetX = juce::jlimit(0.0f, (float)getWidth(), normX * getWidth());
        float targetY = juce::jlimit(0.0f, (float)getHeight(), normY * getHeight());
        // 3. Dynamic Smoothing (The secret sauce)
        float smoothingSpeed = 0.6f; // Default: Fast & responsive in empty space

        if (isPinching) {
            smoothingSpeed = 0.1f; // Heavy smoothing: Prevents slipping while dragging sliders
        }
        else if (isHoveringClickable) {
            smoothingSpeed = 0.25f; // Medium smoothing: Makes buttons feel "magnetic" and sticky
        }

        // Apply Exponential Moving Average filter
        smoothedX += (targetX - smoothedX) * smoothingSpeed;
        smoothedY += (targetY - smoothedY) * smoothingSpeed;

        cursorPos = { (int)smoothedX, (int)smoothedY };

        // 4. Update the Real Mouse Cursor
        if (auto* parent = getParentComponent()) {
            auto globalPos = parent->localPointToGlobal(cursorPos.toFloat());
            juce::Desktop::getInstance().getMainMouseSource().setScreenPosition(globalPos);
        }

        // Reset hover state before checking
        isHoveringClickable = false;
        if (!isPinching) {
            draggedSlider = nullptr;
        }

        // 5. Check for UI Interaction
        if (auto* parent = getParentComponent()) {
            if (auto* hitComp = parent->getComponentAt(cursorPos)) {

                // If we are over an interactive component, flag it so the cursor turns green and slows down
                if (dynamic_cast<juce::Button*>(hitComp) || dynamic_cast<juce::ComboBox*>(hitComp) || dynamic_cast<juce::Slider*>(hitComp)) {
                    isHoveringClickable = true;
                }

                if (isPinching && !wasPinching) {
                    if (auto* btn = dynamic_cast<juce::Button*>(hitComp)) {
                        btn->triggerClick();
                    }
                    else if (auto* cb = dynamic_cast<juce::ComboBox*>(hitComp)) {
                        int nextIndex = cb->getSelectedItemIndex() + 1;
                        if (nextIndex >= cb->getNumItems()) nextIndex = 0;
                        cb->setSelectedItemIndex(nextIndex, juce::sendNotificationSync);
                    }
                    else if (auto* slider = dynamic_cast<juce::Slider*>(hitComp)) {
                        draggedSlider = slider;
                    }
                }
            }
        }

        // 6. Handle Slider Dragging
        if (isPinching && draggedSlider != nullptr) {
            auto localPos = draggedSlider->getLocalPoint(this, cursorPos);
            float proportion = 0.0f;

            if (draggedSlider->isVertical()) {
                proportion = juce::jlimit(0.0f, 1.0f, 1.0f - ((float)localPos.y / (float)draggedSlider->getHeight()));
            }
            else {
                proportion = juce::jlimit(0.0f, 1.0f, (float)localPos.x / (float)draggedSlider->getWidth());
            }

            double range = draggedSlider->getMaximum() - draggedSlider->getMinimum();
            double newValue = draggedSlider->getMinimum() + (range * proportion);
            draggedSlider->setValue(newValue, juce::sendNotificationSync);
        }

        wasPinching = isPinching;
        repaint();
    }

private:
    GestureInstrumentAudioProcessor& audioProcessor;
    juce::Point<int> cursorPos;

    // New variables to hold the smoothed cursor data
    float smoothedX = 0.0f;
    float smoothedY = 0.0f;

    float currentPinchStrength = 0.0f;

    bool isActive = false;
    bool isPinching = false;
    bool wasPinching = false;
    bool isHoveringClickable = false;

    juce::Slider* draggedSlider = nullptr;
};