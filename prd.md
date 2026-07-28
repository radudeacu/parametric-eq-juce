# PRD: Parametric EQ (JUCE)

## Goal
A 5-band parametric equalizer plugin with a built-in real-time (pre-EQ) spectrum analyzer. V1 focuses on a solid processing/parameter "skeleton" using JUCE's default UI components — visual polish and advanced features come later.

## Features

### Bands (fixed count: 5)
- Band 1 (Low): Type switchable — High-Pass / Low Shelf. Freq 20–500 Hz (default 80 Hz). Gain -24 to +24 dB (default 0 dB, ignored in High-Pass mode). Q 0.1–10 (default 0.707). Slope selector: 12/24/36 dB/oct (default 12), applies in both modes.
- Band 2 (Low-Mid): Type fixed Bell/Peak. Freq 20–20,000 Hz (default 300 Hz). Gain -24 to +24 dB (default 0 dB). Q 0.1–10 (default 1.0).
- Band 3 (Mid): Type fixed Bell/Peak. Freq 20–20,000 Hz (default 1,000 Hz). Gain -24 to +24 dB (default 0 dB). Q 0.1–10 (default 1.0).
- Band 4 (High-Mid): Type fixed Bell/Peak. Freq 20–20,000 Hz (default 3,000 Hz). Gain -24 to +24 dB (default 0 dB). Q 0.1–10 (default 1.0).
- Band 5 (High): Type switchable — Low-Pass / High Shelf. Freq 500–20,000 Hz (default 8,000 Hz). Gain -24 to +24 dB (default 0 dB, ignored in Low-Pass mode). Q 0.1–10 (default 0.707). Slope selector: 12/24/36 dB/oct (default 12), applies in both modes.

### Global Controls
- Output Gain/Trim: -24 to +24 dB (default 0 dB).
- Bypass: on/off toggle (default off).

### Spectrum Analyzer
- Displays pre-EQ signal only.
- Real-time, continuously updating (no freeze/hold in V1).

## Behaviour
- Gain parameter has no audible effect while a switchable band is in Pass-filter mode (High-Pass/Low-Pass), but the control remains visible/settable for when the user switches back to shelf mode.
- Slope selector affects filter rolloff steepness for bands 1 and 5 regardless of whether they're in Pass or Shelf mode.
- Bypass mutes all processing (dry passthrough) but the spectrum analyzer keeps running on the pre-EQ signal.
- Plugin formats: Standalone (for testing) and AAX only. No VST3/AU in V1.
- Target platforms: Windows (primary dev/test) and macOS.

## Out of Scope (for V1)
- Preset system / preset browser
- Per-band solo/mute
- Undo/redo for parameter changes
- Oversampling
- Custom UI graphics (draggable EQ curve nodes, etc.) — using JUCE default components for V1
