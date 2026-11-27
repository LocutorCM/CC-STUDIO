# CC Studio

A pink-glass inspired stereo creative reverb plugin built with JUCE 8 and C++.

## Formats
- VST3
- Audio Unit (AU, macOS)
- Standalone

## Reverb spaces
- Dream
- Vintage
- Dark Hall
- Crystal
- Ethereal

## Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

CMake fetches JUCE 8 from its official GitHub repository. On macOS, AU and VST3 targets are enabled; on Windows/Linux, VST3 and Standalone are built.

## Status
Initial development build. The DSP is a creative algorithm based on JUCE's stereo reverb network, with pre-delay, tone shaping, modulation, and five distinct voicings.
