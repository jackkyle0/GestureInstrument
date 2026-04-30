# Gesture-based Musical Instrument Controller
A gesture-controlled MIDI and OSC instrument using the Leap Motion Controller.
------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
# Prerequisites and Hardware Requirements
Hardware: Ultraleap Motion Controller (Compatible with Leap motion LM-010 controller).

Drivers: You must download and install the official Ultraleap Orion Version 4.1.0. Location: https://www.ultraleap.com/downloads/leap-controller/

Note: Ensure your Leap Motion controller is plugged in and the tracking service is running in your system before launching the plugin.

------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
# Setup Instructions: DAW (VST3)
Download the latest GestureInstrument Plugin release package.

Locate your system's default VST3 directory (Windows Only):

Windows: C:\Program Files\Common Files\VST3

Copy the GestureInstrument.vst3 file into this directory.

Crucial Step: The plugin requires the Ultraleap C-API to function. Ensure the included LeapC.dll (Windows) is copied into the same directory alongside the plugin.

Open your DAW scan for new plugins, and load GestureInstrument onto a MIDI track!

------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
# Setup Instructions: Standalone App
Included in the release package is a Standalone version of the application, allowing you to play the instrument without needing a host DAW.

Launch the GestureInstrument standalone executable.

Ensure your audio hardware is configured via the standard Audio/MIDI settings menu.

The Standalone mode includes a built-in synthesizer with generic sound selections (Piano, Pads, Sci-Fi, etc.) so you can start making music immediately!

------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
# OSC Mode (Pure Data Integration)
GestureInstrument features a dedicated OSC (Open Sound Control) output mode, allowing you to route gesture data directly into external visualisers or visual programming languages like Pure Data over a local network.

To use OSC Mode:

Download and install Pure Data (Windows). Location: https://puredata.info/downloads/pure-data

Navigate to the Resources folder included in the GestureInstrument download package.

Open the provided OSC.pd file in Pure Data.
Ensure the DSP button is selected.

Launch GestureInstrument (either in your DAW or as a Standalone App).

Open the Settings menu inside the plugin and change the Output Mode from MIDI to OSC.

Enjoy!