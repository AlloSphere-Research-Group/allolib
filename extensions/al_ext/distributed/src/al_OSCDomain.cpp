#include "al_OSCDomain.hpp"

using namespace al;

bool OSCDomain::initialize(ComputationDomain *parent) {
  (void) parent;
  mHandler.mOscDomain = this;
  mParameterServer.registerOSCListener(&mHandler); // Have the parameter server pass unhandled messages to this app's onMessage virtual function
  return true;
}

bool OSCDomain::start() {
  bool ret = true;
  ret &= mParameterServer.listen();
  return ret;
}

bool OSCDomain::stop() {
  bool ret = true;
  mParameterServer.stopServer();
  return ret;
}

bool OSCDomain::cleanup(ComputationDomain *parent)
{
  (void) parent;
  return true;
}

bool OSCDomain::configure(uint16_t port, std::string address) {
  mParameterServer.configure(port, address);
  return true;
}
