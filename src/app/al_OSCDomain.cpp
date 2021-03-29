#include "al/app/al_OSCDomain.hpp"

using namespace al;

bool OSCDomain::init(ComputationDomain *parent) {
  (void)parent;
  mHandler.mOscDomain = this;
  mParameterServer.registerOSCListener(
      &mHandler); // Have the parameter server pass unhandled messages to this
                  // app's onMessage virtual function
  return true;
}

bool OSCDomain::start() {
  if (parameterServer().listen(port, interfaceIP)) {
    parameterServer().startHandshakeServer();
    return true;
  } else {
    uint16_t primaryPort = port;
    uint16_t portOffset = UINT16_MAX;
    int maxInstances = 100;
    while (!parameterServer().serverRunning() && ++portOffset < maxInstances) {
      parameterServer().listen(primaryPort + portOffset, interfaceIP);
    }
    if (parameterServer().serverRunning()) {
      port = portOffset + port;
      parameterServer().startCommandListener(interfaceIP);
      std::cout << "Application is replica on port: "
                << parameterServer().serverPort() << std::endl;
    } else {
      port = 0;
      std::cerr << "Warning: Application could not start network role."
                << std::endl;
    }
  }
  return parameterServer().serverRunning();
}

bool OSCDomain::stop() {
  bool ret = true;
  mParameterServer.stopServer();
  return ret;
}

bool OSCDomain::cleanup(ComputationDomain *parent) {
  (void)parent;
  mParameterServer.clearOSCListeners();
  return true;
}

bool OSCDomain::configure(uint16_t port_, std::string address) {
  port = port_;
  interfaceIP = address;
  return true;
}
