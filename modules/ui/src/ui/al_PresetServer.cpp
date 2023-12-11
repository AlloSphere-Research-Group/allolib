
#include "al/ui/al_PresetServer.hpp"

using namespace al;

// PresetServer ----------------------------------------------------------------

PresetServer::PresetServer(std::string oscAddress, int oscPort)
    : mServer(nullptr), mOSCpath("/preset"), mAllowStore(true),
      mStoreMode(false), mNotifyPresetChange(true), mParameterServer(nullptr) {
  try {
    mServer = new osc::Recv(oscPort, oscAddress.c_str(),
                            0.001); // Is this 1ms wait OK?
  } catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
  }

  if (mServer) {
    mServer->handler(*this);
    mServer->start();
  } else {
    std::cout << "Error starting OSC server." << std::endl;
  }
}

PresetServer::PresetServer(ParameterServer &paramServer)
    : mServer(nullptr), mOSCpath("/preset"), mAllowStore(true),
      mStoreMode(false), mNotifyPresetChange(true) {
  paramServer.registerOSCListener(this);
  mParameterServer = &paramServer;
}

PresetServer::~PresetServer() {
  //	std::cout << "~PresetServer()" << std::endl;;
  if (mServer) {
    mServer->stop();
    delete mServer;
    mServer = nullptr;
  }
}

void PresetServer::onMessage(osc::Message &m) {
  m.resetStream(); // Should be moved to the caller...
  //	std::cout << "PresetServer::onMessage " << std::endl;
  mPresetChangeLock.lock();
  mPresetChangeSenderAddr = m.senderAddress();
  if (m.addressPattern() == mOSCpath && m.typeTags() == "f") {
    float val;
    m >> val;
    if (!this->mStoreMode) {
      for (PresetHandler *handler : mPresetHandlers) {
        handler->recallPreset(static_cast<int>(val));
      }
    } else {
      for (PresetHandler *handler : mPresetHandlers) {
        handler->storePreset(static_cast<int>(val));
      }
      this->mStoreMode = false;
    }
  } else if (m.addressPattern() == mOSCpath && m.typeTags() == "s") {
    std::string val;
    m >> val;
    if (!this->mStoreMode) {
      for (PresetHandler *handler : mPresetHandlers) {
        handler->recallPreset(val);
      }
    } else {
      for (PresetHandler *handler : mPresetHandlers) {
        handler->storePreset(val);
      }
      this->mStoreMode = false;
    }
  } else if (m.addressPattern() == mOSCpath && m.typeTags() == "i") {
    int val;
    m >> val;
    if (!this->mStoreMode) {
      for (PresetHandler *handler : mPresetHandlers) {
        handler->recallPreset(val);
      }
    } else {
      for (PresetHandler *handler : mPresetHandlers) {
        handler->storePreset(val);
      }
      this->mStoreMode = false;
    }
  } else if (m.addressPattern() == mOSCpath + "/morphTime" &&
             m.typeTags() == "f") {
    float val;
    m >> val;
    for (PresetHandler *handler : mPresetHandlers) {
      handler->setMorphTime(val);
    }
  } else if (m.addressPattern() == mOSCpath + "/store" && m.typeTags() == "f") {
    float val;
    m >> val;
    if (this->mAllowStore) {
      for (PresetHandler *handler : mPresetHandlers) {
        handler->storePreset(static_cast<int>(val));
      }
    }
  } else if (m.addressPattern() == mOSCpath + "/storeMode" &&
             m.typeTags() == "f") {
    float val;
    m >> val;
    if (this->mAllowStore) {
      this->mStoreMode = (val != 0.0f);
    } else {
      std::cout << "Remote storing disabled" << std::endl;
    }
  } else if (m.addressPattern() == mOSCpath + "/queryState") {
    this->mParameterServer->notifyAll();
  } else if (m.addressPattern().substr(0, mOSCpath.size() + 1) ==
             mOSCpath + "/") {
    int index = std::stoi(m.addressPattern().substr(mOSCpath.size() + 1));
    if (m.typeTags() == "f") {
      float val;
      m >> val;
      if (static_cast<int>(val) == 1) {
        if (!this->mStoreMode) {
          for (PresetHandler *handler : mPresetHandlers) {
            handler->recallPreset(index);
          }
        } else {
          for (PresetHandler *handler : mPresetHandlers) {
            handler->storePreset(index);
          }
          this->mStoreMode = false;
        }
      }
    }
  }
  mPresetChangeLock.unlock();
  mHandlerLock.lock();
  for (osc::PacketHandler *handler : mHandlers) {
    m.resetStream();
    handler->onMessage(m);
  }
  mHandlerLock.unlock();
}

void PresetServer::print() {
  if (mServer) {
    std::cout << "Preset server listening on: " << mServer->address() << ":"
              << mServer->port() << std::endl;
    std::cout << "Communicating on path: " << mOSCpath << std::endl;

  } else {
    std::cout << "Preset Server Connected to shared server." << std::endl;
  }

  if (mOSCSenders.size() > 0) {
    std::cout << "Registered listeners: " << std::endl;
    for (auto sender : mOSCSenders) {
      std::cout << sender->address() << ":" << sender->port() << std::endl;
    }
  }
}

void PresetServer::stopServer() {
  if (mServer) {
    mServer->stop();
    delete mServer;
    mServer = nullptr;
  }
}

bool PresetServer::serverRunning() {
  if (mParameterServer) {
    return mParameterServer->serverRunning();
  } else {
    return (mServer != nullptr);
  }
}

void PresetServer::setAddress(std::string address) { mOSCpath = address; }

std::string PresetServer::getAddress() { return mOSCpath; }

void PresetServer::attachPacketHandler(osc::PacketHandler *handler) {
  mHandlerLock.lock();
  mHandlers.push_back(handler);
  mHandlerLock.unlock();
}

void PresetServer::changeCallback(int value, void *sender, void *userData) {
  (void)sender; // remove compiler warnings
  PresetServer *server = static_cast<PresetServer *>(userData);
  //	Parameter *parameter = static_cast<Parameter *>(sender);

  if (server->mNotifyPresetChange) {
    server->notifyListeners(server->mOSCpath, value);
  }
}
