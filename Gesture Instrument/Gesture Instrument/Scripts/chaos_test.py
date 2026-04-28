from pedalboard import load_plugin
import numpy as np
import random
import time
import os

print("Loading Gesture Instrument...")
# --- THE BULLETPROOF DLL FIX ---
# 1. Remember where the script started
original_dir = os.getcwd()

# 2. Tell Python to physically move inside your VST3 folder
vst3_folder = r"C:\Users\Student\Desktop\Jacks Desktop\GestureInstrument\Gesture Instrument\Gesture Instrument\Builds\VisualStudio2022\x64\Debug\VST3\GestureInstrument.vst3\Contents\x86_64-win"
os.chdir(vst3_folder)

# 3. Load the plugin using just the filename, since we are now sitting in the same folder as it!
plugin = load_plugin("GestureInstrument.vst3")

# 4. Step back out to the original directory so the script can run normally
os.chdir(original_dir)

# 2. Create some empty audio to feed the plugin (simulate playback)
# 2 channels (stereo), 512 samples per block
empty_audio_block = np.zeros((2, 512))
sample_rate = 44100.0

# 3. The Monkey Loop
iterations = 1000
start_time = time.time()

try:
    for i in range(iterations):
        # --- A. RAPIDLY CHANGE SETTINGS ---
        
        # Randomly flip the master chord engine switch on or off
        # (Note: pedalboard automatically converts parameter names to snake_case)
        if "chord_engine_enabled" in plugin.parameters:
            plugin.chord_engine_enabled = random.choice([True, False])
        
        # Violently throw the inversion mode between 0, 1, and 2
        if "chord_inversion_mode" in plugin.parameters:
            plugin.chord_inversion_mode = random.choice([0.0, 1.0, 2.0])

        # Throw the volume and sensitivity to random extremes
        if "sensitivity_level" in plugin.parameters:
            plugin.sensitivity_level = random.uniform(0.0, 1.0)
            
        # --- B. PROCESS AUDIO (FORCE THE ENGINE TO UPDATE) ---
        # This is where it will crash if your threads collide!
        processed_audio = plugin(empty_audio_block, sample_rate)

        # Print progress every 100 iterations
        if i % 100 == 0:
            print(f"Chaos iteration {i}/{iterations} complete...")

    print("\n--- TEST PASSED ---")
    print(f"Plugin survived {iterations} chaotic parameter changes in {round(time.time() - start_time, 2)} seconds without crashing!")

except Exception as e:
    print("\n*** TEST FAILED! THE PLUGIN CRASHED ***")
    print(f"Error: {e}")