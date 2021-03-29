#ifndef CONSOLEDOMAIN_H
#define CONSOLEDOMAIN_H

#include <chrono>
#include <iostream>
#include <string>

#include "al_ComputationDomain.hpp"

namespace al {

/**
 * @brief ConsoleDomain class
 * @ingroup App
 *
 * This domain is a blocking domain, i.e. blocks on start(). Set the onLine()
 * function to get lines entered in the console. My default, the domain exits
 * when the line is empty (when return is pressed). You can use this domain
 * instead of app to build your own command line classes.
 */
class ConsoleDomain : public AsynchronousDomain {
public:
  bool init(ComputationDomain *parent = nullptr) override { return true; }

  bool start() override {
    std::string line;
    do {
      std::getline(std::cin, line);
    } while (onLine(line));
    return true;
  }

  bool stop() override { return true; }

  bool cleanup(ComputationDomain *parent = nullptr) override { return true; }

  std::function<bool(const std::string &)> onLine =
      [](const std::string &line) {
        if (line.size() == 0) {
          return false;
        }
        return true;
      };

private:
};

} // namespace al

#endif // CONSOLEDOMAIN_H
