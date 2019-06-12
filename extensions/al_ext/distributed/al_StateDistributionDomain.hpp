#ifndef STATEDISTRIBUTIONDOMAIN_H
#define STATEDISTRIBUTIONDOMAIN_H

#include <stack>
#include <vector>
#include <memory>
#include <iostream>
#include <functional>
#include <cassert>
#include <mutex>

#include "al/core/spatial/al_Pose.hpp"

#include "al_ComputationDomain.hpp"
#include "Gamma/Domain.h"

//#undef AL_USE_CUTTLEBONE

#ifdef AL_USE_CUTTLEBONE
#include "Cuttlebone/Cuttlebone.hpp"
#else
#include "al/core/protocol/al_OSC.hpp"
#endif

namespace al {

struct DefaultState {
    Pose pose;
};

template<class TSharedState = DefaultState>
class StateReceiveDomain : public SynchronousDomain {
public:

  bool initialize(ComputationDomain *parent = nullptr) override {
    assert(parent != nullptr);

#ifdef AL_USE_CUTTLEBONE
    mTaker = std::make_unique<cuttlebone::Taker<TSharedState, mPacketSize, mPort>>();
    mTaker->start();
    return true;
#else
    buf = new char[sizeof(TSharedState)];
    mRecv = std::make_unique<osc::Recv>();
    if (!mRecv || !mRecv->open(mPort, mAddress.c_str())) {
      std::cerr << "Error opening server" << std::endl;
      return false;
    }
    mHandler.mOscDomain = this;
    mRecv->handler(mHandler);
    mRecv->start();

    std::cout << "Opened " << mAddress << ":" << mPort << std::endl;
    return true;
#endif
  }

  bool tick() override {
#ifdef AL_USE_CUTTLEBONE
    assert(mTaker);
    mQueuedStates = mTaker->get(mState);
    return true;
#else
    mRecvLock.lock();
    if (newMessages > 0) {
      if (sizeof(TSharedState) == mBlobBuf.size) {
        mQueuedStates = newMessages;
        memcpy(&mState, mBlobBuf.data, sizeof(TSharedState));
      } else {
        std::cerr << "received state size inconsistent. Ignoring " << newMessages << "states " << std::endl;
      }
      newMessages = 0;
    }
    mRecvLock.unlock();
    return false;
#endif
  }

  bool cleanup(ComputationDomain *parent = nullptr) override {
#ifdef AL_USE_CUTTLEBONE
    mTaker->stop();
    mTaker = nullptr;
    return true;
#else
    mRecv = nullptr;

    delete[] buf;
    std::cerr << "Not using Cuttlebone. Ignoring" << std::endl;
    return true;
#endif
  }

  void configure(uint16_t port=10100, std::string address = "0.0.0.0",
                 uint16_t packetSize = 1400) {
    mPort = port;
    mAddress = address;
    mPacketSize = packetSize;
  }

  TSharedState &state() { return mState;}

  void lockState() { mRecvLock.lock(); }
  void unlockState() {mRecvLock.unlock();}
  int newStates() { return mQueuedStates; }

private:
  TSharedState mState;
  int mQueuedStates {1};

  std::string mAddress {"0.0.0.0"};
  uint16_t mPort = 10100;
  uint16_t mPacketSize = 1400;

#ifdef AL_USE_CUTTLEBONE
  std::unique_ptr<cuttlebone::Taker<TSharedState>> mTaker;
#else
  class Handler: public osc::PacketHandler {
  public:
    StateReceiveDomain *mOscDomain;
    void onMessage(osc::Message &m) override {
//      m.print();
      if (m.addressPattern() == "/_state" && m.typeTags() == "b") {
        mOscDomain->mRecvLock.lock();
        m >> mOscDomain->mBlobBuf;
        mOscDomain->newMessages++;
        mOscDomain->mRecvLock.unlock();
      }
    }
  } mHandler;

  char *buf;
  osc::Blob mBlobBuf;
  uint16_t newMessages = 0;
  std::mutex mRecvLock;
  std::unique_ptr<osc::Recv> mRecv;
#endif

};

template<class TSharedState = DefaultState>
class StateSendDomain : public SynchronousDomain {
public:

  bool initialize(ComputationDomain *parent = nullptr) override {
#ifdef AL_USE_CUTTLEBONE
    mMaker = std::make_unique<cuttlebone::Taker<TSharedState, mPacketSize, mPort>>();
    mMaker->start();
    return true;
#else
//    mSend = std::make_unique<osc::Send>(mPort, mAddress.c_str());
////    std::cout << "StateSendDomain not using Cuttlebone" << std::endl;
//    if (!mSend) {
//      std::cerr << "Can't create sender for StateSendDomain address:" << mAddress << " port:" << mPort <<std::endl;
//    }
    return true;
#endif
  }

  bool tick() override {
#ifdef AL_USE_CUTTLEBONE
    assert(mMaker);
    mMaker->set(mState);
    return true;
#else
//    assert(mSend);

//    osc::Blob b(&mState, sizeof(mState));
//    mSend->send("/_state", b);


    osc::Blob b(&mState, sizeof(mState));
    osc::Send s(mPort, mAddress.c_str());
//    std::cout << mAddress << ":" << mPort << std::endl;
    s.send("/_state", b);

    return true;
#endif
  }

  bool cleanup(ComputationDomain *parent = nullptr) override {
#ifdef AL_USE_CUTTLEBONE
      if (mMaker) {
          mMaker->stop();
      }
      return true;
#else
    std::cerr << "Not using Cuttlebone. Ignoring" << std::endl;
    return true;
#endif
  }

  TSharedState &state() { return mState;}
  int newStates() { return mQueuedStates; }
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

  uint16_t mPort = 10100;
  std::string mAddress {"localhost"};
  uint16_t mPacketSize = 1400;

  TSharedState mState;
  int mQueuedStates {0};
};

}

#endif // STATEDISTRIBUTIONDOMAIN_H
