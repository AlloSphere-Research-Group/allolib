#ifndef AL_SCENE_STATE_BLOB_HPP
#define AL_SCENE_STATE_BLOB_HPP

/**
 * @file al_SceneStateBlob.hpp
 * @brief Fixed POD buffer for Scene::packState / unpackState over the wire.
 *
 * Sized for StateBackend::Cuttlebone (fragmented UDP). Do NOT ship this over
 * OscBlob — sizeof exceeds osc::Send's default 1024-byte buffer.
 *
 * capacity is chosen so sizeof(SceneStateBlob) fits under
 * StateSyncLimits::kCuttleboneComfortableState (header + alignas padding).
 */

#include <cstdint>
#include <cstring>

#include "al/scene/al_Scene.hpp"

namespace al {

struct SceneStateBlob {
  /// Leave room for `size` + alignas padding under a 256 KiB cuttlebone budget.
  static constexpr uint32_t capacity = 256 * 1024 - 64;
  uint32_t size{0};
  alignas(8) char data[capacity]{};

  void clear() {
    size = 0;
    std::memset(data, 0, capacity);
  }

  /// Pack scene into this blob. Returns false if truncated.
  bool packFrom(const Scene &scene) {
    size_t n = scene.packState(data, capacity);
    size = static_cast<uint32_t>(n);
    return n > 0 && n <= capacity;
  }

  void applyTo(Scene &scene) const {
    if (size > 0 && size <= capacity) {
      scene.unpackState(data, size);
    }
  }
};

} // namespace al

#endif
