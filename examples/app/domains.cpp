#include <iostream>
#include <memory>

#include "Gamma/Oscillator.h"
#include "al/app/al_AudioDomain.hpp"
#include "al/app/al_ConsoleDomain.hpp"
#include "al/app/al_OpenGLGraphicsDomain.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/math/al_Random.hpp"
#include "al/system/al_Time.hpp"

// This example shows how to use domains without the App class
// It's not really what you would normally use to create content, but it shows
// the possibilities opened up by the domain infrastructure when you need things
// the App class can't provide, like multimple windows, audio devices or
// interactive creation and destruction of domains.

using namespace al;

int main() {
  // Define domains
  AudioDomain audioDomain;
  bool audioDomainRunning = false;
  OpenGLGraphicsDomain graphicsDomain;
  ConsoleDomain consoleDomain;
  std::vector<std::shared_ptr<GLFWOpenGLWindowDomain>> windows;

  std::cout << "Type w to create new window" << std::endl
            << "Type e to destroy last window" << std::endl
            << "Type a to toggle audio" << std::endl
            << "Press enter to exit" << std::endl;

  // User control happens through the console domain
  consoleDomain.onLine = [&](std::string line) {
    if (line.size() == 0) {
      return false;
    } else if (line == "a") {
      if (audioDomainRunning) {
        audioDomain.stop();
        std::cout << "Audio stopped" << std::endl;
      } else {
        audioDomain.start();
        std::cout << "Audio started" << std::endl;
      }
      audioDomainRunning = !audioDomainRunning;
    } else if (line == "w") {
      auto window = graphicsDomain.newSubDomain<GLFWOpenGLWindowDomain>();
      Color bg(rnd::uniform(), rnd::uniform(), rnd::uniform());
      window->onDraw = [bg](Graphics &g) { g.clear(bg); };
      windows.push_back(window);
      std::cout << "New window" << std::endl;
    } else if (line == "e") {
      if (windows.size() > 0) {
        graphicsDomain.removeSubDomain(windows.back());
        windows.resize(windows.size() - 1);
      } else {
        std::cout << "No window to destroy" << std::endl;
      }
    } else {
      std::cout << "Unknown command" << line << std::endl;
    }
    return true;
  };

  // Audio produces white noise on first output channel
  audioDomain.onSound = [](AudioIOData &io) {
    while (io()) {
      io.out(0) = rnd::uniformS(0.1);
    }
  };

  // Initialize  domains
  consoleDomain.init();

  graphicsDomain.init();

  audioDomain.audioIO().device(AudioDevice(-1));
  audioDomain.audioIO().framesPerSecond(48000);
  audioDomain.audioIO().channelsOut(2);
  audioDomain.init();
  audioDomain.audioIO().print(); // Show audio details
  // The audio domain is stopped and started within the console domain

  // The graphics domain must be started asynchronously as it will block
  // otherwise
  graphicsDomain.startAsync();
  // Start console domain. This domain will block until enter is pressed.
  consoleDomain.start();

  // Once the console domain exits on enter, the domains will be stopped and
  // cleaned up.

  graphicsDomain.stopAsync();
  audioDomain.stop();
  consoleDomain.stop();

  graphicsDomain.cleanup();
  audioDomain.cleanup();
  consoleDomain.cleanup();
  return 0;
}
