#include <math.h>

#include "al/io/al_AudioIO.hpp"
#include "al/math/al_Functions.hpp"
#include "al/sound/al_Lbap.hpp"
#include "al/sphere/al_AlloSphereSpeakerLayout.hpp"

#include "gtest/gtest.h"

using namespace al;

TEST(LBAP, LBAPCube_Mid) {
  const int fpb = 16;

  Speakers sl = CubeLayout();
  Lbap lbapPanner(sl);

  int numSpeakers = sl.size();

  lbapPanner.compile();
  lbapPanner.setDispersionThreshold(0.999);

  lbapPanner.print();

  AudioIOData audioData;
  audioData.framesPerBuffer(fpb);
  audioData.framesPerSecond(44100);
  audioData.channelsIn(0);
  audioData.channelsOut(8);

  lbapPanner.prepare(audioData);

  float samples[fpb];
  for (int i = 0; i < fpb; i++) {
    samples[i] = i + 0.5f;
  }

  Vec3f pos;

  auto elevTopDeg = sl[0].elevation;
  float halfSide = 0.5;
  float bottomDistance = sqrt(halfSide * halfSide + halfSide * halfSide);
  float elevationAngle = atan(1.0f / bottomDistance);

  float diag = sl[4].radius;
  pos = Vec3f(0, 0.5 * tan(0.5 * elevationAngle), -0.5); // Front Center
  audioData.zeroOut();
  lbapPanner.renderBuffer(audioData, pos, samples, fpb);
  for (unsigned int i = 0; i < fpb; i++) {
    for (unsigned int chan = 0; chan < numSpeakers; chan++) {
      if (chan == 0 || chan == 3 || chan == 4 || chan == 7) {
        EXPECT_FLOAT_EQ(audioData.out(chan, i),
                        (i + 0.5f) * cos(M_PI * 0.25) * cos(M_PI * 0.25));
      } else {
        EXPECT_FLOAT_EQ(audioData.out(chan, i), 0.0f);
      }
    }
  }

  pos = Vec3f(0.5, diag * tan(0.5 * (elevTopDeg * M_2PI / 360.0)),
              -0.5); // Front Right
  audioData.zeroOut();
  lbapPanner.renderBuffer(audioData, pos, samples, fpb);
  for (unsigned int i = 0; i < fpb; i++) {
    for (unsigned int chan = 0; chan < numSpeakers; chan++) {
      if (chan == 0 || chan == 4) {
        EXPECT_FLOAT_EQ(audioData.out(chan, i), (i + 0.5f) * cos(M_PI * 0.25));
      } else {
        EXPECT_NEAR(audioData.out(chan, i), 0.0f, 1e-7);
      }
    }
  }

  pos = Vec3f(-0.5, diag * tan(0.5 * (elevTopDeg * M_2PI / 360.0)),
              -0.5); // Front Left
  audioData.zeroOut();
  lbapPanner.renderBuffer(audioData, pos, samples, fpb);
  for (unsigned int i = 0; i < fpb; i++) {
    for (unsigned int chan = 0; chan < numSpeakers; chan++) {
      if (chan == 3 || chan == 7) {
        EXPECT_FLOAT_EQ(audioData.out(chan, i), (i + 0.5f) * cos(M_PI * 0.25));
      } else {
        EXPECT_NEAR(audioData.out(chan, i), 0.0f, 1e-7);
      }
    }
  }

  pos = Vec3f(0.5, diag * tan(0.5 * (elevTopDeg * M_2PI / 360.0)),
              0.5); // Back Right
  audioData.zeroOut();
  lbapPanner.renderBuffer(audioData, pos, samples, fpb);
  for (unsigned int i = 0; i < fpb; i++) {
    for (unsigned int chan = 0; chan < numSpeakers; chan++) {
      if (chan == 1 || chan == 5) {
        EXPECT_FLOAT_EQ(audioData.out(chan, i), (i + 0.5f) * cos(M_PI * 0.25));
      } else {
        EXPECT_NEAR(audioData.out(chan, i), 0.0f, 1e-7);
      }
    }
  }

  pos = Vec3f(-0.5, diag * tan(0.5 * (elevTopDeg * M_2PI / 360.0)),
              0.5); // Back Left
  audioData.zeroOut();
  lbapPanner.renderBuffer(audioData, pos, samples, fpb);
  for (unsigned int i = 0; i < fpb; i++) {
    for (unsigned int chan = 0; chan < numSpeakers; chan++) {
      if (chan == 2 || chan == 6) {
        EXPECT_FLOAT_EQ(audioData.out(chan, i), (i + 0.5f) * cos(M_PI * 0.25));
      } else {
        EXPECT_NEAR(audioData.out(chan, i), 0.0f, 1e-7);
      }
    }
  }

  pos = Vec3f(0.5, 0.5 * tan(0.5 * elevationAngle), 0); // hard right
  audioData.zeroOut();
  lbapPanner.renderBuffer(audioData, pos, samples, fpb);
  for (int i = 0; i < fpb; i++) {
    for (int chan = 0; chan < numSpeakers; chan++) {
      if (chan == 0 || chan == 1 || chan == 4 || chan == 5) {
        EXPECT_FLOAT_EQ(audioData.out(chan, i),
                        (i + 0.5f) * cos(M_PI * 0.25) * cos(M_PI * 0.25));
      } else {
        EXPECT_FLOAT_EQ(audioData.out(chan, i), 0.0f);
      }
    }
  }

  pos = Vec3f(-0.5, 0.5 * tan(0.5 * elevationAngle), 0); // hard left
  audioData.zeroOut();
  lbapPanner.renderBuffer(audioData, pos, samples, fpb);
  for (int i = 0; i < fpb; i++) {
    for (int chan = 0; chan < numSpeakers; chan++) {
      if (chan == 2 || chan == 3 || chan == 6 || chan == 7) {
        EXPECT_FLOAT_EQ(audioData.out(chan, i),
                        (i + 0.5f) * cos(M_PI * 0.25) * cos(M_PI * 0.25));
      } else {
        EXPECT_FLOAT_EQ(audioData.out(chan, i), 0.0f);
      }
    }
  }
}

TEST(LBAP, LBAPCube_Top) {
  const int fpb = 16;

  Speakers sl = CubeLayout();
  Lbap lbapPanner(sl);

  int numSpeakers = sl.size();

  lbapPanner.compile();
  lbapPanner.setDispersionThreshold(0.999);

  lbapPanner.print();

  AudioIOData audioData;
  audioData.framesPerBuffer(fpb);
  audioData.framesPerSecond(44100);
  audioData.channelsIn(0);
  audioData.channelsOut(8);

  lbapPanner.prepare(audioData);

  float samples[fpb];
  for (int i = 0; i < fpb; i++) {
    samples[i] = i + 0.5f;
  }

  Vec3f pos;
  pos = Vec3f(0, 20, -4); // Front Center
  audioData.zeroOut();
  lbapPanner.renderBuffer(audioData, pos, samples, fpb);
  for (unsigned int i = 0; i < fpb; i++) {
    for (unsigned int chan = 0; chan < numSpeakers; chan++) {
      if (chan == 0 || chan == 3) {
        EXPECT_FLOAT_EQ(audioData.out(chan, i), (i + 0.5f) * cos(M_PI * 0.25));
      } else {
        EXPECT_FLOAT_EQ(audioData.out(chan, i), 0.0f);
      }
    }
  }

  pos = Vec3f(4, 20, -4); // Front Right
  audioData.zeroOut();
  lbapPanner.renderBuffer(audioData, pos, samples, fpb);
  for (unsigned int i = 0; i < fpb; i++) {
    for (unsigned int chan = 0; chan < numSpeakers; chan++) {
      if (chan == 0) {
        EXPECT_FLOAT_EQ(audioData.out(chan, i), (i + 0.5f));
      } else {
        EXPECT_NEAR(audioData.out(chan, i), 0.0f, 1e-7);
      }
    }
  }

  pos = Vec3f(-4, 20, -4); // Front Left
  audioData.zeroOut();
  lbapPanner.renderBuffer(audioData, pos, samples, fpb);
  for (unsigned int i = 0; i < fpb; i++) {
    for (unsigned int chan = 0; chan < numSpeakers; chan++) {
      if (chan == 3) {
        EXPECT_FLOAT_EQ(audioData.out(chan, i), (i + 0.5f));
      } else {
        EXPECT_NEAR(audioData.out(chan, i), 0.0f, 1e-7);
      }
    }
  }

  pos = Vec3f(4, 20, 4); // Back Right
  audioData.zeroOut();
  lbapPanner.renderBuffer(audioData, pos, samples, fpb);
  for (unsigned int i = 0; i < fpb; i++) {
    for (unsigned int chan = 0; chan < numSpeakers; chan++) {
      if (chan == 1) {
        EXPECT_FLOAT_EQ(audioData.out(chan, i), (i + 0.5f));
      } else {
        EXPECT_NEAR(audioData.out(chan, i), 0.0f, 1e-7);
      }
    }
  }

  pos = Vec3f(-4, 20, 4); // Back Left
  audioData.zeroOut();
  lbapPanner.renderBuffer(audioData, pos, samples, fpb);
  for (unsigned int i = 0; i < fpb; i++) {
    for (unsigned int chan = 0; chan < numSpeakers; chan++) {
      if (chan == 2) {
        EXPECT_FLOAT_EQ(audioData.out(chan, i), (i + 0.5f));
      } else {
        EXPECT_NEAR(audioData.out(chan, i), 0.0f, 1e-7);
      }
    }
  }

  pos = Vec3f(4, 20, 0); // hard right
  audioData.zeroOut();
  lbapPanner.renderBuffer(audioData, pos, samples, fpb);
  for (int i = 0; i < fpb; i++) {
    for (int chan = 0; chan < numSpeakers; chan++) {
      if (chan == 0 || chan == 1) {
        EXPECT_FLOAT_EQ(audioData.out(chan, i), (i + 0.5f) * cos(M_PI * 0.25));
      } else {
        EXPECT_FLOAT_EQ(audioData.out(chan, i), 0.0f);
      }
    }
  }

  pos = Vec3f(-4, 20, 0); // hard left
  audioData.zeroOut();
  lbapPanner.renderBuffer(audioData, pos, samples, fpb);
  for (int i = 0; i < fpb; i++) {
    for (int chan = 0; chan < numSpeakers; chan++) {
      if (chan == 2 || chan == 3) {
        EXPECT_FLOAT_EQ(audioData.out(chan, i), (i + 0.5f) * cos(M_PI * 0.25));
      } else {
        EXPECT_FLOAT_EQ(audioData.out(chan, i), 0.0f);
      }
    }
  }
}

TEST(LBAP, LBAPCube_Bottom) {
  const int fpb = 16;

  Speakers sl = CubeLayout();
  Lbap lbapPanner(sl);

  int numSpeakers = sl.size();

  lbapPanner.compile();
  lbapPanner.setDispersionThreshold(0.999);

  lbapPanner.print();

  AudioIOData audioData;
  audioData.framesPerBuffer(fpb);
  audioData.framesPerSecond(44100);
  audioData.channelsIn(0);
  audioData.channelsOut(8);

  lbapPanner.prepare(audioData);

  float samples[fpb];
  for (int i = 0; i < fpb; i++) {
    samples[i] = i + 0.5f;
  }

  Vec3f pos;
  pos = Vec3f(0, 0, -4); // Front Center
  audioData.zeroOut();
  lbapPanner.renderBuffer(audioData, pos, samples, fpb);
  for (unsigned int i = 0; i < fpb; i++) {
    for (unsigned int chan = 0; chan < numSpeakers; chan++) {
      if (chan == 4 || chan == 7) {
        EXPECT_FLOAT_EQ(audioData.out(chan, i), (i + 0.5f) * cos(M_PI * 0.25));
      } else {
        EXPECT_FLOAT_EQ(audioData.out(chan, i), 0.0f);
      }
    }
  }

  pos = Vec3f(4, 0, -4); // Bottom Front Right
  audioData.zeroOut();
  lbapPanner.renderBuffer(audioData, pos, samples, fpb);
  for (unsigned int i = 0; i < fpb; i++) {
    for (unsigned int chan = 0; chan < numSpeakers; chan++) {
      if (chan == 4) {
        EXPECT_FLOAT_EQ(audioData.out(chan, i), (i + 0.5f));
      } else {
        EXPECT_NEAR(audioData.out(chan, i), 0.0f, 1e-7);
      }
    }
  }

  pos = Vec3f(-4, 0, -4); // Bottom Front Left
  audioData.zeroOut();
  lbapPanner.renderBuffer(audioData, pos, samples, fpb);
  for (unsigned int i = 0; i < fpb; i++) {
    for (unsigned int chan = 0; chan < numSpeakers; chan++) {
      if (chan == 7) {
        EXPECT_FLOAT_EQ(audioData.out(chan, i), (i + 0.5f));
      } else {
        EXPECT_NEAR(audioData.out(chan, i), 0.0f, 1e-7);
      }
    }
  }

  pos = Vec3f(4, 0, 4); // Bottom Back Right
  audioData.zeroOut();
  lbapPanner.renderBuffer(audioData, pos, samples, fpb);
  for (unsigned int i = 0; i < fpb; i++) {
    for (unsigned int chan = 0; chan < numSpeakers; chan++) {
      if (chan == 5) {
        EXPECT_FLOAT_EQ(audioData.out(chan, i), (i + 0.5f));
      } else {
        EXPECT_NEAR(audioData.out(chan, i), 0.0f, 1e-7);
      }
    }
  }

  pos = Vec3f(-4, 0, 4); // Bottom Back Left
  audioData.zeroOut();
  lbapPanner.renderBuffer(audioData, pos, samples, fpb);
  for (unsigned int i = 0; i < fpb; i++) {
    for (unsigned int chan = 0; chan < numSpeakers; chan++) {
      if (chan == 6) {
        EXPECT_FLOAT_EQ(audioData.out(chan, i), (i + 0.5f));
      } else {
        EXPECT_NEAR(audioData.out(chan, i), 0.0f, 1e-7);
      }
    }
  }

  pos = Vec3f(4, 0, 0); // hard right
  audioData.zeroOut();
  lbapPanner.renderBuffer(audioData, pos, samples, fpb);
  for (int i = 0; i < fpb; i++) {
    for (int chan = 0; chan < numSpeakers; chan++) {
      if (chan == 4 || chan == 5) {
        EXPECT_FLOAT_EQ(audioData.out(chan, i), (i + 0.5f) * cos(M_PI * 0.25));
      } else {
        EXPECT_FLOAT_EQ(audioData.out(chan, i), 0.0f);
      }
    }
  }

  pos = Vec3f(-4, 0, 0); // hard left
  audioData.zeroOut();
  lbapPanner.renderBuffer(audioData, pos, samples, fpb);
  for (int i = 0; i < fpb; i++) {
    for (int chan = 0; chan < numSpeakers; chan++) {
      if (chan == 6 || chan == 7) {
        EXPECT_FLOAT_EQ(audioData.out(chan, i), (i + 0.5f) * cos(M_PI * 0.25));
      } else {
        EXPECT_FLOAT_EQ(audioData.out(chan, i), 0.0f);
      }
    }
  }
}

TEST(LBAP, LBAPAllosphere) {
  const int fpb = 16;

  Speakers sl = AlloSphereSpeakerLayout();
  Lbap lbapPanner(sl);

  lbapPanner.compile();

  lbapPanner.setDispersionThreshold(0.999);

  lbapPanner.print();

  AudioIOData audioData;
  audioData.framesPerBuffer(fpb);
  audioData.framesPerSecond(44100);
  audioData.channelsIn(0);
  audioData.channelsOut(60);

  lbapPanner.prepare(audioData);

  float samples[fpb];
  for (int i = 0; i < fpb; i++) {
    samples[i] = i + 0.5f;
  }

  Vec3f pos;
  pos = Vec3f(0, 0, -4); // Front Center
  audioData.zeroOut();
  lbapPanner.renderBuffer(audioData, pos, samples, fpb);
  for (unsigned int i = 0; i < fpb; i++) {
    for (unsigned int chan = 0; chan < 60; chan++) {
      if (chan == 23) {
        EXPECT_FLOAT_EQ(audioData.out(23, i), (i + 0.5f));
      } else {
        EXPECT_FLOAT_EQ(audioData.out(chan, i), 0.0f);
      }
    }
  }

  pos = Vec3f(4, 0, 0); // hard right (between speakers 30 and 31)
  audioData.zeroOut();
  lbapPanner.renderBuffer(audioData, pos, samples, fpb);
  for (int i = 0; i < fpb; i++) {
    for (int chan = 0; chan < 60; chan++) {
      if (chan == 30 || chan == 31) {
        EXPECT_FLOAT_EQ(audioData.out(chan, i), cos(0.25f * M_PI) * (i + 0.5f));
      } else {
        EXPECT_FLOAT_EQ(audioData.out(chan, i), 0.0f);
      }
    }
  }

  pos = Vec3f(-1, tan(2 * M_PI * 20.5 / 360.0), 0);
  // hard left between two rings (between speakers 16 and 45
  // middle ring and 0 and 11 on top ring)
  audioData.zeroOut();
  lbapPanner.renderBuffer(audioData, pos, samples, fpb);
  for (int i = 0; i < fpb; i++) {
    for (int chan = 0; chan < 60; chan++) {
      if (chan == 16 || chan == 45 || chan == 0 || chan == 11) {
        EXPECT_FLOAT_EQ(audioData.out(chan, i), 0.5 * (i + 0.5f));
      } else {
        EXPECT_FLOAT_EQ(audioData.out(chan, i), 0.0f);
      }
    }
  }

  // top ring center front
  pos = Vec3f(0, 20, -4);
  audioData.zeroOut();
  lbapPanner.renderBuffer(audioData, pos, samples, fpb);
  for (int i = 0; i < fpb; i++) {
    for (int chan = 0; chan < 60; chan++) {
      if (chan == 2 || chan == 3) {
        EXPECT_FLOAT_EQ(audioData.out(chan, i), cos(M_PI * 0.25) * (i + 0.5f));
      } else {
        EXPECT_FLOAT_EQ(audioData.out(chan, i), 0.0f);
      }
    }
  }
}

TEST(LBAP, LBAPAllosphereDispersion) {
  const int fpb = 16;

  Speakers sl = AlloSphereSpeakerLayout();
  Lbap lbapPanner(sl);

  lbapPanner.compile();

  lbapPanner.print();

  AudioIOData audioData;
  audioData.framesPerBuffer(fpb);
  audioData.framesPerSecond(44100);
  audioData.channelsIn(0);
  audioData.channelsOut(60);

  lbapPanner.prepare(audioData);
  lbapPanner.setDispersionThreshold(0.5);

  float samples[fpb];
  for (int i = 0; i < fpb; i++) {
    samples[i] = i + 0.5f;
  }

  Vec3f pos;

  // from above
  pos = Vec3f(0, 5, 0);
  audioData.zeroOut();
  lbapPanner.renderBuffer(audioData, pos, samples, fpb);
  for (int i = 0; i < fpb; i++) {
    for (int chan = 0; chan < 60; chan++) {
      if (chan == 0 || chan == 1 || chan == 2 || chan == 3 || chan == 4 ||
          chan == 5 || chan == 6 || chan == 7 || chan == 8 || chan == 9 ||
          chan == 10 || chan == 11) {
        EXPECT_GT(audioData.out(chan, i), 1e-6);
        EXPECT_FLOAT_EQ(audioData.out(chan, i), audioData.out(0, i));
        //        std::cout << audioData.out(0, i) << std::endl;
      } else {
        EXPECT_NEAR(audioData.out(chan, i), 0, 1e-6);
      }
    }
  }

  // from above but under dispersion offset
  pos = Vec3f(0, 2, -1);
  audioData.zeroOut();
  lbapPanner.renderBuffer(audioData, pos, samples, fpb);
  for (int i = 0; i < fpb; i++) {
    for (int chan = 0; chan < 60; chan++) {
      if (chan == 2 || chan == 3) {
        EXPECT_GT(audioData.out(chan, i), 1e-6);
      } else {
        EXPECT_NEAR(audioData.out(chan, i), 0, 1e-6);
      }
    }
  }

  // at speaker elevation (no dispersion)
  float highElev = sl[0].elevation;
  pos = Vec3f(0, 1 / cos(highElev * M_2PI / 360.0), -1);
  audioData.zeroOut();
  lbapPanner.renderBuffer(audioData, pos, samples, fpb);
  for (int i = 0; i < fpb; i++) {
    for (int chan = 0; chan < 60; chan++) {
      if (chan == 2 || chan == 3) {
        EXPECT_GT(audioData.out(chan, i), 1e-6);
      } else {
        EXPECT_NEAR(audioData.out(chan, i), 0, 1e-6);
      }
    }
  }

  // at dispersion threshold
  float dispThreshold = 0.5 * (90 + sl[0].elevation);
  pos = Vec3f(0, tan(dispThreshold * M_2PI / 360.0), -1);
  audioData.zeroOut();
  lbapPanner.renderBuffer(audioData, pos, samples, fpb);
  for (int i = 0; i < fpb; i++) {
    for (int chan = 0; chan < 60; chan++) {
      if (chan == 2 || chan == 3) {
        EXPECT_GT(audioData.out(chan, i), 1e-6);
      } else {
        EXPECT_NEAR(audioData.out(chan, i), 0, 1e-6);
      }
    }
  }

  // at half dispersion
  float halfDispersion = 0.75 * (90 - sl[0].elevation) + sl[0].elevation;
  pos = Vec3f(0, tan(halfDispersion * M_2PI / 360.0), -1);
  audioData.zeroOut();
  lbapPanner.renderBuffer(audioData, pos, samples, fpb);
  for (int i = 0; i < fpb; i++) {
    for (int chan = 0; chan < 60; chan++) {
      if (chan == 2 || chan == 3) {
        EXPECT_NEAR(
            audioData.out(chan, i),
            (i + 0.5) * cos(0.25 * M_PI) *        // spread between 2 speakers
                ((1.0 / sqrt(12))                 // base gain
                 + cos(0.25 * M_PI)               // half dispersion
                       * (1 - (1.0 / sqrt(12)))), // range between base and 1
            1e-6);
      } else if (chan < 12) {
        EXPECT_NEAR(audioData.out(chan, i),
                    (i + 0.5) * cos(0.25 * M_PI) / sqrt(12), 1e-6);
      } else {
        EXPECT_LT(audioData.out(chan, i), 1e-6);
      }
    }
  }
}
