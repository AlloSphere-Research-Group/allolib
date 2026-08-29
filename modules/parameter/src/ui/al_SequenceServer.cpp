#include "al/ui/al_SequenceServer.hpp"

#include <iostream>

using namespace al;

// SequenceServer
// ----------------------------------------------------------------

SequenceServer::SequenceServer(std::string oscAddress, int oscPort)
    : mServer(nullptr),
      mRecorder(nullptr),
      // mParamServer(nullptr),
      mOSCpath("/sequence") {
  mServer = new osc::Recv(oscPort, oscAddress.c_str(),
                          0.001);  // Is this 1ms wait OK?
  if (mServer) {
    mServer->handler(*this);
    mServer->start();
  } else {
    std::cout << "Error starting OSC server." << std::endl;
  }
}

SequenceServer::SequenceServer(ParameterServer &paramServer)
    : mServer(nullptr),
      // mParamServer(&paramServer),
      mOSCpath("/sequence") {
  paramServer.registerOSCListener(this);
}

SequenceServer::~SequenceServer() {
  //	std::cout << "~SequenceServer()" << std::endl;;
  if (mServer) {
    mServer->stop();
    delete mServer;
    mServer = nullptr;
  }
}

void SequenceServer::onMessage(osc::Message &m) {
  if (m.addressPattern() == mOSCpath + "/last") {
    if (mSequencer && mRecorder) {
      std::cout << "start last recorder sequence "
                << mRecorder->lastSequenceName() << std::endl;
      mSequencer->setHandlerSubDirectory(mRecorder->lastSequenceSubDir());
      mSequencer->playSequence(mRecorder->lastSequenceName());
    } else {
      std::cerr << "SequenceRecorder and PresetSequencer must be registered to "
                   "enable /*/last."
                << std::endl;
    }
  } else {
    for (osc::MessageConsumer *consumer : mConsumers) {
      if (consumer->consumeMessage(m, mOSCpath)) {
        break;
      }
    }
  }
}

SequenceServer &SequenceServer::registerMessageConsumer(
    osc::MessageConsumer &consumer) {
  mConsumers.push_back(&consumer);
  return *this;
}

SequenceServer &SequenceServer::registerRecorder(SequenceRecorder &recorder) {
  mRecorder = &recorder;
  mConsumers.push_back(static_cast<osc::MessageConsumer *>(&recorder));
  return *this;
}

SequenceServer &SequenceServer::registerSequencer(PresetSequencer &sequencer) {
  mSequencer = &sequencer;
  mConsumers.push_back(&sequencer);
  return *this;
}

void SequenceServer::print() {
  if (mServer) {
    std::cout << "Sequence server listening on: " << mServer->address() << ":"
              << mServer->port() << std::endl;
    std::cout << "Communicating on path: " << mOSCpath << std::endl;
  }
  for (auto sender : mOSCSenders) {
    std::cout << sender->address() << ":" << sender->port() << std::endl;
  }
}

void SequenceServer::stopServer() {
  if (mServer) {
    mServer->stop();
    delete mServer;
    mServer = nullptr;
  }
}

void SequenceServer::setAddress(std::string address) { mOSCpath = address; }

std::string SequenceServer::getAddress() { return mOSCpath; }

void SequenceServer::changeCallback(int value, void *sender, void *userData) {
  SequenceServer *server = static_cast<SequenceServer *>(userData);
  // Parameter *parameter = static_cast<Parameter *>(sender);
  server->notifyListeners(server->mOSCpath, value);
}
