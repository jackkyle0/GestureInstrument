#pragma once
#include <JuceHeader.h>
#include "../PluginProcessor.h"

class NiceDialLookAndFeel : public juce::LookAndFeel_V4 {
public:
    NiceDialLookAndFeel() {}

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
        const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider) override {

        auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(10.0f);
        auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
        auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        auto lineW = 6.0f;
        auto arcRadius = radius - lineW * 0.5f;

        juce::Point<float> center = bounds.getCentre();

        juce::Path backgroundArc;
        backgroundArc.addCentredArc(center.x, center.y, arcRadius, arcRadius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
        g.setColour(juce::Colour::fromFloatRGBA(0.15f, 0.15f, 0.15f, 1.0f));
        g.strokePath(backgroundArc, juce::PathStrokeType(lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        if (slider.isEnabled()) {
            juce::Path valueArc;
            valueArc.addCentredArc(center.x, center.y, arcRadius, arcRadius, 0.0f, rotaryStartAngle, toAngle, true);
            g.setColour(slider.findColour(juce::Slider::rotarySliderFillColourId));
            g.strokePath(valueArc, juce::PathStrokeType(lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }

        float capRadius = arcRadius - 8.0f;
        g.setColour(juce::Colour::fromFloatRGBA(0.2f, 0.2f, 0.22f, 1.0f));
        g.fillEllipse(center.x - capRadius, center.y - capRadius, capRadius * 2.0f, capRadius * 2.0f);

        juce::Path pointer;
        float pointerLength = capRadius * 0.7f;
        float pointerThickness = 3.0f;
        pointer.addRectangle(-pointerThickness * 0.5f, -capRadius, pointerThickness, pointerLength);
        pointer.applyTransform(juce::AffineTransform::rotation(toAngle).translated(center.x, center.y));
        g.setColour(juce::Colours::white.withAlpha(0.8f));
        g.fillPath(pointer);
    }
};

class StaticDialsComponent : public juce::Component, public juce::Timer {
public:
    StaticDialsComponent(GestureInstrumentAudioProcessor& p) : audioProcessor(p) {

        setLookAndFeel(&customDialLook);

        setupDial(volumeDial, volumeLabel, "Volume", 0.0f, 1.0f, audioProcessor.staticVolume, false);
        setupDial(panDial, panLabel, "Pan", 0.0f, 1.0f, audioProcessor.staticPan, false);
        setupDial(modDial, modLabel, "Modulation", 0.0f, 1.0f, audioProcessor.staticModulation, false);
        setupDial(exprDial, exprLabel, "Expression", 0.0f, 1.0f, audioProcessor.staticExpression, false);

        setupDial(delayDial, delayLabel, "Delay Time", 0.0f, 1.0f, audioProcessor.staticDelay, false);
        setupDial(distDial, distLabel, "Distortion", 0.0f, 1.0f, audioProcessor.staticDistortion, false);

        setupDial(cutoffDial, cutoffLabel, "Cutoff", 0.0f, 1.0f, audioProcessor.staticCutoff, false);
        setupDial(resDial, resLabel, "Resonance", 0.0f, 1.0f, audioProcessor.staticResonance, false);
        setupDial(attackDial, attackLabel, "Attack", 0.0f, 1.0f, audioProcessor.staticAttack, false);
        setupDial(releaseDial, releaseLabel, "Release", 0.0f, 1.0f, audioProcessor.staticRelease, false);

        setupDial(reverbDial, reverbLabel, "Reverb", 0.0f, 1.0f, audioProcessor.staticReverb, false);
        setupDial(chorusDial, chorusLabel, "Chorus", 0.0f, 1.0f, audioProcessor.staticChorus, false);
        setupDial(vibDial, vibLabel, "Vibrato", 0.0f, 1.0f, audioProcessor.staticVibrato, false);
        setupDial(waveDial, waveLabel, "Waveform", 0.0f, 1.0f, audioProcessor.staticWaveform, true);

        auto sendGlobalOSC = [this](GestureTarget target, float value) {
            if (audioProcessor.currentOutputMode == OutputMode::OSC_Only) {
                audioProcessor.oscManager.routeMessage(
                    target, value, "global",
                    audioProcessor.rootNote, audioProcessor.scaleType, audioProcessor.octaveRange, audioProcessor.currentRangeMode, audioProcessor.startNote, audioProcessor.endNote
                );
            }
            };

        volumeDial.onValueChange = [this, sendGlobalOSC] {
            audioProcessor.staticVolume = (float)volumeDial.getValue();
            sendGlobalOSC(GestureTarget::Volume, audioProcessor.staticVolume);
            };
        panDial.onValueChange = [this, sendGlobalOSC] {
            audioProcessor.staticPan = (float)panDial.getValue();
            sendGlobalOSC(GestureTarget::Pan, audioProcessor.staticPan);
            };

        delayDial.onValueChange = [this, sendGlobalOSC] {
            audioProcessor.staticDelay = (float)delayDial.getValue();
            sendGlobalOSC(GestureTarget::Delay, audioProcessor.staticDelay);
            }; // <-- FIXED MISSING BRACKET

        modDial.onValueChange = [this, sendGlobalOSC] {
            audioProcessor.staticModulation = (float)modDial.getValue();
            sendGlobalOSC(GestureTarget::Modulation, audioProcessor.staticModulation);
            };

        distDial.onValueChange = [this, sendGlobalOSC] {
            audioProcessor.staticDistortion = (float)distDial.getValue();
            sendGlobalOSC(GestureTarget::Distortion, audioProcessor.staticDistortion);
            }; // <-- FIXED MISSING BRACKET

        exprDial.onValueChange = [this, sendGlobalOSC] {
            audioProcessor.staticExpression = (float)exprDial.getValue();
            sendGlobalOSC(GestureTarget::Expression, audioProcessor.staticExpression);
            };

        cutoffDial.onValueChange = [this, sendGlobalOSC] {
            audioProcessor.staticCutoff = (float)cutoffDial.getValue();
            sendGlobalOSC(GestureTarget::Cutoff, audioProcessor.staticCutoff);
            };
        resDial.onValueChange = [this, sendGlobalOSC] {
            audioProcessor.staticResonance = (float)resDial.getValue();
            sendGlobalOSC(GestureTarget::Resonance, audioProcessor.staticResonance);
            };
        attackDial.onValueChange = [this, sendGlobalOSC] {
            audioProcessor.staticAttack = (float)attackDial.getValue();
            sendGlobalOSC(GestureTarget::Attack, audioProcessor.staticAttack);
            };
        releaseDial.onValueChange = [this, sendGlobalOSC] {
            audioProcessor.staticRelease = (float)releaseDial.getValue();
            sendGlobalOSC(GestureTarget::Release, audioProcessor.staticRelease);
            };
        reverbDial.onValueChange = [this, sendGlobalOSC] {
            audioProcessor.staticReverb = (float)reverbDial.getValue();
            sendGlobalOSC(GestureTarget::Reverb, audioProcessor.staticReverb);
            };
        chorusDial.onValueChange = [this, sendGlobalOSC] {
            audioProcessor.staticChorus = (float)chorusDial.getValue();
            sendGlobalOSC(GestureTarget::Chorus, audioProcessor.staticChorus);
            };
        vibDial.onValueChange = [this, sendGlobalOSC] {
            audioProcessor.staticVibrato = (float)vibDial.getValue();
            sendGlobalOSC(GestureTarget::Vibrato, audioProcessor.staticVibrato);
            };
        waveDial.onValueChange = [this, sendGlobalOSC] {
            audioProcessor.staticWaveform = (float)waveDial.getValue();
            sendGlobalOSC(GestureTarget::Waveform, audioProcessor.staticWaveform);
            };

        addAndMakeVisible(closeButton);
        closeButton.setButtonText("Close");

        startTimerHz(60);
    }

    ~StaticDialsComponent() override {
        stopTimer();
        setLookAndFeel(nullptr);
    }

    void paint(juce::Graphics& g) override {
        g.fillAll(juce::Colours::black.withAlpha(0.95f));
        g.setColour(juce::Colours::white);
        g.setFont(24.0f);
        g.drawText("Static Dials", getLocalBounds().removeFromTop(60), juce::Justification::centred);
    }

    void resized() override {
        auto area = getLocalBounds().reduced(20);
        auto topBar = area.removeFromTop(40);
        closeButton.setBounds(topBar.removeFromRight(150));

        area.removeFromTop(10);

        int dialW = area.getWidth() / 4;
        int rowH = area.getHeight() / 3;

        auto layoutRow = [&](juce::Rectangle<int> rowArea,
            juce::Slider& s1, juce::Label& l1, juce::Slider& s2, juce::Label& l2,
            juce::Slider& s3, juce::Label& l3, juce::Slider& s4, juce::Label& l4)
            {
                auto a1 = rowArea.removeFromLeft(dialW); auto a2 = rowArea.removeFromLeft(dialW);
                auto a3 = rowArea.removeFromLeft(dialW); auto a4 = rowArea.removeFromLeft(dialW);

                s1.setBounds(a1.removeFromTop(rowH - 30).withSizeKeepingCentre(100, 100)); l1.setBounds(a1.withSizeKeepingCentre(100, 30));
                s2.setBounds(a2.removeFromTop(rowH - 30).withSizeKeepingCentre(100, 100)); l2.setBounds(a2.withSizeKeepingCentre(100, 30));
                s3.setBounds(a3.removeFromTop(rowH - 30).withSizeKeepingCentre(100, 100)); l3.setBounds(a3.withSizeKeepingCentre(100, 30));
                s4.setBounds(a4.removeFromTop(rowH - 30).withSizeKeepingCentre(100, 100)); l4.setBounds(a4.withSizeKeepingCentre(100, 30));
            };

        // Lay out the first row using the Volume and Pan, but use Mod/Expr for the layout math
        layoutRow(area.removeFromTop(rowH), volumeDial, volumeLabel, panDial, panLabel, modDial, modLabel, exprDial, exprLabel);

        // Now physically move the Delay and Dist dials to the EXACT same spot as the Mod/Expr dials
        delayDial.setBounds(modDial.getBounds());
        delayLabel.setBounds(modLabel.getBounds());

        distDial.setBounds(exprDial.getBounds());
        distLabel.setBounds(exprLabel.getBounds());

        // Continue with other rows...
        layoutRow(area.removeFromTop(rowH), cutoffDial, cutoffLabel, resDial, resLabel, attackDial, attackLabel, releaseDial, releaseLabel);
        layoutRow(area.removeFromTop(rowH), reverbDial, reverbLabel, chorusDial, chorusLabel, vibDial, vibLabel, waveDial, waveLabel);
    }

    void timerCallback() override {
        bool isOsc = (audioProcessor.currentOutputMode == OutputMode::OSC_Only);

        // Toggle OSC specific dials
        delayDial.setVisible(isOsc);
        delayLabel.setVisible(isOsc);
        distDial.setVisible(isOsc);
        distLabel.setVisible(isOsc);
        waveDial.setVisible(isOsc);
        waveLabel.setVisible(isOsc);

        // Toggle MIDI specific dials
        modDial.setVisible(!isOsc);
        modLabel.setVisible(!isOsc);
        exprDial.setVisible(!isOsc);
        exprLabel.setVisible(!isOsc);

        auto updateMotorizedDial = [&](juce::Slider& dial, std::atomic<float>* oscLive, std::atomic<float>* midiLive, float& staticVal) {
            if (dial.isMouseButtonDown()) {
                staticVal = (float)dial.getValue();
                return;
            }

            float currentLive = -1.0f;
            if (isOsc && oscLive != nullptr) currentLive = oscLive->load();
            else if (!isOsc && midiLive != nullptr) currentLive = midiLive->load();

            if (currentLive >= 0.0f) {
                dial.setValue(currentLive, juce::dontSendNotification);
                staticVal = currentLive;
            }
            else {
                dial.setValue(staticVal, juce::dontSendNotification);
            }
            };

        updateMotorizedDial(volumeDial, &audioProcessor.oscManager.liveVolume, &audioProcessor.midiManager.liveVolume, audioProcessor.staticVolume);
        updateMotorizedDial(panDial, &audioProcessor.oscManager.livePan, &audioProcessor.midiManager.livePan, audioProcessor.staticPan);

        updateMotorizedDial(delayDial, &audioProcessor.oscManager.liveDelay, nullptr, audioProcessor.staticDelay);
        updateMotorizedDial(distDial, &audioProcessor.oscManager.liveDistortion, nullptr, audioProcessor.staticDistortion);

        updateMotorizedDial(cutoffDial, &audioProcessor.oscManager.liveCutoff, &audioProcessor.midiManager.liveCutoff, audioProcessor.staticCutoff);
        updateMotorizedDial(resDial, &audioProcessor.oscManager.liveResonance, &audioProcessor.midiManager.liveResonance, audioProcessor.staticResonance);
        updateMotorizedDial(attackDial, &audioProcessor.oscManager.liveAttack, &audioProcessor.midiManager.liveAttack, audioProcessor.staticAttack);
        updateMotorizedDial(releaseDial, &audioProcessor.oscManager.liveRelease, &audioProcessor.midiManager.liveRelease, audioProcessor.staticRelease);

        updateMotorizedDial(reverbDial, &audioProcessor.oscManager.liveReverb, &audioProcessor.midiManager.liveReverb, audioProcessor.staticReverb);
        updateMotorizedDial(chorusDial, &audioProcessor.oscManager.liveChorus, &audioProcessor.midiManager.liveChorus, audioProcessor.staticChorus);
        updateMotorizedDial(vibDial, &audioProcessor.oscManager.liveVibrato, &audioProcessor.midiManager.liveVibrato, audioProcessor.staticVibrato);
        updateMotorizedDial(waveDial, &audioProcessor.oscManager.liveWaveform, &audioProcessor.midiManager.liveWaveform, audioProcessor.staticWaveform);

        updateMotorizedDial(modDial, nullptr, &audioProcessor.midiManager.liveModulation, audioProcessor.staticModulation);
        updateMotorizedDial(exprDial, nullptr, &audioProcessor.midiManager.liveExpression, audioProcessor.staticExpression);
    }

    juce::TextButton closeButton;

private:
    GestureInstrumentAudioProcessor& audioProcessor;

    juce::Slider volumeDial; juce::Slider panDial;
    juce::Slider modDial; juce::Slider exprDial;
    juce::Slider delayDial; juce::Slider distDial;
    juce::Slider cutoffDial; juce::Slider resDial;
    juce::Slider attackDial; juce::Slider releaseDial;
    juce::Slider reverbDial; juce::Slider chorusDial;
    juce::Slider vibDial; juce::Slider waveDial;

    juce::Label volumeLabel; juce::Label panLabel;
    juce::Label modLabel; juce::Label exprLabel;
    juce::Label delayLabel; juce::Label distLabel;
    juce::Label cutoffLabel; juce::Label resLabel;
    juce::Label attackLabel; juce::Label releaseLabel;
    juce::Label reverbLabel; juce::Label chorusLabel;
    juce::Label vibLabel; juce::Label waveLabel;

    void setupDial(juce::Slider& dial, juce::Label& label, juce::String name, float min, float max, float startVal, bool isWaveform) {
        addAndMakeVisible(dial);
        dial.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        dial.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);

        if (isWaveform) {
            dial.textFromValueFunction = [](double value) {
                if (value < 0.25) return juce::String("Sine");
                if (value < 0.50) return juce::String("Triangle");
                if (value < 0.75) return juce::String("Saw");
                return juce::String("Square");
                };
        }

        dial.setRange(min, max, 0.01);
        dial.setValue(startVal);
        dial.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::orange);
        dial.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);

        addAndMakeVisible(label);
        label.setText(name, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    }

    NiceDialLookAndFeel customDialLook;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StaticDialsComponent)
};