#include "al/sound/al_StereoPanner.hpp"

#include <cstring>

void al::StereoPanner::renderSample(al::AudioIOData &io,
                                    const al::Pose &listeningPose,
                                    const float &sample,
                                    const unsigned int &frameIndex) {
  Vec3d vec = listeningPose.vec();
  Quatd srcRot = listeningPose.quat();
  vec = srcRot.rotate(vec);
  if (numSpeakers >= 2) {
    float gainL, gainR;
    equalPowerPan(vec, gainL, gainR);

    io.out(0, frameIndex) += gainL * sample;
    io.out(1, frameIndex) += gainR * sample;
  } else {  // don't pan
    for (unsigned int i = 0; i < numSpeakers; i++)
      io.out(i, frameIndex) = sample;
  }
}

void al::StereoPanner::renderBuffer(al::AudioIOData &io,
                                    const al::Pose &listeningPose,
                                    const float *samples,
                                    const unsigned int &numFrames) {
  Vec3d vec = listeningPose.vec();
  Quatd srcRot = listeningPose.quat();
  vec = srcRot.rotate(vec);
  if (numSpeakers >= 2) {
    float *bufL = io.outBuffer(0);
    float *bufR = io.outBuffer(1);

    float gainL, gainR;
    equalPowerPan(vec, gainL, gainR);

    for (unsigned int i = 0; i < numFrames; i++) {
      bufL[i] += gainL * samples[i];
      bufR[i] += gainR * samples[i];
    }
  } else {  // dont pan
    for (unsigned int i = 0; i < numSpeakers; i++) {
      memcpy(io.outBuffer(i), samples, sizeof(float) * numFrames);
    }
  }
}

void al::StereoPanner::equalPowerPan(const al::Vec3d &relPos, float &gainL,
                                     float &gainR) {
  double panVal = 0.5;
  if (relPos.z != 0.0 || relPos.x != 0.0) {
    panVal = 1.0 - std::fabs(std::atan2(relPos.z, relPos.x) / M_PI);
  }

  gainL = std::cos((M_PI / 2.0) * panVal);
  gainR = std::sin((M_PI / 2.0) * panVal);
}
