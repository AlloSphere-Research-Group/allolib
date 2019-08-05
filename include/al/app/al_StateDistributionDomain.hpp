#ifndef STATEDISTRIBUTIONDOMAIN_H
#define STATEDISTRIBUTIONDOMAIN_H

#include <stack>
#include <vector>
#include <memory>
#include <iostream>
#include <functional>
#include <mutex>
#include <cstring>
#include <cassert>

#include "al/spatial/al_Pose.hpp"
#include "al/protocol/al_OSC.hpp"

#include "al/app/al_ComputationDomain.hpp"
#include "Gamma/Domain.h"

//#undef AL_USE_CUTTLEBONE

#ifdef AL_USE_CUTTLEBONE
#include "Cuttlebone/Cuttlebone.hpp"
#else
#include "al/protocol/al_OSC.hpp"
#endif

namespace al {

struct DefaultState {
    Pose pose;
};
template<class TSharedState>
class StateReceiveDomain;

template<class TSharedState>
class StateSendDomain;

class StateDistributionDomain : public SynchronousDomain {
public:

  template<class TSharedState = DefaultState>
  std::shared_ptr<StateSendDomain<TSharedState>> addStateSender(std::string id = "", std::shared_ptr<TSharedState> statePtr = nullptr);

  template<class TSharedState = DefaultState>
  std::shared_ptr<StateReceiveDomain<TSharedState>> addStateReceiver(std::string id = "", std::shared_ptr<TSharedState> statePtr = nullptr);

private:
};

template<class TSharedState = DefaultState>
class StateReceiveDomain : public SynchronousDomain {
public:

  bool initialize(ComputationDomain *parent = nullptr) override;

  bool tick() override {
    tickSubdomains(true);

    assert(mState); // State must have been set at this point
#ifdef AL_USE_CUTTLEBONE
    assert(mTaker);
    mQueuedStates = mTaker->get(mState);
    return true;
#else
    mRecvLock.lock();
    if (newMessages > 0) {
      mQueuedStates = newMessages;
      std::memcpy(mState.get(), buf.get(), sizeof(TSharedState));
      newMessages = 0;
    }
    mRecvLock.unlock();
    tickSubdomains(false);
    return true;
#endif
  }

  bool cleanup(ComputationDomain *parent = nullptr) override {
    cleanupSubdomains(true);
#ifdef AL_USE_CUTTLEBONE
    mTaker->stop();
    mTaker = nullptr;
    cleanupSubdomains(false);
    return true;
#else
    mRecv = nullptr;
    mState = nullptr;

//    std::cerr << "Not using Cuttlebone. Ignoring" << std::endl;
    cleanupSubdomains(false);
    return true;
#endif
  }

  void configure(uint16_t port=10100, std::string id = "state", std::string address = "localhost",
                 uint16_t packetSize = 1400) {
    mPort = port;
    mId = id;
    mAddress = address;
    mPacketSize = packetSize;
  }

  std::shared_ptr<TSharedState> state() { return mState;}

  void setStatePointer(std::shared_ptr<TSharedState> ptr) {mState = ptr;}

  void lockState() { mRecvLock.lock(); }
  void unlockState() {mRecvLock.unlock();}
  int newStates() { return mQueuedStates; }

  std::string id() const
  {
    return mId;
  }

  void setId(const std::string &id)
  {
    mId = id;
  }

private:
  std::shared_ptr<TSharedState> mState;
  int mQueuedStates {1};

  std::string mAddress {"localhost"};
  uint16_t mPort = 10100;
  uint16_t mPacketSize = 1400;
  std::string mId;

#ifdef AL_USE_CUTTLEBONE
  std::unique_ptr<cuttlebone::Taker<TSharedState>> mTaker;
#else
  class Handler: public osc::PacketHandler {
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
          } else {
            std::cerr << "ERROR: received state size mismatch" << std::endl;
          }
        }
      }
    }
  } mHandler;

  std::unique_ptr<unsigned char []> buf;

  uint16_t newMessages = 0;
  std::mutex mRecvLock;
  std::unique_ptr<osc::Recv> mRecv;
#endif

};

template<class TSharedState>
bool StateReceiveDomain<TSharedState>::initialize(ComputationDomain *parent) {
  initializeSubdomains(true);
  assert(parent != nullptr);

#ifdef AL_USE_CUTTLEBONE
  mTaker = std::make_unique<cuttlebone::Taker<TSharedState, mPacketSize, mPort>>();
  mTaker->start();
  initializeSubdomains(false);
  return true;
#else
  buf = std::make_unique<unsigned char []>(sizeof(TSharedState));
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

  std::cout << "Opened " << mAddress << ":" << mPort << std::endl;
  initializeSubdomains(false);
  return true;
#endif
}

template<class TSharedState = DefaultState>
class StateSendDomain : public SynchronousDomain {
public:

  bool initialize(ComputationDomain *parent = nullptr) override {
    initializeSubdomains(true);

#ifdef AL_USE_CUTTLEBONE
    mMaker = std::make_unique<cuttlebone::Taker<TSharedState, mPacketSize, mPort>>();
    mMaker->start();
    initializeSubdomains(false);
    return true;
#else
    //    mSend = std::make_unique<osc::Send>(mPort, mAddress.c_str());
    ////    std::cout << "StateSendDomain not using Cuttlebone" << std::endl;
    //    if (!mSend) {
    //      std::cerr << "Can't create sender for StateSendDomain address:" << mAddress << " port:" << mPort <<std::endl;
    //    }
    initializeSubdomains(false);
    return true;
#endif
  }

  bool tick() override {
    tickSubdomains(true);

    assert(mState); // State must have been set at this point
#ifdef AL_USE_CUTTLEBONE
    assert(mMaker);
    mMaker->set(mState);
    tickSubdomains(false);
    return true;
#else
//    assert(mSend);

//    osc::Blob b(&mState, sizeof(mState));
//    mSend->send("/_state", b);

    mStateLock.lock();
    osc::Blob b(mState.get(), sizeof(TSharedState));
    osc::Send s(mPort, mAddress.c_str());
//    std::cout << mAddress << ":" << mPort << std::endl;
    s.send("/_state", mId, b);

    mStateLock.unlock();

    tickSubdomains(false);
    return true;
#endif
  }

  bool cleanup(ComputationDomain *parent = nullptr) override {
    cleanupSubdomains(true);
#ifdef AL_USE_CUTTLEBONE
      if (mMaker) {
          mMaker->stop();
      }
      cleanupSubdomains(false);
      return true;
#else
    mState = nullptr;
//    std::cerr << "Not using Cuttlebone. Ignoring" << std::endl;
    cleanupSubdomains(false);
    return true;
#endif
  }

  void configure(uint16_t port, std::string id = "state", std::string address = "localhost", uint16_t packetSize = 1400) {
    mPort = port;
    mId = id;
    mAddress = address;
    mPacketSize = packetSize;
  }

  std::shared_ptr<TSharedState> state() { return mState;}

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

private:

#ifdef AL_USE_CUTTLEBONE
  std::unique_ptr<cuttlebone::Maker<TSharedState>> mMaker;
#else
  std::unique_ptr<osc::Send> mSend;
#endif

#ifdef AL_USE_CUTTLEBONE
  if (role() & ROLE_SIMULATOR) {
      std::string broadcastAddress = configLoader.gets("broadcastAddress");
      mMaker = std::make_unique<cuttlebone::Maker<TSharedState>>(broadcastAddress.c_str());
      mMaker->start();
  } else if (role() & ROLE_RENDERER){
  }

#endif

  std::string mId = "";
  uint16_t mPort = 10100;
  std::string mAddress {"localhost"};
  uint16_t mPacketSize = 1400;

  std::shared_ptr<TSharedState> mState;
  std::mutex mStateLock;
  int mQueuedStates {0};
};


template<class TSharedState>
std::shared_ptr<StateSendDomain<TSharedState>> StateDistributionDomain::addStateSender(std::string id, std::shared_ptr<TSharedState> statePtr) {
  auto newDomain = newSubDomain<StateSendDomain<TSharedState>>(false);
  newDomain->setId(id);
  return newDomain;
}


template<class TSharedState>
std::shared_ptr<StateReceiveDomain<TSharedState>>  StateDistributionDomain::addStateReceiver(std::string id , std::shared_ptr<TSharedState> statePtr) {
  auto newDomain = newSubDomain<StateReceiveDomain<TSharedState>>(true);
  newDomain->setId(id);

  return newDomain;
}


}

#endif // STATEDISTRIBUTIONDOMAIN_H
