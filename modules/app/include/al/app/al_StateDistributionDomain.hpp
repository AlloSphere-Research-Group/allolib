#ifndef STATEDISTRIBUTIONDOMAIN_H
#define STATEDISTRIBUTIONDOMAIN_H

#include <cassert>
#include <cstdint>
#include <cstring>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <stack>
#include <vector>

#include "al/app/al_SimulationDomain.hpp"
#include "al/protocol/al_OSC.hpp"
#include "al/spatial/al_Pose.hpp"

namespace al {

struct DefaultState {
  Pose pose;
};

template <class TSharedState> class StateReceiveDomain;

template <class TSharedState> class StateSendDomain;

template <class TSharedState> class StateSimulationDomain;

/**
 * @brief Simulation Stage that owns a POD state slot + optional sync Stages.
 * @ingroup App
 *
 * Prefer al_StateSync.hpp: attachSceneStateSync() with a StateBackend.
 * This type is the Stage that holds TSharedState and runs simulationFunction.
 * Send/recv are additional graphics Stages (not Runtime Modules).
 *
 * addStateSender / addStateReceiver attach the legacy OscBlob Stages.
 * OscBlob cannot carry SceneStateBlob (osc::Send default buffer is 1024).
 */
template <class TSharedState = DefaultState>
class StateDistributionDomain : public StateSimulationDomain<TSharedState> {
public:
  std::shared_ptr<StateSendDomain<TSharedState>>
  addStateSender(std::string id = "",
                 std::shared_ptr<TSharedState> statePtr = nullptr);

  std::shared_ptr<StateReceiveDomain<TSharedState>>
  addStateReceiver(std::string id = "",
                   std::shared_ptr<TSharedState> statePtr = nullptr);

  bool isSender() { return mIsSender; }

  void disconnect() {
    for (auto sendrecv : mSendRecvDomains) {
      this->removeSubDomain(sendrecv);
    }
  }

protected:
  bool mIsSender{false};

  std::vector<std::shared_ptr<SynchronousDomain>> mSendRecvDomains;

private:
};

template <class TSharedState = DefaultState>
class StateReceiveDomain : public SynchronousDomain {
public:
  bool init(ComputationDomain *parent = nullptr) override;

  bool tick() override {
    tickSubdomains(true);

    assert(mState); // State must have been set at this point
    mRecvLock.lock();
    if (newMessages > 0) {
      mQueuedStates = newMessages;
      std::memcpy(mState.get(), buf.get(), sizeof(TSharedState));
      newMessages = 0;
    }
    mRecvLock.unlock();
    tickSubdomains(false);
    return true;
  }

  bool cleanup(ComputationDomain *parent = nullptr) override {
    cleanupSubdomains(true);
    mRecv = nullptr;
    mState = nullptr;

    std::cout << "StateReceiveDomain: cleanup called." << std::endl;
    cleanupSubdomains(false);
    return true;
  }

  void configure(uint16_t port = 10100, std::string id = "state",
                 std::string address = "0.0.0.0", uint16_t packetSize = 1400) {
    mPort = port;
    mId = id;
    mAddress = address;
    mPacketSize = packetSize;
  }

  std::shared_ptr<TSharedState> state() { return mState; }

  void setStatePointer(std::shared_ptr<TSharedState> ptr) { mState = ptr; }

  void lockState() { mRecvLock.lock(); }
  void unlockState() { mRecvLock.unlock(); }
  int newStates() { return mQueuedStates; }

  std::string id() const { return mId; }

  void setId(const std::string &id) { mId = id; }

  void setAddress(std::string address) { mAddress = address; };

protected:
  std::shared_ptr<TSharedState> mState;
  int mQueuedStates{1};
  std::string mAddress{"localhost"};
  uint16_t mPort = 10100;
  uint16_t mPacketSize = 1400;

private:
  std::string mId;

  class Handler : public osc::PacketHandler {
  public:
    StateReceiveDomain *mOscDomain;
    void onMessage(osc::Message &m) override {
      //      m.print();
      if (m.addressPattern() == "/_state" && m.typeTags() == "sb") {
        std::string id;
        m >> id;
        if (id == mOscDomain->mId) {
          osc::Blob inBlob;
          m >> inBlob;
          if (sizeof(TSharedState) == inBlob.size) {
            mOscDomain->mRecvLock.lock();
            memcpy(mOscDomain->buf.get(), inBlob.data, sizeof(TSharedState));
            mOscDomain->newMessages++;
            mOscDomain->mRecvLock.unlock();
            // std::cerr << "OSC State Received: " << inBlob.size << std::endl;

          } else {
            std::cerr << "ERROR: received state size mismatch" << std::endl;
          }
        }
      }
    }
  } mHandler;

  std::unique_ptr<unsigned char[]> buf;

  uint16_t newMessages = 0;
  std::mutex mRecvLock;
  std::unique_ptr<osc::Recv> mRecv;
};

template <class TSharedState>
bool StateReceiveDomain<TSharedState>::init(ComputationDomain *parent) {
  initializeSubdomains(true);
  assert(parent != nullptr);

  buf = std::make_unique<unsigned char[]>(sizeof(TSharedState));
  mRecv = std::make_unique<osc::Recv>();
  if (!mRecv || !mRecv->open(mPort, mAddress.c_str())) {
    std::cerr << "Error opening server" << std::endl;
    return false;
  }
  mHandler.mOscDomain = this;
  mRecv->handler(mHandler);
  if (!mRecv->start()) {
    std::cerr << "Failed to start receiver. " << std::endl;
    return false;
  }

  std::cout << "StateReceiveDomain: init called using " << mAddress
            << ":" << mPort << std::endl;

  initializeSubdomains(false);
  return true;
}

template <class TSharedState = DefaultState>
class StateSendDomain : public SynchronousDomain {
public:
  bool init(ComputationDomain *parent = nullptr) override {
    initializeSubdomains(true);

    initializeSubdomains(false);

    std::cout << "StateSendDomain: init called using " << mAddress
              << ":" << mPort << std::endl;
    return true;
  }

  bool tick() override {
    tickSubdomains(true);

    assert(mState);

    // osc::Send default buffer is 1024 — refuse oversized POD loudly.
    constexpr size_t kOscDefaultBuf = 1024;
    constexpr size_t kOscFramingSlop = 256;
    if (sizeof(TSharedState) + kOscFramingSlop > kOscDefaultBuf) {
      std::cerr
          << "[StateSendDomain/OscBlob] sizeof(T)=" << sizeof(TSharedState)
          << " exceeds osc::Send buffer (" << kOscDefaultBuf
          << "). Use StateBackend::Cuttlebone (see al_StateSync.hpp).\n";
      tickSubdomains(false);
      return false;
    }

    mStateLock.lock();
    osc::Blob b(mState.get(), sizeof(TSharedState));
    osc::Send s(mPort, mAddress.c_str());
    s.send("/_state", mId, b);
    mStateLock.unlock();

    tickSubdomains(false);
    return true;
  }

  bool cleanup(ComputationDomain *parent = nullptr) override {
    cleanupSubdomains(true);
    mState = nullptr;
    std::cout << "StateSendDomain: cleanup called." << std::endl;
    cleanupSubdomains(false);
    return true;
  }

  void configure(uint16_t port, std::string id = "state",
                 std::string address = "localhost",
                 uint16_t packetSize = 1400) {
    mPort = port;
    mId = id;
    mAddress = address;
    mPacketSize = packetSize;
  }

  std::shared_ptr<TSharedState> state() { return mState; }

  //  void lockState() { mStateLock.lock(); }
  //  void unlockState() {mStateLock.unlock();}

  void setStatePointer(std::shared_ptr<TSharedState> ptr) {
    mStateLock.lock();
    mState = ptr;
    mStateLock.unlock();
  }

  int newStates() { return mQueuedStates; }

  std::string id() const { return mId; }

  void setId(const std::string &id) { mId = id; }

  void setAddress(std::string address) { mAddress = address; };

protected:
  std::shared_ptr<TSharedState> mState;
  std::mutex mStateLock;
  int mQueuedStates{0};
  uint16_t mPort = 10100;
  std::string mAddress{"localhost"};
  uint16_t mPacketSize = 1400;

private:
  std::string mId = "";
};

template <class TSharedState>
std::shared_ptr<StateSendDomain<TSharedState>>
StateDistributionDomain<TSharedState>::addStateSender(
    std::string id, std::shared_ptr<TSharedState> statePtr) {
  auto newDomain =
      this->template newSubDomain<StateSendDomain<TSharedState>>(false);
  newDomain->setId(id);
  newDomain->setStatePointer(statePtr);
  mSendRecvDomains.push_back(newDomain);
  return newDomain;
}

template <class TSharedState>
std::shared_ptr<StateReceiveDomain<TSharedState>>
StateDistributionDomain<TSharedState>::addStateReceiver(
    std::string id, std::shared_ptr<TSharedState> statePtr) {
  auto newDomain =
      this->template newSubDomain<StateReceiveDomain<TSharedState>>(true);
  newDomain->setId(id);
  newDomain->setStatePointer(statePtr);
  mSendRecvDomains.push_back(newDomain);
  return newDomain;
}

} // namespace al

#endif // STATEDISTRIBUTIONDOMAIN_H
