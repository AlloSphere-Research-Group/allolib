#ifndef AUDIODOMAIN_H
#define AUDIODOMAIN_H

#include <stack>
#include <vector>
#include <memory>
#include <iostream>
#include <functional>
#include <cassert>

#include "al_ComputationDomain.hpp"

#include "al/io/al_AudioIO.hpp"
#include "Gamma/Domain.h"

namespace  al
{

class AudioDomain : public AsynchronousDomain, public gam::Domain
{
public:

  virtual ~AudioDomain() {}

  // Domain management functions
  bool initialize(ComputationDomain *parent = nullptr) override;
  bool start() override;
  bool stop() override;
  bool cleanup(ComputationDomain *parent = nullptr) override;

  AudioIO& audioIO(){ return mAudioIO; }
  const AudioIO& audioIO() const { return mAudioIO; }


  // initialize audio with default values from default device
  enum AudioIOConfig : unsigned int {
    IN_ONLY = 0b1,
    OUT_ONLY = 0b10,
    IN_AND_OUT = 0b11
  };

  void configure(double audioRate, int audioBlockSize,
      int audioOutputs, int audioInputs,
      int device = -1);

  void configure(AudioDevice &dev,
                 double audioRate, int audioBlockSize,
                 int audioOutputs, int audioInputs);

  void configure(AudioIOConfig config = OUT_ONLY);


  std::function<void(AudioIOData &io)> onSound = [](AudioIOData &){};

protected:

  static void AppAudioCB(AudioIOData& io){
    AudioDomain& app = io.user<AudioDomain>();
    io.frame(0);
    app.onSound(app.audioIO());
  }
private:

  AudioIO mAudioIO;
};

}


#endif // AUDIODOMAIN_H
