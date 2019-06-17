#ifndef OSCDOMAIN_H
#define OSCDOMAIN_H

#include <iostream>
#include <functional>

#include "al_ComputationDomain.hpp"

#include "al/core/protocol/al_OSC.hpp"
#include "al/util/ui/al_ParameterServer.hpp"
//#include "al/util/al_FlowAppParameters.hpp"

namespace al {

class OSCDomain: public AsynchronousDomain {
public:

  // Domain management functions
  bool initialize(ComputationDomain *parent = nullptr) override;
  bool start() override;
  bool stop() override;
  bool cleanup(ComputationDomain *parent = nullptr) override;

  // Configuration and query
  bool configure(uint16_t port, std::string address = "");

  bool running() { return mParameterServer.serverRunning();}

  ParameterServer& parameterServer() { return mParameterServer; }
  ParameterServer const& parameterServer() const { return mParameterServer; }

  // PacketHandler
  std::function<void(osc::Message&)> onMessage = [](osc::Message &m){std::cout << "Received unhandled message." <<std::endl; m.print();};
private:

  class Handler: public osc::PacketHandler {
  public:
    OSCDomain *mOscDomain;
    void onMessage(osc::Message &m) {
      this->onMessage(m);
    }
  } mHandler;
    ParameterServer mParameterServer {"0.0.0.0", 9010, false};
};

} // namespace al

#endif // OSCDOMAIN_H
