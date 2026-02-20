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

        // Change color to GREEN if hovering, YELLOW if pinching
        juce::Colour cursorCol = juce::Colours::white;
        if (isPinching) cursorCol = juce::Colours::yellow;
        else if (isHoveringClickable) cursorCol = juce::Colours::green;

        g.setColour(cursorCol);
        g.fillEllipse((float)cursorPos.x - 10.0f, (float)cursorPos.y - 10.0f, 20.0f, 20.0f);

        float ringSize = 20.0f + (40.0f * (1.0f - currentPinchStrength));
        g.setColour(cursorCol.withAlpha(currentPinchStrength + 0.2f));
        g.drawEllipse((float)cursorPos.x - ringSize / 2.0f,
            (float)cursorPos.y - ringSize / 2.0f,
            ringSize, ringSize, 2.0f);
    }

    void updateCursorLogic(bool editModeActive) {
        if (!editModeActive || !audioProcessor.rightHand.isPresent) {
            isActive = false;
            isPinching = false;
            wasPinching = false;
            isHoveringClickable = false;
            draggedSlider = nullptr; // Let go of any slider
            repaint();
            return;
        }

        isActive = true;
        currentPinchStrength = audioProcessor.rightHand.pinchStrength;

        // Map Leap Motion bounds to Screen Size
        int screenX = (int)juce::jmap(audioProcessor.rightHand.currentHandPositionX,
            audioProcessor.minWidthThreshold, audioProcessor.maxWidthThreshold,
            0.0f, (float)getWidth());

        int screenY = (int)juce::jmap(audioProcessor.rightHand.currentHandPositionY,
            audioProcessor.minHeightThreshold, audioProcessor.maxHeightThreshold,
            (float)getHeight(), 0.0f);

        cursorPos = { screenX, screenY };
        isHoveringClickable = false;
        isPinching = currentPinchStrength > 0.8f;

        // If we let go of the pinch, clear the dragged slider
        if (!isPinching) {
            draggedSlider = nullptr;
        }

        // Raycast to see what UI element is under the cursor
        if (auto* parent = getParentComponent()) {
            if (auto* hitComp = parent->getComponentAt(cursorPos)) {

                // --- HOVER LOGIC ---
                if (dynamic_cast<juce::Button*>(hitComp) ||
                    dynamic_cast<juce::ComboBox*>(hitComp) ||
                    dynamic_cast<juce::Slider*>(hitComp)) {
                    isHoveringClickable = true;
                }

                // --- INITIAL CLICK LOGIC ---
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
                        // Lock onto the slider!
                        draggedSlider = slider;
                    }
                }
            }
        }

        // --- DRAG LOGIC ---
        if (isPinching && draggedSlider != nullptr) {
            // Convert the window-level cursor position into the slider's local coordinate space
            auto localPos = draggedSlider->getLocalPoint(this, cursorPos);

            float proportion = 0.0f;

            // Calculate the 0.0 to 1.0 percentage based on whether the slider is horizontal or vertical
            if (draggedSlider->isVertical()) {
                // Y goes down, so we invert it so "up" is max
                proportion = juce::jlimit(0.0f, 1.0f, 1.0f - ((float)localPos.y / (float)draggedSlider->getHeight()));
            }
            else {
                proportion = juce::jlimit(0.0f, 1.0f, (float)localPos.x / (float)draggedSlider->getWidth());
            }

            // Map that percentage to the actual min/max values of the slider
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
    float currentPinchStrength = 0.0f;

    bool isActive = false;
    bool isPinching = false;
    bool wasPinching = false;
    bool isHoveringClickable = false;

    // Tracks the slider we are currently holding
    juce::Slider* draggedSlider = nullptr;
};