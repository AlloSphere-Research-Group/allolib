#include "al/sound/al_StereoPanner.hpp"

#include <cmath>
#include <cstring>

void al::StereoPanner::renderSample(al::AudioIOData &io, const al::Vec3f &pos,
                                    const float &sample,
                                    const unsigned int &frameIndex) {
  Vec3d vec = pos;
  if (numSpeakers >= 2) {
    float gainL, gainR;
    equalPowerPan(vec, gainL, gainR);

    io.out(0, frameIndex) += gainL * sample;
    io.out(1, frameIndex) += gainR * sample;
  } else { // don't pan
    for (unsigned int i = 0; i < numSpeakers; i++)
      io.out(i, frameIndex) = sample;
  }
}

void al::StereoPanner::renderBuffer(al::AudioIOData &io, const al::Vec3f &pos,
                                    const float *samples,
                                    const unsigned int &numFrames) {
  Vec3d vec = pos;
  if (numSpeakers >= 2) {
    float *bufL = io.outBuffer(0);
    float *bufR = io.outBuffer(1);

    float gainL, gainR;
    equalPowerPan(vec, gainL, gainR);

    for (unsigned int i = 0; i < numFrames; i++) {
      bufL[i] += gainL * samples[i];
      bufR[i] += gainR * samples[i];
    }
  } else { // dont pan
    for (unsigned int i = 0; i < numSpeakers; i++) {
      memcpy(io.outBuffer(i), samples, sizeof(float) * numFrames);
    }
  }
}

void al::StereoPanner::equalPowerPan(const al::Vec3d &relPos, float &gainL,
                                     float &gainR) {
  // Expect listener-local OpenGL frame: +X right, +Y up, -Z forward.
  // Lateral fraction x/|xz| → pan 0 (left) .. 1 (right); front/back stay center.
  double panVal = 0.5;
  const double r = std::hypot(relPos.x, relPos.z);
  if (r > 1e-12) {
    panVal = 0.5 + 0.5 * (relPos.x / r);
  }

  gainL = static_cast<float>(std::cos((M_PI / 2.0) * panVal));
  gainR = static_cast<float>(std::sin((M_PI / 2.0) * panVal));
}
