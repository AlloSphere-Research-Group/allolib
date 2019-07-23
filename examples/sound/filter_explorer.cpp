// OutputMaster example
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

class MyApp : public App {
public:
  Parameter filterFrequency {"filterFrequency", "", 1200, "", 5, 5000};
  Parameter filterResonance {"filterResonance", "", 1.0, "", 0.1f, 20};
  ParameterMenu filterType {"filterType"};
  Trigger resetFilterState {"resetFilterState"};

  ControlGUI gui;


  void onInit() override {
    gam::sampleRate(audioIO().framesPerSecond());

    navControl().disable();

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

  void onDraw(Graphics &g) override {
    g.clear();

    Mesh spectrumMesh;
    spectrumMesh.primitive(Mesh::LINE_STRIP);
    for (int i = 0 ; i < STFT_SIZE/2; i++) {
      spectrumMesh.vertex(i, spectrum[i], 0.0);
    }

    g.color(1.0);
    g.pushMatrix();
    g.translate(-8.0/STFT_SIZE,0, -4.0);
    g.scale(4.0/STFT_SIZE, 100, 0.0);
    g.draw(spectrumMesh);

    g.popMatrix();

    gui.draw(g);
  }

  void onExit() override {
    gui.cleanup();
  }

private:

  gam::Biquad<> mFilter {};
  gam::NoiseWhite<> mNoise;

  gam::STFT stft {STFT_SIZE,2048, 0, gam::WindowType::HANN, gam::MAG_PHASE};
  float spectrum[STFT_SIZE];

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

