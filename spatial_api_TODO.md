# Spatial audio API — TODO / known issues

Status for `al::Scene` spatial path (`enableSpatialAudio` / `renderAudioSpatial`) and the
classic panners under `allolib/modules/audio`. Interactive consumer today:
`hydrogen_allolib` `atom_view` (StereoPanner on ≤2 outs, Lbap + Allosphere on >2).

Related code:

| Piece | Path |
|-------|------|
| Scene dry / spatial mix | `allolib/modules/scene/src/scene/al_Scene.cpp` |
| DynamicScene (legacy) | `allolib/modules/scene/src/scene/al_DynamicScene.cpp` |
| Stereo / DBAP / LBAP / VBAP / Ambisonics | `allolib/modules/audio/src/sound/al_*.cpp` |
| Speaker layouts | `al_Speaker.cpp`, `al_AlloSphereSpeakerLayout*` |
| Cult DBAP normalize (reference) | https://github.com/Cult-DSP/cult-allolib (`src/sound/al_Dbap.cpp`, Lucian Parisi) |

---

## Done recently

- [x] **Listener world→local** — Scene (and DynamicScene) use `quat.rotateTransposed(direction)` so yaw matches graphics (`+X` right, `+Y` up, `−Z` forward).
- [x] **StereoPanner pan** — replaced `fabs(atan2(z,x))` with lateral `x/|xz|` equal-power pan in listener-local OpenGL frame.
- [x] **AudioIO device buffer mismatch** — resize buffers when RtAudio negotiates a different block size (heap corruption / abort).
- [x] **Scene re-prepare** — reconfigure internal voice bus if `framesPerBuffer` changes mid-run.

---

## P0 — correctness / safety

### Shared Scene path

- [ ] **Mono source for spatializers** — Hydrogen / Scene currently spatialize each entity output channel at the same pose (`numOutChannels == 2` → ~2× level / wrong for pan laws that assume mono). Prefer mono bus into the spatializer (sum or take ch0); keep stereo only for dry path.
- [ ] **Document / enforce coordinate contract** — listener-local OpenGL into Scene; DBAP/VBAP remap `(x,−z,y)` to allocore speaker space (`Speaker::vec()`). Single helper e.g. `graphicsToSpeaker(Vec3f)` used by every panner; unit-test identity / 90° yaw cases.
- [ ] **Distance attenuation vs panner** — Scene optional `DistAtten` vs DBAP’s own `1/(1+d)`. Policy: atten in one place only; document per spatializer.

### DBAP

- [ ] **Missing L2 power normalization** — current tree applies raw `(1/(1+d))^focus` with no normalize → total power drops as focus↑ or source moves. Port Cult-DSP max-scaled L2 so `∑v_k² = 1` (Lossius et al., ICMC 2009 eq. 2). Optional flag `normalize(bool)` default **on**.
- [ ] **Focus clamp** — Cult clamps focus ≥ 0.1 (negative focus inverts distance). Mirror that.
- [ ] **Bounds check** — reject / assert layouts with `> DBAP_MAX_NUM_SPEAKERS` (Cult throws).

### LBAP

- [ ] **`renderSample` unimplemented** — `assert(0 == 1)`. Implement or clearly `#error` / return silence in release.
- [ ] **`bufferSize` vs `numFrames`** — between-ring path uses `prepare()`’s `bufferSize`; if device renegotiates block size, gains/buffers desync. Realloc in `prepare` / always use `numFrames`.
- [ ] **Zenith/nadir dispersion mutates shared `io.out`** — `*=` on device channels after other sources may already be mixed → multi-source bug. Accumulate into a scratch bus then add.
- [ ] **Rename `LdapRing` → `LbapRing`** (typo).

### VBAP

- [ ] **`speakerChan[2] = s2Chan` typo** — should be `s3Chan` in `SpeakerTriple::loadVectors`.
- [ ] **Cached triplet index** — always searches from 0 (`FIXME AC store cached index`). Cache last hit per source (or Scene-side cache) for CPU.
- [ ] **Phantom channel gain** — `splitGain = gains[i] / mPhantomChannels.size()` divides by map size, not assignee count. Fix + test.
- [ ] Confirm 2D vs 3D compile path for Allosphere rings (LBAP embeds VBAP per ring).

### Ambisonics

- [ ] **Normalize direction before encode** — `AmbiEncode::direction(Vec3f)` docs require unit vector; Scene passes full `listeningDir` (distance leaks into SH weights). Normalize (or pass unit + separate distance).
- [ ] **Axis / FuMa convention** — commented remaps (`(−z,−x,y)` etc.) vs raw OpenGL. Lock one convention, document, test encode→decode identity for a front source on a known layout.
- [ ] **`prepare` assumes fixed buffer size** — first-callback allocate; reallocate if `framesPerBuffer` changes (same class of bug as LBAP / AudioIO).

### StereoPanner

- [ ] Speaker layout angles unused (decorative). Either drive pan from layout azimuths or document “layout ignored, OpenGL lateral pan only.”
- [ ] Optional: front-biased / constant-power variants; keep current as default.

---

## P1 — API ergonomics (switch spatializers without breaking)

- [ ] **Runtime spatializer switch on `al::Scene`** — e.g. `setSpatializerKind(enum)` or keep `enableSpatialAudio<T>(Speakers)` but add a small facade that owns layout + kind and can rebuild without tearing down the Scene.
- [ ] **Layout presets** — `StereoSpeakerLayout`, `AlloSphereSpeakerLayoutCompensated`, thin/horizontal variants selectable with the method.
- [ ] **GUI dropdown** (atom_view and/or dedicated test app) — Stereo | DBAP | LBAP | VBAP | Ambisonics (+ order/dim for ambi). Changing selection recompiles spat; does not stop transport.
- [ ] **TOML / env override** — e.g. `[audio] spatializer = "lbap"` for sphere deploy without rebuilding.
- [ ] **Dry mix escape hatch** — explicit “Off / dry sum” for debugging.

---

## P2 — tests (automated)

Target: `allolib/modules/audio` (or `allolib/test`) + Scene-level tests. Prefer headless AudioIOData fixtures (existing pattern in `test_lbap.cpp`, `test_vbap.cpp`, `test_dynamicScene.cpp`).

### After each fix, add / extend

- [ ] **Listener transform** — source at listener-left (−X); identity pose → left channel energy; after +90° yaw, energy moves as expected (regression for `rotate` vs `rotateTransposed`).
- [ ] **StereoPanner** — hard L / hard R / center; energy conservation within tolerance.
- [ ] **DBAP** — with normalize on: `∑g² ≈ 1` at several positions and focus values; without normalize: document old behavior if kept as option. Port Cult cases if any.
- [ ] **DBAP** — focus clamp; oversized layout rejected.
- [ ] **VBAP** — 3D triplet containing a known interior point; gains ≥ 0; `speakerChan[2]` regression; phantom assignee count.
- [ ] **LBAP** — between two elevation rings; elevation above top / below bottom; `numFrames` ≠ prepared size; multi-source dispersion does not scale prior content.
- [ ] **Ambisonics** — unit vs non-unit direction; encode→decode front source peaks on expected speaker(s) for a simple layout.
- [ ] **Scene buffer renegotiate** — prepare @ 256, render @ 512 (or mock); no overrun / correct mix length.
- [ ] **Coord helper** — `graphicsToSpeaker` round-trip / known vectors.

---

## P3 — interactive sphere / laptop test app

Goal: validate spatializers in the Allosphere (and stereo laptop) with controllable sources — not just unit tests.

### App sketch (`spatial_lab` or `allolib` example)

- [ ] **AppBuilder** recipe: local stereo + distributed Motu / Allosphere (`distributed_app.toml`).
- [ ] **Spatializer + layout dropdown** (P1).
- [ ] **Sources** — N positioned entities (start with 1–4):
  - broadband noise burst / pink noise
  - pulsed click train (localization)
  - swept sine / band-limited noise
  - optional: one HydrogenAtom or simple oscillator for “musical” check
- [ ] **GUI positioning** — intuitive placement:
  - 3D gizmo or azimuth / elevation / distance sliders
  - optional: click-drag on a top-down ring + elevation slider
  - “snap to speaker” / “orbit listener”
  - mirror listener = nav (keyboard look already) vs locked listener at origin
- [ ] **Meters** — per-channel peak/RMS (especially 64ch sphere); highlight active speakers for current spat.
- [ ] **Solo / mute source**, freeze position, A/B spat without moving sources.
- [ ] **Log line** — spat name, layout, outs, buffer, SR, listener pose.
- [ ] Deploy script / README: run on ar00 vs laptop dry-run (channels clamped → Stereo).

---

## Reference notes

### Cult DBAP normalize (summary)

Unnormalized weights `w_i = (1/(1+d_i))^focus`, then max-scaled L2:

`kNorm = 1 / (maxW * sqrt(∑(w_j/maxW)²))` → gains `kNorm * w_i` with `∑g² = 1`.

See commits on `Cult-DSP/cult-allolib` path `src/sound/al_Dbap.cpp` (2026-04).

### Coordinate cheat sheet

| Space | Axes |
|-------|------|
| Pose / graphics / Scene listeningDir | `+X` right, `+Y` up, `−Z` forward |
| `Speaker::vec()` (allocore / many panners) | Y-forward, Z-up (see comment in `al_Speaker.cpp`) |
| Common remap in DBAP/VBAP | `(x,y,z)_gl → (x, −z, y)_spk` |

---

## Suggested order of work

1. Coord helper + Scene mono-into-spat + Ambisonics normalize direction.  
2. Port Cult DBAP L2 + tests.  
3. LBAP buffer/`renderSample`/dispersion scratch; VBAP typo + phantom.  
4. Spatializer switch API + atom_view / `spatial_lab` dropdown.  
5. Full `spatial_lab` sphere app (sources + GUI pose + meters).  
6. Expand automated tests as each P0 item lands.
