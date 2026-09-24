#ifndef AL_COMPONENT_HPP
#define AL_COMPONENT_HPP

/**
 * @file al_Component.hpp
 * @brief Base Component for the Entity Component Scene API.
 *
 * Components are swappable affordances (draw, audio, motion, …). They may own
 * Parameters (event/OSC/composition) and/or a POD via StatefulComponent
 * (per-frame packState sync).
 */

#include <cstring>

#include "al/graphics/al_Graphics.hpp"
#include "al/io/al_AudioIOData.hpp"

namespace al {

class Entity;

struct Component {
  Entity *entity{nullptr};

  virtual ~Component() = default;

  virtual void init() {}
  virtual void onTriggerOn() {}
  virtual void onTriggerOff() {}
  virtual void onFree() {}
  virtual void update(double dt) { (void)dt; }
  virtual void onAudio(AudioIOData &io) { (void)io; }
  virtual void onDraw(Graphics &g) { (void)g; }

  /// Bytes of POD state for packState (0 = none).
  virtual size_t stateSize() const { return 0; }
  virtual void packState(char *buf) const { (void)buf; }
  virtual void unpackState(const char *buf) { (void)buf; }
};

/**
 * Component with memcpy-able POD \p TState for cuttlebone-style sync.
 * TState must be trivially copyable (no pointers / heap members).
 */
template <typename TState> struct StatefulComponent : Component {
  TState state{};

  size_t stateSize() const override { return sizeof(TState); }

  void packState(char *buf) const override {
    std::memcpy(buf, &state, sizeof(TState));
  }

  void unpackState(const char *buf) override {
    std::memcpy(&state, buf, sizeof(TState));
  }
};

} // namespace al

#endif
