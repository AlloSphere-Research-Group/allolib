#ifndef AL_TIME_MASTER_MODE_HPP
#define AL_TIME_MASTER_MODE_HPP

/**
 * @file al_TimeMasterMode.hpp
 * @brief Which App callback (or thread) advances sequenced time.
 *
 * Used by PolySynth / DynamicScene / SynthSequencer / PresetHandler — not by
 * Parameter value types. Lives in al::types so scene and parameter modules
 * can share it without pulling the full Parameter header.
 *
 * TIME_MASTER_AUDIO    — advance in onSound / audio callback
 * TIME_MASTER_GRAPHICS — advance in onDraw / render
 * TIME_MASTER_UPDATE   — advance in onAnimate / update
 * TIME_MASTER_CPU      — dedicated CPU thread
 * TIME_MASTER_FREE     — caller drives stepping explicitly (e.g. tests, GUI)
 */

namespace al {

enum class TimeMasterMode {
  TIME_MASTER_AUDIO,
  TIME_MASTER_GRAPHICS,
  TIME_MASTER_UPDATE,
  TIME_MASTER_FREE,
  TIME_MASTER_CPU
};

} // namespace al

#endif
