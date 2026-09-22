#ifndef INCLUDE_AL_KEEPALIVEDOMAIN_HPP
#define INCLUDE_AL_KEEPALIVEDOMAIN_HPP

#include <atomic>
#include <chrono>
#include <thread>

#include "al/app/al_ComputationDomain.hpp"

namespace al {

/**
 * @brief Blocks Runtime::run() until quit() — legacy only.
 * @ingroup App
 *
 * Prefer Runtime idle (no Main modules) or LoopMode::Runtime graphics.
 * AppBuilder no longer uses KeepAliveDomain. Kept for older recipes that
 * still rely on a blocking start() peer beside Callback modules.
 *
 * @deprecated Use Runtime cooperative pump / idle instead.
 */
class KeepAliveDomain : public AsynchronousDomain {
public:
  bool init(ComputationDomain *parent = nullptr) override {
    (void)parent;
    return true;
  }

  bool start() override {
    while (!mQuit.load()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return true;
  }

  bool stop() override {
    mQuit = true;
    return true;
  }

  bool cleanup(ComputationDomain *parent = nullptr) override {
    (void)parent;
    return true;
  }

  void quit() override { mQuit = true; }
  bool shouldQuit() const override { return mQuit.load(); }

  /// Forces legacy Runtime path (blocks inside start()).
  bool blocksInStart() const override { return true; }

private:
  std::atomic<bool> mQuit{false};
};

} // namespace al

#endif // INCLUDE_AL_KEEPALIVEDOMAIN_HPP
