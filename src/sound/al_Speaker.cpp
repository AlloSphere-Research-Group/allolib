#include "al/sound/al_Speaker.hpp"

using namespace al;

Speaker::Speaker(unsigned int deviceChan, float az, float el, int group,
                 float radius, float gain)
    : deviceChannel(deviceChan),
      azimuth(az),
      elevation(el),
      group(group),
      radius(radius),
      gain(gain) {}

void Speaker::posCart2(Vec3d xyz) {
  using namespace std;

  radius =
      sqrt(float((xyz[0] * xyz[0]) + (xyz[1] * xyz[1]) + (xyz[2] * xyz[2])));
  float gd = sqrt(float((xyz[0] * xyz[0]) + (xyz[1] * xyz[1])));
  elevation = atan2f(float(xyz[2]), gd) * 180.f / float(M_PI);
  azimuth = atan2f(float(xyz[0]), float(xyz[1])) * 180.f / float(M_PI);
}

Vec3d Speaker::vec() const {
  // TODO doxygen style commenting on coordinates like ambisonics
  double cosel = cos(toRad(double(elevation)));
  double x = cos(toRad(double(azimuth))) * cosel * double(radius);
  double y = sin(toRad(double(azimuth))) * cosel * double(radius);
  double z = sin(toRad(double(elevation))) * double(radius);
  // Ryan: the standard conversions assume +z is up, these are correct for
  // allocore

  //        double x = sin(toRad(azimuth)) * cosel * radius;
  //		double y = sin(toRad(elevation)) * radius;
  //        double z = -1*cos(toRad(azimuth)) * cosel * radius;
  return Vec3d(x, y, z);
}

Vec3d Speaker::vecGraphics() const {
  // TODO doxygen style commenting on coordinates like ambisonics
  double cosel = cos(toRad(double(elevation)));
  double x = cos(toRad(double(azimuth))) * cosel * double(radius);
  double y = sin(toRad(double(azimuth))) * cosel * double(radius);
  double z = sin(toRad(double(elevation))) * double(radius);
  // Ryan: the standard conversions assume +z is up, these are correct for
  // allocore

  //        double x = sin(toRad(azimuth)) * cosel * radius;
  //		double y = sin(toRad(elevation)) * radius;
  //        double z = -1*cos(toRad(azimuth)) * cosel * radius;
  return Vec3d(-y, z, -x);
}

// --------------------
namespace al {
Speakers HeadsetSpeakerLayout(unsigned int deviceChannelStart, float radius,
                              float gain) {
  return SpeakerRingLayout<2>(deviceChannelStart, 90, radius, gain);
}

Speakers StereoSpeakerLayout(unsigned int deviceChannelStart, float angle,
                             float distance, float gain) {
  return Speakers{{deviceChannelStart, angle, 0, 0, distance, gain},
                  {deviceChannelStart + 1, -angle, 0, 0, distance, gain}};
}

Speakers OctalSpeakerLayout(unsigned int deviceChannelStart, float phase,
                            float radius, float gain) {
  return SpeakerRingLayout<8>(deviceChannelStart, phase, radius, gain);
}

Speakers CubeLayout(unsigned int deviceChannelStart) {
  Speakers mSpeakers;
  mSpeakers.reserve(8);
  for (unsigned int i = 0; i < 4; ++i) {
    mSpeakers.emplace_back(Speaker(i + deviceChannelStart, 45.f + (i * 90), 0));
    mSpeakers.emplace_back(
        Speaker(4 + i + deviceChannelStart, 45.f + (i * 90), 60, 0, sqrt(5.f)));
  }
  return mSpeakers;
}

}  // namespace al
