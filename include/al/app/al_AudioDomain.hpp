#ifndef AUDIODOMAIN_H
#define AUDIODOMAIN_H

#include <cassert>
#include <functional>
#include <iostream>
#include <memory>
#include <stack>
#include <vector>

#include "al_ComputationDomain.hpp"

#include "Gamma/Domain.h"
#include "al/io/al_AudioIO.hpp"

namespace al {

/**
 * @brief AsynchronousDomain class
 * @ingroup App
 */
class AudioDomain : public AsynchronousDomain {
 public:
  virtual ~AudioDomain() {}

  // Domain management functions
  bool initialize(ComputationDomain *parent = nullptr) override;
  bool start() override;
  bool stop() override;
  bool cleanup(ComputationDomain *parent = nullptr) override;

  AudioIO &audioIO() { return mAudioIO; }
  const AudioIO &audioIO() const { return mAudioIO; }

  void configure(double audioRate = 44100, int audioBlockSize = 512,
                 int audioOutputs = -1, int audioInputs = -1);

  void configure(AudioDevice &dev, double audioRate, int audioBlockSize,
                 int audioOutputs, int audioInputs);

  std::function<void(AudioIOData &io)> onSound = [](AudioIOData &) {};

 protected:
  static void AppAudioCB(AudioIOData &io) {
    AudioDomain &app = io.user<AudioDomain>();
    io.frame(0);
    app.onSound(app.audioIO());
  }

 private:
  AudioIO mAudioIO;
};

class GammaAudioDomain : public AudioDomain, public gam::Domain {
 public:
  bool start() override;
};

}  // namespace al

#endif  // AUDIODOMAIN_H
