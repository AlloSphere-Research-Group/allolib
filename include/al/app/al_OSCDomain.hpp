#ifndef OSCDOMAIN_H
#define OSCDOMAIN_H

#include <functional>
#include <iostream>

#include "al_ComputationDomain.hpp"

#include "al/protocol/al_OSC.hpp"
#include "al/ui/al_ParameterServer.hpp"

namespace al {

/**
 * @brief OSCDomain class
 * @ingroup App
 */
class OSCDomain : public AsynchronousDomain {
public:
  virtual ~OSCDomain() {}
  // Domain management functions
  bool init(ComputationDomain *parent = nullptr) override;
  bool start() override;
  bool stop() override;
  bool cleanup(ComputationDomain *parent = nullptr) override;

  // Configuration and query
  bool configure(uint16_t port, std::string address = "");

  bool running() { return mParameterServer.serverRunning(); }

  osc::PacketHandler &handler() { return mHandler; }

  ParameterServer &parameterServer() { return mParameterServer; }
  ParameterServer const &parameterServer() const { return mParameterServer; }

  // PacketHandler
  std::function<void(osc::Message &)> onMessage = [](osc::Message &m) {
    std::cout << "Received unhandled message." << std::endl;
    m.print();
  };

  std::string interfaceIP = "0.0.0.0";
  uint16_t port = 9010;

private:
  class Handler : public osc::PacketHandler {
  public:
    OSCDomain *mOscDomain;
    void onMessage(osc::Message &m) { mOscDomain->onMessage(m); }
  } mHandler;
  ParameterServer mParameterServer{"0.0.0.0", 9010, false};
};

} // namespace al

#endif // OSCDOMAIN_H
