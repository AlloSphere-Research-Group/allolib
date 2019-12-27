#ifndef INCLUDE_AL_PANNING_STEREO
#define INCLUDE_AL_PANNING_STEREO

#include <cstring>
#include <memory>

#include "al/math/al_Constants.hpp"
#include "al/math/al_Vec.hpp"
#include "al/sound/al_Spatializer.hpp"
#include "al/sound/al_Speaker.hpp"
#include "al/spatial/al_DistAtten.hpp"
#include "al/spatial/al_Pose.hpp"

namespace al {

///
/// \brief The StereoPanner class
///
/// @ingroup Sound
class StereoPanner : public Spatializer {
 public:
  StereoPanner(Speakers& sl) : Spatializer(sl), numSpeakers(0) {
    numSpeakers = mSpeakers.size();
    if (numSpeakers != 2) {
      std::cout << "Stereo Panner Requires exactly 2 speakers (" << numSpeakers
                << " used). First two will be used!" << std::endl;
    }
  }

  /// Per Sample Processing
  virtual void renderSample(AudioIOData& io, const Pose& listeningPose,
                            const float& sample,
                            const unsigned int& frameIndex) override;

  /// Per Buffer Processing
  virtual void renderBuffer(AudioIOData& io, const Pose& listeningPose,
                            const float* samples,
                            const unsigned int& numFrames) override;

 private:
  size_t numSpeakers;

  void equalPowerPan(const Vec3d& relPos, float& gainL, float& gainR);
};

}  // namespace al

#endif
