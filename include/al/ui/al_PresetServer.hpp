#ifndef AL_PRESETSERVER_H
#define AL_PRESETSERVER_H

/*	Allocore --
        Multimedia / virtual environment application class library

        Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology,
   UCSB. Copyright (C) 2016. The Regents of the University of California. All
   rights reserved.

        Redistribution and use in source and binary forms, with or without
        modification, are permitted provided that the following conditions are
   met:

                Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

                Redistributions in binary form must reproduce the above
   copyright notice, this list of conditions and the following disclaimer in the
                documentation and/or other materials provided with the
   distribution.

                Neither the name of the University of California nor the names
   of its contributors may be used to endorse or promote products derived from
                this software without specific prior written permission.

        THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
   IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
        IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
   PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
   OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
   OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
   ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

        File description:
        Preset classes that encapsulates storing values for groups of parameters
        File author(s):
        Andrés Cabrera mantaraya36@gmail.com
*/


#include "al/ui/al_PresetHandler.hpp"

namespace al {

///@ingroup UI
class PresetServer : public osc::PacketHandler, public OSCNotifier {
 public:
  /**
   * @brief PresetServer constructor
   *
   * @param oscAddress The network address on which to listen to. If empty use
   * all available network interfaces. Defaults to "127.0.0.1".
   * @param oscPort The network port on which to listen. Defaults to 9011.
   *
   */

  PresetServer(std::string oscAddress = "127.0.0.1", int oscPort = 9011);
  /**
   * @brief using this constructor reuses the existing osc::Recv server from the
   * ParameterServer object
   * @param paramServer an existing ParameterServer object
   *
   * You will want to reuse an osc::Recv server when you want to expose the
   * interface thorugh the same network port. Since network ports are exclusive,
   * once a port is bound, it can't be used. You might need to expose the
   * parameters on the same network port when using things like
   * interface.simpleserver.js That must connect all interfaces to the same
   * network port.
   */
  PresetServer(ParameterServer &paramServer);
  ~PresetServer();

  /**
   * @brief print prints information about the server to std::out
   */
  void print();

  /**
   * @brief stopServer stops the OSC server thread. Calling this function
   * is sometimes required when this object is destroyed abruptly and the
   * destructor is not called
   */
  void stopServer();

  bool serverRunning();

  void allowStore(bool allow) { mAllowStore = allow; }
  bool allowStore() { return mAllowStore; }

  virtual void onMessage(osc::Message &m);

  PresetServer &registerPresetHandler(PresetHandler &presetHandler) {
    mPresetHandlers.push_back(&presetHandler);
    presetHandler.registerPresetCallback(PresetServer::changeCallback,
                                         (void *)this);

    presetHandler.registerMorphTimeCallback([&](float value) {
      this->notifyListeners(this->mOSCpath + "/morphTime", value);
    });
    return *this;
  }

  PresetServer &operator<<(PresetHandler &presetHandler) {
    return registerPresetHandler(presetHandler);
  }

  void setAddress(std::string address);
  std::string getAddress();

  void notifyPresetChange(bool notify) { mNotifyPresetChange = notify; }

  void attachPacketHandler(osc::PacketHandler *handler);

 protected:
  static void changeCallback(int value, void *sender, void *userData);

 private:
  osc::Recv *mServer;
  std::vector<PresetHandler *> mPresetHandlers;
  //	std::mutex mServerLock;
  std::string mOSCpath;
  std::mutex mHandlerLock;
  std::vector<osc::PacketHandler *> mHandlers;
  bool mAllowStore;
  bool mStoreMode;
  bool mNotifyPresetChange;

  std::mutex mPresetChangeLock;
  std::string mPresetChangeSenderAddr;

  std::vector<std::string> mDisabledListeners;

  ParameterServer *mParameterServer;
};

} //namespace al

#endif // AL_PRESETSERVER_H