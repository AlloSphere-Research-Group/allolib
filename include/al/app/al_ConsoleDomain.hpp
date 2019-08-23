#ifndef CONSOLEDOMAIN_H
#define CONSOLEDOMAIN_H

#include <iostream>
#include <chrono>

#include "al_ComputationDomain.hpp"

namespace al {

class ConsoleDomain: public AsynchronousDomain {
public:
  bool initialize(ComputationDomain *parent = nullptr) override {
    mRunning = true;
    return true;
  }

  bool start() override {
    char inChar[128];
    do {
      std::cin.getline(inChar, 128);
    } while(onLine(inChar));
    return true;
  }

  bool stop() override {
    return true;
  }

  bool cleanup(ComputationDomain *parent = nullptr) override {
    mRunning = false;
    return true;
  }

  std::function<bool(std::string)> onLine = [](std::string line) {
    if (line.size() == 0) { return false;}
    return true;
  };

private:
    bool mRunning;
};


} // namespace al


#endif //CONSOLEDOMAIN_H
