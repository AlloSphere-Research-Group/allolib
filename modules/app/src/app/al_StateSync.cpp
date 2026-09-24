#include "al/app/al_StateSync.hpp"

#include <mutex>

namespace al {
namespace {

std::map<StateBackend, SceneStateBackendFactory> &factories() {
  static std::map<StateBackend, SceneStateBackendFactory> g;
  return g;
}

std::mutex &factoriesMutex() {
  static std::mutex m;
  return m;
}

/// OscBlob Stages — only for tiny POD. Hard-fails size check above.
bool attachOscBlob(StateDistributionDomain<SceneStateBlob> &domain,
                   StateSyncConfig const &cfg, bool isSender) {
  if (!StateSyncLimits::checkStateSize<SceneStateBlob>(StateBackend::OscBlob)) {
    return false;
  }
  if (isSender) {
    auto sender = domain.addStateSender(cfg.id, domain.statePtr());
    sender->configure(cfg.port, cfg.id, cfg.address, cfg.packetBytes);
    std::cout << "[StateSync] OscBlob SEND → " << cfg.address << ":" << cfg.port
              << " (sizeof=" << sizeof(SceneStateBlob) << ")\n";
  } else {
    auto receiver = domain.addStateReceiver(cfg.id, domain.statePtr());
    receiver->configure(cfg.port, cfg.id, "0.0.0.0", cfg.packetBytes);
    std::cout << "[StateSync] OscBlob RECV :" << cfg.port << "\n";
  }
  return true;
}

struct OscBlobAutoRegister {
  OscBlobAutoRegister() {
    registerSceneStateBackend(StateBackend::OscBlob, attachOscBlob);
  }
};

OscBlobAutoRegister gOscBlobRegister;

} // namespace

void registerSceneStateBackend(StateBackend backend,
                               SceneStateBackendFactory factory) {
  std::lock_guard<std::mutex> lock(factoriesMutex());
  factories()[backend] = std::move(factory);
}

bool hasSceneStateBackend(StateBackend backend) {
  std::lock_guard<std::mutex> lock(factoriesMutex());
  return factories().count(backend) > 0 &&
         static_cast<bool>(factories().at(backend));
}

bool attachSceneStateSync(StateDistributionDomain<SceneStateBlob> &domain,
                          StateSyncConfig const &cfg, bool isSender) {
  if (cfg.backend == StateBackend::None) {
    return true;
  }
  if (!StateSyncLimits::checkStateSize<SceneStateBlob>(cfg.backend)) {
    return false;
  }

  SceneStateBackendFactory factory;
  {
    std::lock_guard<std::mutex> lock(factoriesMutex());
    auto it = factories().find(cfg.backend);
    if (it == factories().end() || !it->second) {
      std::cerr << "[StateSync] no factory registered for backend '"
                << StateSyncLimits::backendName(cfg.backend)
                << "'. Link al_statedistribution (cuttlebone) or register one.\n";
      return false;
    }
    factory = it->second;
  }
  return factory(domain, cfg, isSender);
}

} // namespace al
