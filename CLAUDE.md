# parametric-eq-juce

## What This Is
A 5-band parametric EQ plugin with a real-time pre-EQ spectrum analyzer. See `prd.md` for full spec.

## Tech Stack
- **Framework:** JUCE
- **Language:** C++17
- **Build system:** CMake with CPM
- **Plugin formats:** Standalone, AAX only (no VST3/AU in V1)
- **Platforms:** Windows (primary dev/test), macOS
- Use `juce::dsp` FFT processor for the spectrum analyzer

## Coding Principles
- Single responsibility; small functions (~30 lines max).
- No god classes — each class is one clear concept.
- Composition over deep inheritance (1-2 levels max).
- Descriptive names over comments; comment why, not what.
- Fail early: validate inputs at function boundaries.

## Architecture Preferences
- Separate DSP (PluginProcessor) from UI (PluginEditor); communicate via APVTS, not direct coupling.
- Each filter band is its own DSP class; PluginProcessor composes 5 of them, not raw filter math.
- Implement `getStateInformation()`/`setStateInformation()` for session recall.

## Code Style
- JUCE conventions: camelCase members/methods, PascalCase classes. Explicit `juce::` prefix, no `using namespace juce;`.
- RAII; prefer `std::unique_ptr` over raw pointers.

## DSP Constraints
- No allocations or locks in `processBlock()`; pre-allocate in `prepareToPlay()`.
- Use `juce::SmoothedValue` for all audio-rate parameter changes.
- Use `juce::dsp` module processors before writing custom DSP.

## What NOT To Do
- Don't add third-party dependencies without asking.
- Don't put DSP logic in the editor/UI code.
- Don't over-engineer — flat architecture, no premature abstraction.
- Don't add features not specified in `prd.md` (no presets, solo/mute, undo/redo, oversampling, or custom UI graphics in V1).
