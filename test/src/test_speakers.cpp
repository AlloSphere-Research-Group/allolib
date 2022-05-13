#include <math.h>

#include "al/io/al_AudioIO.hpp"
#include "al/math/al_Functions.hpp"
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
