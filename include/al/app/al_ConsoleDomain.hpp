#ifndef CONSOLEDOMAIN_H
#define CONSOLEDOMAIN_H

#include <iostream>
#include <chrono>
#include <string>

#include "al_ComputationDomain.hpp"

namespace al {

class ConsoleDomain: public AsynchronousDomain {
public:
  bool initialize(ComputationDomain *parent = nullptr) override {
    return true;
  }

  bool start() override {
    std::string line;
    do {
      std::getline(std::cin, line);
    } while(onLine(line));
    return true;
  }

  bool stop() override {
    return true;
  }

  bool cleanup(ComputationDomain *parent = nullptr) override {
    return true;
  }

  std::function<bool(const std::string &)> onLine = [](const std::string &line) {
    if (line.size() == 0) { return false; }
    return true;
  };

private:
};


} // namespace al


#endif //CONSOLEDOMAIN_H
