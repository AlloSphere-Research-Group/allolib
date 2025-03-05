
#include "al/sound/al_Lbap.hpp"

#include <algorithm>
#include <cmath>

using namespace al;

void Lbap::compile() {
  std::map<int, Speakers> speakerRingMap;
  std::map<int, float> elevation;
  std::map<int, std::vector<int>> virtualSpeakers;
  for (auto &speaker : mSpeakers) {
    if (speakerRingMap.find(speaker.group) == speakerRingMap.end()) {
      // assumes elevation of ring is elevation of first speaker in ring
      elevation[speaker.group] = speaker.elevation;
    }
    speakerRingMap[speaker.group].push_back(speaker);
  }
  for (auto speakerRing : speakerRingMap) {
    mRings.push_back(LdapRing(speakerRing.second));
  }
  // Sort by elevation (high to low
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
                           sizeof(float)); // Allocate 2 buffers
  bufferSize = io.framesPerBuffer();
}

void Lbap::renderSample(AudioIOData &io, const Vec3f &reldir,
                        const float &sample, const unsigned int &frameIndex) {

  assert(0 == 1);
  // FIXME implement
}

void Lbap::renderBuffer(AudioIOData &io, const Vec3f &reldir,
                        const float *samples, const unsigned int &numFrames) {
  //  Vec3d vec = reldir;

  // Rotate vector according to listener-rotation
  //  Quatd srcRot = listeningPose.quat();
  //  vec = srcRot.rotate(vec);

  // Turn to audio space
  //  vec = Vec3d(vec.x, vec.z, -vec.y);

  float elev =
      RAD_2_DEG_SCALE *
      atan2f(reldir.y, sqrt(reldir.x * reldir.x + reldir.z * reldir.z));

  auto it = mRings.begin();
  while (it != mRings.end() && it->elevation > elev) {
    it++;
  }
  if (it == mRings.begin()) { // Above Top ring
    it->vbap->renderBuffer(io, reldir, samples, numFrames);
    if (mRings.size() > 1) {
      // assumes circular ring (all elevations and radii equal)
      float fraction = (elev - it->vbap->speakerLayout()[0].elevation) /
                       (90 - it->vbap->speakerLayout()[0].elevation);
      assert(fraction <= 1.0);
      if (fraction > mDispersionOffset) {
        // Adjust fraction to effective fraction (discarding offset
        fraction = (fraction - mDispersionOffset) / (1.0 - mDispersionOffset);
        // Using linear adjustment. should other adjustment be used?
        float dispersionBaseGain = 1.0 / sqrt(it->vbap->speakerLayout().size());
        float disperseFractionGain = sin(fraction * M_PI_2);
        float focusedFractionGain = cos(fraction * M_PI_2);

        io.frame(0);
        while (io()) { // Add dispersion

          for (const auto &spkr : it->vbap->speakerLayout()) {
            if (io.out(spkr.deviceChannel) == 0.0) {
              io.out(spkr.deviceChannel) += disperseFractionGain *
                                            dispersionBaseGain *
                                            samples[io.frame()];
            } else {
              io.out(spkr.deviceChannel) *=
                  dispersionBaseGain +
                  focusedFractionGain * (1 - dispersionBaseGain);
            }
          }
        }
      }
    }
  } else if (it == mRings.end()) { // Below Bottom ring
    mRings.back().vbap->renderBuffer(io, reldir, samples, numFrames);
    if (mRings.size() > 1) {
      // assumes circular ring (all elevations and radii equal)
      float fraction =
          (elev - mRings.back().vbap->speakerLayout()[0].elevation) /
          (-90 - mRings.back().vbap->speakerLayout()[0].elevation);
      if (fraction > 1.0) {
        fraction = 1.0;
      } else if (fraction < 0.0) {
        fraction = 0.0;
      }
      if (fraction > mDispersionOffset) {
        // Adjust fraction to effective fraction (discarding offset
        fraction = (fraction - mDispersionOffset) / (1.0 - mDispersionOffset);
        // Using linear adjustment. should other adjustment be used?
        float dispersionBaseGain =
            1.0 / sqrt(mRings.back().vbap->speakerLayout().size());
        float disperseFractionGain = cos(fraction * M_PI_2);
        float focusedFractionGain = sin(fraction * M_PI_2);

        io.frame(0);
        while (io()) { // Add dispersion

          for (const auto &spkr : mRings.back().vbap->speakerLayout()) {
            if (io.out(spkr.deviceChannel) == 0.0) {
              io.out(spkr.deviceChannel) += disperseFractionGain *
                                            dispersionBaseGain *
                                            samples[io.frame()];
            } else {
              io.out(spkr.deviceChannel) *= focusedFractionGain;
            }
          }
        }
      }
    }

  } else {                   // Between inner rings
    auto topRingIt = it - 1; // top ring is previous ring
    float fraction = (elev - it->elevation) /
                     (topRingIt->elevation -
                      it->elevation); // elevation angle between layers
    float gainTop = sin(M_PI_2 * fraction);
    float gainBottom = cos(M_PI_2 * fraction);
    for (int i = 0; i < bufferSize; i++) {
      buffer[i] = samples[i] * gainTop;
      buffer[i + bufferSize] = samples[i] * gainBottom;
    }

    // TODO we should do dispersion on inner rings too
    if (gainTop != 0) {
      topRingIt->vbap->renderBuffer(io, reldir, buffer, bufferSize);
    }
    if (gainBottom != 0) {
      it->vbap->renderBuffer(io, reldir, buffer + bufferSize, bufferSize);
    }
  }
}

void Lbap::print(std::ostream &stream) {
  for (const auto &ring : mRings) {
    stream << " ---- Ring at elevation:" << ring.elevation << std::endl;
    ring.vbap->print(stream);
  }
}
