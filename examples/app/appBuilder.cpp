/*
AppBuilder — fluent Module composition (audio-only or graphics+audio).

Uses Runtime-owned Main pump (no KeepAliveDomain).

Usage:
  ./example_app_appBuilder              # graphics + audio, ~4s then quit
  ./example_app_appBuilder audio        # audio-only (Runtime idle)
*/

#include "al/app/al_AppBuilder.hpp"

#include <cmath>
#include <cstring>
#include <iostream>
#include <thread>

using namespace al;

int main(int argc, char **argv) {
  const bool audioOnly =
      argc > 1 && std::strcmp(argv[1], "audio") == 0;

  AppBuilder app;

  if (audioOnly) {
    app.audio(44100, 512, 2, 0)
        .onSound([](AudioIOData &io) {
          static double phase = 0.0;
          const double step = 2.0 * M_PI * 220.0 / io.framesPerSecond();
          while (io()) {
            float s = float(0.1 * std::sin(phase));
            io.out(0) = s;
            if (io.channelsOut() > 1)
              io.out(1) = s;
            phase += step;
          }
        });
  } else {
    app.graphics("AppBuilder", 640, 480)
        .audio(44100, 512, 2, 0)
        .onAnimate([](double) {})
        .onDraw([](Graphics &g) {
          static float t = 0.f;
          t += 0.02f;
          g.clear(0.15f + 0.1f * std::sin(t), 0.1f, 0.2f);
        })
        .onSound([](AudioIOData &io) {
          static double phase = 0.0;
          const double step = 2.0 * M_PI * 330.0 / io.framesPerSecond();
          while (io()) {
            float s = float(0.05 * std::sin(phase));
            io.out(0) = s;
            if (io.channelsOut() > 1)
              io.out(1) = s;
            phase += step;
          }
        });
  }

  std::thread([&app] {
    std::this_thread::sleep_for(std::chrono::seconds(4));
    std::cout << "[appBuilder] quitting\n";
    app.runtime().quit();
  }).detach();

  std::cout << "[appBuilder] start ("
            << (audioOnly ? "audio-only" : "graphics+audio") << ")\n";
  app.start();
  std::cout << "[appBuilder] done\n";
}
