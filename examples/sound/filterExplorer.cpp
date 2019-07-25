// Filter explorer example
// By Andrés Cabrera mantaraya36@gmail.com
// July 2018

#include <iostream>


#include "al/app/al_App.hpp"
#include "al/ui/al_ControlGUI.hpp"
#include "Gamma/Filter.h"
#include "Gamma/Noise.h"
#include "Gamma/DFT.h"

using namespace al;

#define STFT_SIZE 4096

struct MyApp : public App {

  // Parameters
  Parameter filterFrequency {"filterFrequency", "", 1200, "", 5, 5000};
  Parameter filterResonance {"filterResonance", "", 1.0, "", 0.1f, 20};
  ParameterMenu filterType {"filterType"};
  Trigger resetFilterState {"resetFilterState"};

  ControlGUI gui;

  // Sound generators
  gam::Biquad<> mFilter {};
  gam::NoiseWhite<> mNoise;

  // Spectrum analysis
  gam::STFT stft {STFT_SIZE,2048, 0, gam::WindowType::HANN, gam::MAG_PHASE};
  float spectrum[STFT_SIZE];
  Mesh spectrumMesh;

  void onInit() override {
    gam::sampleRate(audioIO().framesPerSecond());

    navControl().disable();

    // Register callbacks to change filter settings when parameters change
    filterFrequency.registerChangeCallback([this](float value) {
      mFilter.freq(value);
    });
    filterResonance.registerChangeCallback([this](float value) {
      mFilter.res(value);
    });

    filterType.setElements({"low pass", "high pass", "resonant", "band reject",
                           "all_pass", "peaking", "low_shelf", "high shelf", "smoothing"});

    filterType.registerChangeCallback([this](int value) {
      mFilter.type(gam::FilterType(value));
    });

    resetFilterState.registerChangeCallback([this](bool value) {
      mFilter.zero();
    });
  }

  void onCreate() override {

    // Register parameters with GUI
    gui << filterType << filterFrequency <<filterResonance << resetFilterState;
    gui.init();
  }

  void onSound(AudioIOData &io) override {

    while(io()) {

      float noise = mNoise();
      float filtered = mFilter(noise * 0.3f);
      io.out(0) = filtered;
      io.out(1) = filtered;
      if (stft(filtered)) {
        for (int i = 0 ; i < STFT_SIZE/2; i ++) {
          stft(); // Ignore phase
          spectrum[i] = stft.bin(i).real();
        }

      }
    }

  }

  void onAnimate(double dt) override {

    spectrumMesh.reset();
    spectrumMesh.primitive(Mesh::LINE_STRIP);
    for (int i = 0 ; i < STFT_SIZE/2; i++) {
      spectrumMesh.color(HSV(0.5 - spectrum[i]* 100));
      spectrumMesh.vertex(i, spectrum[i], 0.0);
    }
  }

  void onDraw(Graphics &g) override {
    g.clear();

    g.meshColor(); // Use the color in the mesh
    g.pushMatrix();
    g.translate(-1.0, 0, -4.0);
    g.scale(4.0/STFT_SIZE, 100, 1.0);
    g.draw(spectrumMesh);

    g.popMatrix();

    gui.draw(g);
  }

  void onExit() override {
    gui.cleanup();
  }

};

int main(int argc, char *argv[])
{
  MyApp app;
  app.dimensions(640, 480);
  app.title("Biquad Explorer");
  app.initAudio();
  app.start();
  return 0;
}

