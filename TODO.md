# AlloLib v3 module work

## 1. FILE_SET / header completeness (later)

Install currently globs `modules/*/include` so consumers compile despite incomplete
FILE_SET lists. That glob should go away once inventories match reality.

Per module: compare disk vs `FILE_SET HEADERS` vs actual `#include` usage.

Known gaps:

- **math:** 6 listed / 16 on disk (`Random`, `Functions`, `Ray`, …). Examples use them.
- **types:** `Color` + `VariantValue` listed; missing `Conversion`, `ValueSource`, `Buffer`, …
- Same audit for graphics, ui, app, io, …

**Hard spot:** `al_Random.hpp` → `al/types/al_Conversion.hpp`. Avoid a public
`math ↔ types` cycle. Options:

1. `math` PUBLIC → `types`, `types` PRIVATE → `math` (Color.cpp only)
2. Move `Conversion` into math or a tiny leaf (`al::conversion`)

Do not block module splits on this. Keep the install glob until FILE_SETs are honest.

---

## 2. Module boundaries (current focus)

Goal: small, reusable libraries that can live outside a kitchen-sink AlloLib app —
audio/video/distributed building blocks, not sphere-specific framework.

### Audio (done — refine later)

```
al::audio          DSP, ambisonics, spatializers, sound file  (uses AudioIOData)
al::audio_io       AudioIO + AudioIOData public API
al::audio_backend_rtaudio | portaudio | dummy
al::audio_device   INTERFACE: audio_io + one selected backend
```

One cache string `ALLOLIB_AUDIO_BACKEND` instead of two booleans.
No `alaudio ↔ alaudiobackend` cycle: backends depend on `audio_io`; apps link
`al::audio_device` (or `audio_io` + a backend).

### Parameter / composition (done — rewrite internals later)

`modules/parameter` / `al::parameter`: Parameter, bundles, OSC server, presets,
sequences, composition. No graphics, imgui, or App.

Header paths stay `al/ui/al_Parameter.hpp` until a later rename to `al/parameter/`.
Implementation (casts, type switches, VariantValue soup) is a follow-up rewrite.

### Scene (done — graphics coupling later)

`modules/scene` / `al::scene`: PolySynth, DynamicScene, voices, sequencers.
Depends on `al::parameter` + `al::audio`. Still PUBLIC-links `al::graphics`
because SynthVoice/PolySynth include `al_Graphics.hpp` — decouple later so
headless scene use does not pull OpenGL.

### UI

`al::ui` — graphics-space interaction + imgui bindings. Depends on parameter
and scene (widgets wrap scene types).

### Sphere (extracted; al_ext later)

`modules/sphere` / `al::sphere`: AlloSphere speakers, per-projection, meter,
host utils. Header paths stay `al/sphere/*`. App still PUBLIC-links it so
OmniRenderer / DistributedApp keep working. May move to **al_ext** later.

`NodeConfiguration` lives in `al::system` (`al/system/al_NodeConfiguration.hpp`);
`al/app/al_NodeConfiguration.hpp` is a compatibility include.

### App / domains

`al::Runtime` owns the asynchronous domain graph and start/stop/cleanup.
`App` is the default desktop recipe (OSC + Gamma audio + OpenGL + simulation);
`DistributedApp` uses the same Runtime. Quit lives on Runtime / AsynchronousDomain,
not only graphics.

Still to do:

- Small **components / domains** that compose without inheriting App
- Drop remaining RTTI wiring in `initializeDomains()`
- Window accessors off App

---

## 3. After boundaries

- Drop glob header install once FILE_SETs are complete
- `al_ext`: `al_ext_add()`, finish `al::video`, optionally take sphere
- Playground / template: `al::app` + explicit extensions
- Longer term: vcpkg/Conan → drop `AlloLibBundledTargets`
