
#include "al/sound/al_DownMixer.hpp"
#include "al/io/al_AudioIOData.hpp"

#include <cinttypes>

using namespace al;

void DownMixer::layoutToStereo(const Speakers &sl, AudioIOData &io) {
  uint32_t leftChannel = 0;
  uint32_t rightChannel = 1;
  mRoutingMap.clear();

  for (const auto &spkr : sl) {
    auto az = spkr.azimuth;
    while (az > 180) { // wrap to -180 -> 180
      az -= 180;
    }
    if (az > 90) { // fold positive around x axis
      az = 180 - az;
    }
    if (az < -90) { // fold negative around x axis
      az = -(180 + az);
    }
    auto l = cos(((0.5 * az) + 45) * M_2PI / 360);
    auto r = sin(((0.5 * az) + 45) * M_2PI / 360);
    if (l > 0) {
      mRoutingMap[spkr.deviceChannel].push_back({leftChannel, l});
    }
    if (r > 0) {
      mRoutingMap[spkr.deviceChannel].push_back({rightChannel, r});
    }
  }
  if (mBusStartNumber == -1) {

    mBusStartNumber = io.channelsBus();
    io.channelsBus(io.channelsBus() + 2);
  } else {
    // TODO check if we have enough buses
  }
}

void DownMixer::set5_1toStereo(AudioIOData &io) {
  uint32_t leftChannel = 0;
  uint32_t rightChannel = 1;
  mRoutingMap.clear();

  const float threeDbDown = powf(10, -3.0f / 20.0f);
  const float sixDbDown = powf(10, -6.0f / 20.0f);
  // L C R Ls Rs LFE
  mRoutingMap[0] = {{leftChannel, 1.0}};
  mRoutingMap[1] = {{leftChannel, threeDbDown}, {rightChannel, threeDbDown}};
  mRoutingMap[2] = {{rightChannel, 1.0}};

  mRoutingMap[3] = {{leftChannel, sixDbDown}};
  mRoutingMap[4] = {{rightChannel, sixDbDown}};
  mRoutingMap[5] = {{leftChannel, sixDbDown}, {rightChannel, sixDbDown}};
  if (mBusStartNumber == -1) {
    mBusStartNumber = io.channelsBus();
    io.channelsBus(io.channelsBus() + 2);
  } else {
    // TODO check if we have enough buses
  }
}

void DownMixer::setStereoOutput() { setOutputs({0, 1}); }

void DownMixer::setOutputs(std::vector<uint32_t> outs) { mOuts = outs; }

void DownMixer::downMixToBus(AudioIOData &io) {
  // Zero bus buffers
  for (int i = mBusStartNumber; i < io.channelsBus(); i++) {
    memset(io.busBuffer(i), 0, io.framesPerBuffer() * sizeof(float));
  }
  io.frame(0);
  while (io()) {
    for (const auto &mapEntry : mRoutingMap) {
      for (const auto &routing : mapEntry.second) {
        io.bus(routing.first) += io.out(mapEntry.first) * routing.second;
      }
    }
  }
}

void DownMixer::copyBusToOuts(AudioIOData &io) {
  int offset = io.channelsBus() - mOuts.size();
  for (size_t i = 0; i < mOuts.size(); i++) {
    if (mOuts[i] != UINT32_MAX) {
      memcpy(io.outBuffer(mOuts[i]), io.busBuffer(offset + i),
             io.framesPerBuffer() * sizeof(float));
    }
  }
}

void DownMixer::downMix(AudioIOData &io) {
  downMixToBus(io);
  copyBusToOuts(io);
}
