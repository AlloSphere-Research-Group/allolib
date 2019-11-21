
#include "al/sound/al_Lbap.hpp"

#include <algorithm>

using namespace al;

void Lbap::compile() {
  std::map<int, Speakers> speakerRingMap;
  for (auto &speaker : mSpeakers) {
    if (speakerRingMap.find(speaker.group) == speakerRingMap.end()) {
      speakerRingMap[speaker.group] = Speakers();
    }
    speakerRingMap[speaker.group].push_back(speaker);
  }
  for (auto speakerRing : speakerRingMap) {
    mRings.push_back(LdapRing(speakerRing.second));
  }
  // Sort by elevation
  std::sort(mRings.begin(), mRings.end(),
            [](const LdapRing &a, const LdapRing &b) -> bool {
              return a.elevation > b.elevation;
            });
}

void Lbap::prepare(AudioIOData &io) {
  if (buffer) {
    free(buffer);
  }
  buffer = (float *)malloc(2 * io.framesPerBuffer() *
                           sizeof(float));  // Allocate 2 buffers
  bufferSize = io.framesPerBuffer();
}

void Lbap::renderSample(AudioIOData &io, const Pose &reldir,
                        const float &sample, const unsigned int &frameIndex) {}

void Lbap::renderBuffer(AudioIOData &io, const Pose &listeningPose,
                        const float *samples, const unsigned int &numFrames) {
  Vec3d vec = listeningPose.vec();

  // Rotate vector according to listener-rotation
  Quatd srcRot = listeningPose.quat();
  vec = srcRot.rotate(vec);
  vec = Vec4d(-vec.z, -vec.x, vec.y);

  float elev =
      RAD_2_DEG_SCALE * atan(vec.z / sqrt(vec.x * vec.x + vec.y * vec.y));

  auto it = mRings.begin();
  while (it != mRings.end() && it->elevation > elev) {
    it++;
  }
  if (it == mRings.begin()) {  // Top ring
    it->vbap->renderBuffer(io, listeningPose, samples, numFrames);
  } else if (it == mRings.end()) {  // Bottom ring
    mRings.back().vbap->renderBuffer(io, listeningPose, samples, numFrames);
  } else {                    // Between inner rings
    auto topRingIt = it - 1;  // top ring is previous ring
    float fraction = (elev - it->elevation) /
                     (topRingIt->elevation -
                      it->elevation);  // elevation angle between layers
    float gainTop = sin(M_PI_2 * fraction);
    float gainBottom = cos(M_PI_2 * fraction);
    for (int i = 0; i < bufferSize; i++) {
      buffer[i] = samples[i] * gainTop;
      buffer[i + bufferSize] = samples[i] * gainBottom;
    }

    topRingIt->vbap->renderBuffer(io, listeningPose, buffer, bufferSize);
    it->vbap->renderBuffer(io, listeningPose, buffer + bufferSize, bufferSize);
  }
}

void Lbap::print(std::ostream &stream) {
  for (auto ring : mRings) {
    stream << " ---- Ring at elevation:" << ring.elevation << std::endl;
    ring.vbap->print(stream);
  }
}
