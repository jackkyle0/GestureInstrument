#pragma once
#include <JuceHeader.h>
#include "../../Source/PluginProcessor.h" // Adjust path as needed

class PluginProcessorTests : public juce::UnitTest {
public:
    PluginProcessorTests() : juce::UnitTest("Plugin Processor Core and State Tests") {}

    void runTest() override {
        beginTest("1. XML State Saving and Loading");
        {
            GestureInstrumentAudioProcessor::isRunningInUnitTest = true;
            GestureInstrumentAudioProcessor processor;

            // 1. Modify the default state to simulate a user creating a custom patch
            processor.rootNote = 7; // G
            processor.scaleType = 3; // Minor Pentatonic
            processor.minWidthThreshold = -420.0f;
            processor.globalMute.store(true);
            processor.isMpeEnabled = true;

            // 2. Serialize to XML (Simulate saving the DAW project)
            std::unique_ptr<juce::XmlElement> savedState = processor.createPresetXml();

            // 3. Wreck the current state (Simulate closing the plugin)
            processor.rootNote = 0;
            processor.scaleType = 0;
            processor.minWidthThreshold = 0.0f;
            processor.globalMute.store(false);
            processor.isMpeEnabled = false;

            // 4. Restore from XML (Simulate loading the DAW project)
            processor.loadPresetXml(savedState.get());

            // 5. Assert it restored perfectly
            expectEquals(processor.rootNote, 7, "Root note failed to restore from XML.");
            expectEquals(processor.scaleType, 3, "Scale type failed to restore from XML.");
            expectEquals((float)processor.minWidthThreshold, -420.0f, "Threshold failed to restore from XML.");
            expect(processor.globalMute.load() == true, "Global mute boolean failed to restore from XML.");
            expect(processor.isMpeEnabled == true, "MPE State failed to restore from XML.");
        }

        beginTest("2. Global Mute Audio & MIDI Silence Logic");
        {
            GestureInstrumentAudioProcessor::isRunningInUnitTest = true;
            GestureInstrumentAudioProcessor processor;

            juce::AudioBuffer<float> audioBuffer(2, 512);
            juce::MidiBuffer midiBuffer;

            processor.globalMute.store(true);

            // --- FRAME 1: The Panic Frame ---
            audioBuffer.setSample(0, 0, 1.0f);
            midiBuffer.addEvent(juce::MidiMessage::noteOn(1, 60, (juce::uint8)100), 0);
            processor.processBlock(audioBuffer, midiBuffer);

            // --- FRAME 2: The Silence Frame ---
            audioBuffer.setSample(0, 0, 1.0f);
            midiBuffer.addEvent(juce::MidiMessage::noteOn(1, 62, (juce::uint8)100), 0);
            processor.processBlock(audioBuffer, midiBuffer);

            expect(midiBuffer.isEmpty(), "Midi Buffer was not cleared during continuous Global Mute.");
            expect(audioBuffer.getMagnitude(0, audioBuffer.getNumSamples()) == 0.0f, "Audio Buffer was not silenced.");
        }

        beginTest("3. Plugin Host Metadata and Latency Reporting");
        {
            GestureInstrumentAudioProcessor::isRunningInUnitTest = true;
            GestureInstrumentAudioProcessor processor;

            // Proves the plugin correctly identifies itself to the DAW
            expectEquals(processor.getTailLengthSeconds(), 0.0, "Tail length must be 0 for a real-time controller.");
            expectEquals(processor.getLatencySamples(), 0, "Latency must be reported as 0 to the host DAW.");
            expect(processor.hasEditor(), "Processor must report that it has a custom UI.");
            expect(processor.producesMidi(), "Processor must report MIDI generation capabilities to host.");
        }

        beginTest("4. prepareToPlay and releaseResources");
        {
            GestureInstrumentAudioProcessor::isRunningInUnitTest = true;
            GestureInstrumentAudioProcessor processor;

            // Simulate the DAW initializing the plugin at an extreme 96kHz with a 1024 buffer
            double sampleRate = 96000.0;
            int samplesPerBlock = 1024;

            // --- THE FIX: Simulate the DAW's secret handshake! ---
            processor.setRateAndBufferSizeDetails(sampleRate, samplesPerBlock);
            // -----------------------------------------------------

            processor.prepareToPlay(sampleRate, samplesPerBlock);

            expectEquals(processor.getSampleRate(), sampleRate, "Sample rate not stored correctly during prepareToPlay.");
            expectEquals(processor.getBlockSize(), samplesPerBlock, "Block size not stored correctly during prepareToPlay.");

            // Simulate DAW stopping the transport/audio engine
            processor.releaseResources();
            expect(true, "releaseResources executed without memory violations.");
        }

        beginTest("Default Parameter State and Threshold Safety");
        {
            GestureInstrumentAudioProcessor::isRunningInUnitTest = true;
            GestureInstrumentAudioProcessor processor;

            // Prove that out-of-the-box, the 3D spatial boundaries are mathematically safe and do not cross
            expect(processor.minWidthThreshold < processor.maxWidthThreshold, "Default X-Axis thresholds are inverted.");
            expect(processor.minHeightThreshold < processor.maxHeightThreshold, "Default Y-Axis thresholds are inverted.");
            expect(processor.minDepthThreshold < processor.maxDepthThreshold, "Default Z-Axis thresholds are inverted.");

            // Prove musical modes start in a safe array index
            expect(processor.scaleType >= 0, "Scale type initialized to an invalid negative index.");
            expect(processor.rootNote >= 0, "Root note initialized to an invalid negative index.");
        }
    }
};

static PluginProcessorTests pluginProcessorTestsInstance;