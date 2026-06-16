# Juanjah — Multitap Delay

A dub-style multi-tap delay Audio Unit / VST3 plugin built with JUCE.

## Features

- 3 independent taps, each with its own:
  - Tap tempo (independent BPM per tap)
  - Sync to BPM with note division (1/4, 1/8, 1/8D, 1/16, 1/32...)
  - Time, Level, Feedback, Filter (low-pass)
- LFO modulation (rate + depth) modulating all delay times
- Parallel distortion (drive + blend)
- Dry/Wet mix

## Installation

### Windows (VST3)

1. Download `Juanjah-Windows` artifact from the [Actions tab](../../actions) (or from a Release)
2. Unzip it
3. Copy `Juanjah.vst3` to `C:\Program Files\Common Files\VST3\`
4. Restart your DAW (Cakewalk Sonar, etc.)
5. Load as VST3 in your DAW

### macOS (AU / VST3)

1. Download `Juanjah-macOS` artifact
2. Copy `Juanjah.vst3` to `~/Library/Audio/Plug-Ins/VST3/`
3. Copy `Juanjah.component` to `~/Library/Audio/Plug-Ins/Components/`
4. Restart your DAW

## Building from source

Requires:
- CMake 3.27+
- C++20 compiler (Xcode 16+ on macOS, Visual Studio 2022 on Windows)
- JUCE 8.0.13 (cloned automatically by the workflow, or run `git clone --depth 1 --branch 8.0.13 https://github.com/juce-framework/JUCE.git lib/JUCE`)

```bash
cmake -B build -S .
cmake --build build
```

## Project structure

- `source/PluginProcessor.cpp/h` — DSP and parameters
- `source/PluginEditor.cpp/h` — UI with custom-painted knobs
- `source/DelayLine.h` — circular delay line
- `source/LFO.h` — sine LFO
- `source/Distortion.h` — tanh soft-clip
- `source/OnePoleFilter.h` — one-pole low-pass filter
- `Resources/bg.png` — background image (not in git, add manually)
- `.github/workflows/build.yml` — CI builds for Win/Mac/Linux
