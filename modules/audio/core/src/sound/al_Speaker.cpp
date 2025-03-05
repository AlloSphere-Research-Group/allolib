#include "al/sound/al_Speaker.hpp"

using namespace al;

Speaker::Speaker(unsigned int deviceChan, float az, float el, int group,
                 float radius, float gain)
    : deviceChannel(deviceChan), azimuth(az), elevation(el), group(group),
      radius(radius), gain(gain) {}

void Speaker::posCart2(Vec3d xyz) {
  using namespace std;

  radius =
      sqrt(float((xyz[0] * xyz[0]) + (xyz[1] * xyz[1]) + (xyz[2] * xyz[2])));
  float gd = sqrt(float((xyz[0] * xyz[0]) + (xyz[1] * xyz[1])));
  elevation = atan2f(float(xyz[2]), gd) * 180.f / float(M_PI);
  azimuth = atan2f(float(xyz[0]), float(xyz[1])) * 180.f / float(M_PI);
}

Vec3d Speaker::vec() const {
  double cosel = cos(toRad(double(elevation)));
  //  double x = cos(toRad(double(azimuth))) * cosel * double(radius);
  //  double y = -sin(toRad(double(azimuth))) * cosel * double(radius);
  double x = sin(toRad(double(azimuth))) * cosel * double(radius);
  double y = cos(toRad(double(azimuth))) * cosel * double(radius);
  double z = sin(toRad(double(elevation))) * double(radius);
  // Ryan: the standard conversions assume +z is up, these are correct for
  // allocore
  return Vec3d(x, y, z);
}

Vec3d Speaker::vecGraphics() const {
  double cosel = cos(toRad(double(elevation)));
  double x = sin(toRad(double(azimuth))) * cosel * double(radius);
  double y = sin(toRad(double(elevation))) * double(radius);
  double z = -cos(toRad(double(azimuth))) * cosel * double(radius);
  return Vec3d(x, y, z);
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

Speakers CubeLayout(unsigned int deviceChannelStart, float cubeSide) {
  Speakers mSpeakers;

  float halfSide = 0.5 * cubeSide;
  float bottomDistance = sqrt(halfSide * halfSide + halfSide * halfSide);
  float elevationAngle = atan(1.0f / bottomDistance);
  float topElevationAngle = (elevationAngle * 57.29577951308232); // 360/(2*pi);
  float bottomElevation = 0.0;
  float topDistance = sqrt(bottomDistance * bottomDistance + 1);
  mSpeakers.reserve(8);
  // Top square

  mSpeakers.emplace_back(
      Speaker(0 + deviceChannelStart, 45.f, topElevationAngle, 0, topDistance));
  mSpeakers.emplace_back(Speaker(1 + deviceChannelStart, 45.f + 90,
                                 topElevationAngle, 0, topDistance));
  mSpeakers.emplace_back(Speaker(2 + deviceChannelStart, 45.f + 180,
                                 topElevationAngle, 0, topDistance));
  mSpeakers.emplace_back(Speaker(3 + deviceChannelStart, 45.f + 270,
                                 topElevationAngle, 0, topDistance));
  // bottom square
  mSpeakers.emplace_back(Speaker(4 + deviceChannelStart, 45.f, bottomElevation,
                                 1, bottomDistance));
  mSpeakers.emplace_back(Speaker(5 + deviceChannelStart, 45.f + 90,
                                 bottomElevation, 1, bottomDistance));
  mSpeakers.emplace_back(Speaker(6 + deviceChannelStart, 45.f + 180,
                                 bottomElevation, 1, bottomDistance));
  mSpeakers.emplace_back(Speaker(7 + deviceChannelStart, 45.f + 270,
                                 bottomElevation, 1, bottomDistance));

  return mSpeakers;
}

} // namespace al
