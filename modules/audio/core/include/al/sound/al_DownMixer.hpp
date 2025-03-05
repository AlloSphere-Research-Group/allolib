#ifndef INCLUDE_AL_DOWNMIXER_HPP
#define INCLUDE_AL_DOWNMIXER_HPP

#include <map>
#include <vector>

#include "al/sound/al_Speaker.hpp"

namespace al {

class AudioIOData;

/**
 *  DownMixer uses buses in AudioIOData. If you are using buses in your
application, ensure that they have been created before running DownMixer.
Downmixer will create append new buses that it needs to the existing buses
on the first buffer it processes.

DownMixer will downmix to buffers, and can optionally copy the buses to outputs
                                       using the setOutputs() function.
 */
class DownMixer {
public:
  // Configure channel mapping
  void layoutToStereo(const Speakers &sl, AudioIOData &io);
  void set5_1toStereo(AudioIOData &io);

  void setStereoOutput();
  void setOutputs(std::vector<uint32_t> outs);

  // process
  void downMix(AudioIOData &io);
  void downMixToBus(AudioIOData &io);
  void copyBusToOuts(AudioIOData &io);

private:
  std::map<uint32_t, std::vector<std::pair<uint32_t, float>>> mRoutingMap;
  std::vector<uint32_t> mOuts;
  int mBusStartNumber = -1;
};

} // namespace al

#endif // INCLUDE_AL_DOWNMIXER_HPP
