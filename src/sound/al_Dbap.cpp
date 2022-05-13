#include "al/sound/al_Dbap.hpp"

namespace al {

Dbap::Dbap(const Speakers &sl, float focus)
    : Spatializer(sl), mNumSpeakers(0), mFocus(focus) {
  mNumSpeakers = mSpeakers.size();
  std::cout << "DBAP Compiled with " << mNumSpeakers << " speakers"
            << std::endl;

  for (unsigned int i = 0; i < mNumSpeakers; i++) {
    mSpeakerVecs[i] = mSpeakers[i].vec();
    mDeviceChannels[i] = mSpeakers[i].deviceChannel;
  }
}

void Dbap::renderSample(AudioIOData &io, const Vec3f &pos, const float &sample,
                        const unsigned int &frameIndex) {
  //  Vec3d relpos = listeningPose.vec();

  //  // Rotate vector according to listener-rotation
  //  Quatd srcRot = listeningPose.quat();
  //  relpos = srcRot.rotate(relpos);
  //  relpos = Vec3d(relpos.x, relpos.z, relpos.y);

  Vec3d relpos = Vec3d(pos.x, -pos.z, pos.y);
  for (unsigned int i = 0; i < mNumSpeakers; ++i) {
    float gain = 1.f;
    Vec3d vec = relpos - mSpeakerVecs[i];
    double dist = vec.mag();
    gain = 1.f / (1.f + float(dist));
    gain = powf(gain, mFocus);

    io.out(mDeviceChannels[i], frameIndex) += gain * sample;
  }
}

void Dbap::renderBuffer(AudioIOData &io, const Vec3f &pos, const float *samples,
                        const unsigned int &numFrames) {
  //  Vec3d relpos = listeningPose.vec();

  //  // Rotate vector according to listener-rotation
  //  Quatd srcRot = listeningPose.quat();
  //  relpos = srcRot.rotate(relpos);

  // FIXME test DBAP
  Vec3d relpos = Vec3d(pos.x, -pos.z, pos.y);

  for (unsigned int k = 0; k < mNumSpeakers; ++k) {
    float gain = 1.f;

    Vec3d vec = relpos - mSpeakerVecs[k];
    double dist = vec.mag();
    gain = 1.0f / (1.0f + float(dist));
    gain = powf(gain, mFocus);

    float *out = io.outBuffer(mDeviceChannels[k]);
    for (size_t i = 0; i < numFrames; ++i) {
      out[i] += gain * samples[i];
    }
  }
}

void Dbap::print(std::ostream &stream) {
  stream << "Using DBAP Panning- need to add panner info for print function"
         << std::endl;
}

} // namespace al
