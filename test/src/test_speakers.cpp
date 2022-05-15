#include <math.h>

#include "al/io/al_AudioIO.hpp"
#include "al/math/al_Functions.hpp"
#include "al/sound/al_DownMixer.hpp"
#include "al/sound/al_Lbap.hpp"
#include "al/sphere/al_AlloSphereSpeakerLayout.hpp"

#include "gtest/gtest.h"

using namespace al;

TEST(Speakers, CubeLayout) {

  float side = 1.0;
  Speakers sl = CubeLayout();

  EXPECT_EQ(sl.size(), 8);
  EXPECT_EQ(sl[0].azimuth, 45);
  EXPECT_EQ(sl[1].azimuth, 135);
  EXPECT_EQ(sl[2].azimuth, 225);
  EXPECT_EQ(sl[3].azimuth, 315);
  EXPECT_EQ(sl[4].azimuth, 45);
  EXPECT_EQ(sl[5].azimuth, 135);
  EXPECT_EQ(sl[6].azimuth, 225);
  EXPECT_EQ(sl[7].azimuth, 315);

  float halfSide = side * 0.5f;
  float bottomDistance = sqrt(halfSide * halfSide + halfSide * halfSide);
  float elev = atan(bottomDistance / halfSide) * 57.29577951308232f;

  EXPECT_FLOAT_EQ(sl[0].elevation, elev);
  EXPECT_FLOAT_EQ(sl[1].elevation, elev);
  EXPECT_FLOAT_EQ(sl[2].elevation, elev);
  EXPECT_FLOAT_EQ(sl[3].elevation, elev);
  EXPECT_FLOAT_EQ(sl[4].elevation, 0.0);
  EXPECT_FLOAT_EQ(sl[5].elevation, 0.0);
  EXPECT_FLOAT_EQ(sl[6].elevation, 0.0);
  EXPECT_FLOAT_EQ(sl[7].elevation, 0.0);

  float topDistance = sqrt(bottomDistance * bottomDistance + 1);

  EXPECT_FLOAT_EQ(sl[0].radius, topDistance);
  EXPECT_FLOAT_EQ(sl[1].radius, topDistance);
  EXPECT_FLOAT_EQ(sl[2].radius, topDistance);
  EXPECT_FLOAT_EQ(sl[3].radius, topDistance);
  EXPECT_FLOAT_EQ(sl[4].radius, bottomDistance);
  EXPECT_FLOAT_EQ(sl[5].radius, bottomDistance);
  EXPECT_FLOAT_EQ(sl[6].radius, bottomDistance);
  EXPECT_FLOAT_EQ(sl[7].radius, bottomDistance);

  // Graphics space vector
  auto vec = sl[0].vecGraphics();
  EXPECT_FLOAT_EQ(vec.x, halfSide);
  EXPECT_FLOAT_EQ(vec.y, side);
  EXPECT_FLOAT_EQ(vec.z, -halfSide);

  vec = sl[1].vecGraphics();
  EXPECT_FLOAT_EQ(vec.x, halfSide);
  EXPECT_FLOAT_EQ(vec.y, side);
  EXPECT_FLOAT_EQ(vec.z, halfSide);

  vec = sl[2].vecGraphics();
  EXPECT_FLOAT_EQ(vec.x, -halfSide);
  EXPECT_FLOAT_EQ(vec.y, side);
  EXPECT_FLOAT_EQ(vec.z, halfSide);

  vec = sl[3].vecGraphics();
  EXPECT_FLOAT_EQ(vec.x, -halfSide);
  EXPECT_FLOAT_EQ(vec.y, side);
  EXPECT_FLOAT_EQ(vec.z, -halfSide);

  vec = sl[4].vecGraphics();
  EXPECT_FLOAT_EQ(vec.x, halfSide);
  EXPECT_FLOAT_EQ(vec.y, 0);
  EXPECT_FLOAT_EQ(vec.z, -halfSide);

  vec = sl[5].vecGraphics();
  EXPECT_FLOAT_EQ(vec.x, halfSide);
  EXPECT_FLOAT_EQ(vec.y, 0);
  EXPECT_FLOAT_EQ(vec.z, halfSide);

  vec = sl[6].vecGraphics();
  EXPECT_FLOAT_EQ(vec.x, -halfSide);
  EXPECT_FLOAT_EQ(vec.y, 0);
  EXPECT_FLOAT_EQ(vec.z, halfSide);

  vec = sl[7].vecGraphics();
  EXPECT_FLOAT_EQ(vec.x, -halfSide);
  EXPECT_FLOAT_EQ(vec.y, 0);
  EXPECT_FLOAT_EQ(vec.z, -halfSide);

  // Audio space vector
  vec = sl[0].vec();
  EXPECT_FLOAT_EQ(vec.x, halfSide);
  EXPECT_FLOAT_EQ(vec.y, halfSide);
  EXPECT_FLOAT_EQ(vec.z, side);

  vec = sl[1].vec();
  EXPECT_FLOAT_EQ(vec.x, halfSide);
  EXPECT_FLOAT_EQ(vec.y, -halfSide);
  EXPECT_FLOAT_EQ(vec.z, side);

  vec = sl[2].vec();
  EXPECT_FLOAT_EQ(vec.x, -halfSide);
  EXPECT_FLOAT_EQ(vec.y, -halfSide);
  EXPECT_FLOAT_EQ(vec.z, side);

  vec = sl[3].vec();
  EXPECT_FLOAT_EQ(vec.x, -halfSide);
  EXPECT_FLOAT_EQ(vec.y, halfSide);
  EXPECT_FLOAT_EQ(vec.z, side);

  vec = sl[4].vec();
  EXPECT_FLOAT_EQ(vec.x, halfSide);
  EXPECT_FLOAT_EQ(vec.y, halfSide);
  EXPECT_FLOAT_EQ(vec.z, 0);

  vec = sl[5].vec();
  EXPECT_FLOAT_EQ(vec.x, halfSide);
  EXPECT_FLOAT_EQ(vec.y, -halfSide);
  EXPECT_FLOAT_EQ(vec.z, 0);

  vec = sl[6].vec();
  EXPECT_FLOAT_EQ(vec.x, -halfSide);
  EXPECT_FLOAT_EQ(vec.y, -halfSide);
  EXPECT_FLOAT_EQ(vec.z, 0);

  vec = sl[7].vec();
  EXPECT_FLOAT_EQ(vec.x, -halfSide);
  EXPECT_FLOAT_EQ(vec.y, halfSide);
  EXPECT_FLOAT_EQ(vec.z, 0);
}

TEST(Speakers, DownMixAllosphere) {
  Speakers sl = AlloSphereSpeakerLayout();
  AudioIOData io;
  io.channelsOut(64);
  DownMixer downMixer;
  downMixer.layoutToStereo(sl, io);
  downMixer.setStereoOutput();

  // downmixer should create 2 buses
  EXPECT_EQ(io.channelsBus(), 2);
  downMixer.downMix(io);
  for (int i = 0; i < 64; i++) {
    io.frame(1);
    io.zeroOut();
    io.out(i) = 1.0;
    downMixer.downMix(io);

    io.frame(1);
    if (i == 12 || i == 13 || i == 14 || i == 15 || i == 46 || i == 47 ||
        i == 60 || i == 61 || i == 62 || i == 63) {
      EXPECT_EQ(io.out(0), 0);
      EXPECT_EQ(io.out(1), 0);
    } else {
      EXPECT_NE(io.out(0), 0);
      EXPECT_NE(io.out(1), 0);
      if (i == 23 || i == 38) {
        EXPECT_FLOAT_EQ(io.out(0), io.out(1));
      } else if (i < 3 || (i > 8 && i < 24) || (i > 38 && i < 51) ||
                 (i > 56 && i < 61)) {
        EXPECT_GT(io.out(0), io.out(1)) << "i = " << i;
      } else {
        EXPECT_GT(io.out(1), io.out(0));
      }
    }
  }
}
