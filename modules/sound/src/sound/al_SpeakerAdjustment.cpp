#include "al/sound/al_SpeakerAdjustment.hpp"

#include <cfloat>
#include <iostream>

using namespace al;

void SpeakerDistanceGainAdjustment::configure(Speakers layout, double expon) {
  mLayout = layout;
  float max_distance = 0.0;
  float min_distance = FLT_MAX;
  for (auto speaker : layout) {
    if (speaker.radius > max_distance) {
      max_distance = speaker.radius;
    }
    if (speaker.radius < min_distance) {
      min_distance = speaker.radius;
    }
  }
  std::cout << "Min distance " << min_distance
            << "  Max distance: " << max_distance << std::endl;
  mGains.clear();
  mGains.reserve(layout.size());
  for (auto speaker : layout) {
    double gain =
        std::pow(speaker.radius, expon) / std::pow(min_distance, expon);
    mGains.push_back(float(std::pow(gain, expon)));
    std::cout << speaker.deviceChannel << " -- " << gain << " -- "
              << mGains.back() << std::endl;
  }
}

void SpeakerDistanceGainAdjustment::processGains(AudioIOData& io) {
  if (io.channelsOut() == 60) {
    size_t counter = 0;
    for (auto speaker : mLayout) {
      float* ioBus = io.outBuffer(speaker.deviceChannel);
      int samples = io.framesPerBuffer();
      float gain = mGains[counter++];
      while (samples-- > 0) {
        *ioBus = *ioBus * gain;
        ioBus++;
      }
    }
  }
}
