#ifndef INCLUDE_AL_STATE_SYNC_HPP
#define INCLUDE_AL_STATE_SYNC_HPP

/**
 * @file al_StateSync.hpp
 * @brief POD state replication as graphics Stages + pluggable backends.
 *
 * Vocabulary (see al_Module.hpp / TODO_local.md):
 *
 *   Module  — Runtime root (graphics, audio, osc)
 *   Stage   — Ordered work inside a Module tick
 *   Surface — Window / FBO owned by graphics
 *
 * State sync is NOT a Runtime Module. It is Stages on the graphics Module:
 *
 *   1. State recv (if replica)
 *   2. Simulation (owns the POD slot)
 *   3. Draw / surfaces
 *   4. State send (if primary)
 *
 * The POD slot lives on StateSimulationDomain / StateDistributionDomain.
 * A StateBackend is only how bytes move (cuttlebone, zmq, osc-blob, …).
 *
 * OscBlob is unsuitable for SceneStateBlob: default osc::Send buffer is
 * 1024 bytes and will throw "out of buffer memory". Prefer Cuttlebone.
 */

#include <cstddef>
#include <cstdint>
#include <functional>
#include <iostream>
#include <map>
#include <string>
#include <utility>

#include "al/app/al_StateDistributionDomain.hpp"
#include "al/scene/al_SceneStateBlob.hpp"

namespace al {

/// How POD state moves across the cluster.
enum class StateBackend : uint8_t {
  None = 0,
  /// Single OSC blob per frame. Tiny POD only — see StateSyncLimits.
  OscBlob,
  /// UDP fragmented broadcast (Allosphere default). Handles large fixed STATE.
  Cuttlebone,
  /// Reserved for PUB/SUB comparison backend.
  Zmq,
};

struct StateSyncConfig {
  StateBackend backend{StateBackend::Cuttlebone};
  std::string address{"127.0.0.1"};
  /// Cuttlebone default port; OscBlob historically used 10101.
  uint16_t port{63059};
  /// Wire fragment size (MTU-ish). Not the capacity of T.
  uint16_t packetBytes{1400};
  std::string id{"state"};
};

namespace StateSyncLimits {

/// Default osc::Send buffer (al_OSC.hpp). Blob + address + id must fit.
constexpr size_t kOscSendDefaultBuffer = 1024;
/// Leave headroom for "/_state", id string, OSC framing.
constexpr size_t kOscBlobMaxState = 768;
/// Comfortable LAN bound for cuttlebone-fragmented STATE (tune later).
constexpr size_t kCuttleboneComfortableState = 256 * 1024;

inline const char *backendName(StateBackend b) {
  switch (b) {
  case StateBackend::None:
    return "none";
  case StateBackend::OscBlob:
    return "osc-blob";
  case StateBackend::Cuttlebone:
    return "cuttlebone";
  case StateBackend::Zmq:
    return "zmq";
  }
  return "unknown";
}

inline size_t maxStateBytes(StateBackend b) {
  switch (b) {
  case StateBackend::OscBlob:
    return kOscBlobMaxState;
  case StateBackend::Cuttlebone:
    return kCuttleboneComfortableState;
  case StateBackend::Zmq:
    return size_t(-1);
  case StateBackend::None:
    return 0;
  }
  return 0;
}

inline bool fits(StateBackend b, size_t stateBytes) {
  return stateBytes <= maxStateBytes(b);
}

/// Fail loudly before first send. Returns false if backend cannot carry T.
template <class T> bool checkStateSize(StateBackend b) {
  const size_t n = sizeof(T);
  if (fits(b, n)) {
    return true;
  }
  std::cerr << "[StateSync] sizeof(T)=" << n << " exceeds "
            << backendName(b) << " limit " << maxStateBytes(b)
            << ". Pick another StateBackend (cuttlebone/zmq for Scene).\n";
  return false;
}

} // namespace StateSyncLimits

/**
 * Factory that attaches send/recv Stages onto an existing StateDistributionDomain.
 * Registered by backends (OscBlob in alapp; Cuttlebone/Zmq in al_ext).
 */
using SceneStateBackendFactory = std::function<bool(
    StateDistributionDomain<SceneStateBlob> &domain, StateSyncConfig const &cfg,
    bool isSender)>;

/// Register or replace a backend factory. Safe to call from static init.
void registerSceneStateBackend(StateBackend backend,
                               SceneStateBackendFactory factory);

/// Attach transport Stages. Returns false if backend missing or size check fails.
bool attachSceneStateSync(StateDistributionDomain<SceneStateBlob> &domain,
                          StateSyncConfig const &cfg, bool isSender);

/// True if a factory is registered for \p backend.
bool hasSceneStateBackend(StateBackend backend);

} // namespace al

#endif // INCLUDE_AL_STATE_SYNC_HPP
