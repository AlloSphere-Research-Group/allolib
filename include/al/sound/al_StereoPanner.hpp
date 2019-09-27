#ifndef INCLUDE_AL_PANNING_STEREO
#define INCLUDE_AL_PANNING_STEREO

#include <memory>
#include <cstring>

#include "al/math/al_Vec.hpp"
#include "al/spatial/al_DistAtten.hpp"
#include "al/spatial/al_Pose.hpp"
#include "al/sound/al_Speaker.hpp"
#include "al/math/al_Constants.hpp"
#include "al/sound/al_Spatializer.hpp"

namespace al{

///
/// \brief The StereoPanner class
///
/// @ingroup Sound
class StereoPanner : public Spatializer{
public:
	StereoPanner(SpeakerLayout &sl) : Spatializer(sl), numSpeakers(0) {
		numSpeakers = mSpeakers.size();
		if(numSpeakers != 2) {
			printf("Stereo Panner Requires exactly 2 speakers (%i used), no panning will occur!\n", numSpeakers);
		}
	}

	///Per Sample Processing
    virtual void renderSample(AudioIOData& io, const Pose& listeningPose, const float& sample, const unsigned int& frameIndex) override;

	/// Per Buffer Processing
    virtual void renderBuffer(AudioIOData& io, const Pose& listeningPose, const float *samples, const unsigned int& numFrames) override;


private:
	int numSpeakers;

	void equalPowerPan(const Vec3d& relPos, float &gainL, float &gainR);

};


} // al::

#endif
