#pragma once

#include <JuceHeader.h>
#include "../PluginProcessor.h"

class VirtualCursor : public juce::Component {
public:
    VirtualCursor(GestureInstrumentAudioProcessor& p)
        : audioProcessor(p) {
        setInterceptsMouseClicks(false, false);
    }

    void paint(juce::Graphics& g) override {
        if (!isActive) return;

        juce::Colour cursorCol = juce::Colours::white;
        if (isPinching) cursorCol = juce::Colours::yellow;
        else if (isHoveringClickable) cursorCol = juce::Colours::green;

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

        if (currentPinchStrength > 0.8f) {
            isPinching = true;
        }
        else if (currentPinchStrength < 0.5f) {
            isPinching = false;
        }

        float normalizedX = juce::jmap(audioProcessor.rightHand.currentHandPositionX, audioProcessor.minWidthThreshold, audioProcessor.maxWidthThreshold, 0.0f, 1.0f);
        float normalizedY = juce::jmap(audioProcessor.rightHand.currentHandPositionY, audioProcessor.minHeightThreshold, audioProcessor.maxHeightThreshold, 1.0f, 0.0f);

        const float sensitivity = 1.3f;
        normalizedX = 0.5f + ((normalizedX - 0.5f) * sensitivity);
        normalizedY = 0.5f + ((normalizedY - 0.5f) * sensitivity);

        float targetX = juce::jlimit(0.0f, (float)getWidth(), normalizedX * getWidth());
        float targetY = juce::jlimit(0.0f, (float)getHeight(), normalizedY * getHeight());

        float smoothingSpeed = 0.6f;

        if (isPinching) {
            smoothingSpeed = 0.1f;
        }
        else if (isHoveringClickable) {
            smoothingSpeed = 0.25f;
        }

        smoothedX += (targetX - smoothedX) * smoothingSpeed;
        smoothedY += (targetY - smoothedY) * smoothingSpeed;

        cursorPos = { (int)smoothedX, (int)smoothedY };

        if (auto* parent = getParentComponent()) {
            auto globalPos = parent->localPointToGlobal(cursorPos.toFloat());
            juce::Desktop::getInstance().getMainMouseSource().setScreenPosition(globalPos);
        }

        isHoveringClickable = false;
        if (!isPinching) {
            draggedSlider = nullptr;
        }

        if (auto* parent = getParentComponent()) {
            if (auto* hoveredComponent = parent->getComponentAt(cursorPos)) {

                if (dynamic_cast<juce::Button*>(hoveredComponent) || dynamic_cast<juce::ComboBox*>(hoveredComponent) || dynamic_cast<juce::Slider*>(hoveredComponent)) {
                    isHoveringClickable = true;
                }

                if (isPinching && !wasPinching) {
                    if (auto* btn = dynamic_cast<juce::Button*>(hoveredComponent)) {
                        btn->triggerClick();
                    }
                    else if (auto* cb = dynamic_cast<juce::ComboBox*>(hoveredComponent)) {
                        int nextIndex = cb->getSelectedItemIndex() + 1;
                        if (nextIndex >= cb->getNumItems()) nextIndex = 0;
                        cb->setSelectedItemIndex(nextIndex, juce::sendNotificationSync);
                    }
                    else if (auto* slider = dynamic_cast<juce::Slider*>(hoveredComponent)) {
                        draggedSlider = slider;
                    }
                }
            }
        }

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

    float smoothedX = 0.0f;
    float smoothedY = 0.0f;
    float currentPinchStrength = 0.0f;

    bool isActive = false;
    bool isPinching = false;
    bool wasPinching = false;
    bool isHoveringClickable = false;

    juce::Slider* draggedSlider = nullptr;
};