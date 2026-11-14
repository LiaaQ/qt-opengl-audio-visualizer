# Vaporwave Audio Visualizer

An audio visualizer built with Qt 6 (QML) and OpenGL. Load a song and fly through a neon vaporwave world whose terrain ripples to the music.

> **Status:** v0.0 (https://github.com/LiaaQ/qt-opengl-audio-visualizer/releases/tag/v0.0)
> <img width="985" height="768" alt="image" src="https://github.com/user-attachments/assets/afe44eca-a8df-4572-a146-d1e36b47e6b4" />

## Vision

- A vaporwave world: grid terrain, a bright sun, a gradient sky
- Different parts of the world react to different frequency bands
- *(Later)* Solo any frequency band or change its volume, and watch the world respond

## Tech stack

| Part | Choice |
|---|---|
| UI | Qt 6.10 / QML |
| Build | CMake |
| Audio (v0.0 and v0.1) | `MediaPlayer` + `QAudioBufferOutput` (Qt Multimedia) |
| Audio (per-band mixer) | `QAudioDecoder` + `QAudioSink` with a custom `QIODevice` |
| FFT | Small radix-2 FFT (or [KissFFT](https://github.com/mborgerding/kissfft)) |
| Graphics | `QQuickFramebufferObject` - OpenGL with GLSL shaders |

## Releases

### v0.0: audio in, 2D bars
- [ ] Open an audio file (mp3 / wav / flac / ogg)
- [ ] Play, pause, position slider
- [ ] FFT on the playing audio, grouped into **8 bands** (bass to treble)
- [ ] 8 QML bars that move with the music

### v0.1: the vaporwave world (the MVP)
- [ ] One OpenGL scene: sky gradient, a bright sun, a scrolling neon grid
- [ ] Terrain height ripples, sun pulsing driven by the band levels, smoothed so it doesn't flicker

### v0.2: more world
- [ ] Hills from noise and a flat road down the middle
- [ ] Stars, a better palette, a seek slider

### After that: per-band volume and solo
Replace `MediaPlayer` with `QAudioDecoder` and `QAudioSink`, and process playback with an STFT: FFT in overlapping windows (sqrt-Hann, 50% overlap), multiply the bins by per-band gains, inverse FFT, overlap-add. Build the band masks so they sum to 1, so all gains at 1 is a perfect passthrough, and solo is gain 1 for one band and 0 for the rest. Then add a mixer UI with sliders and solo buttons, and make the visuals follow the gains.

## Out of scope for now
- Camera controls, settings screens
- Packaging and installers

## Building

Requirements: Qt 6.10 (with the Multimedia, Quick, and OpenGL modules), CMake 3.21+, and a C++20 compiler.

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH=/path/to/Qt/6.8.x/<kit>
cmake --build build
./build/audio_visualizer
```

## License

TBD
