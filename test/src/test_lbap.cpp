#include <math.h>

#include "al/io/al_AudioIO.hpp"
#include "al/math/al_Functions.hpp"
#include "al/sound/al_Lbap.hpp"
#include "al/sphere/al_AlloSphereSpeakerLayout.hpp"

#include "gtest/gtest.h"

using namespace al;

// TEST(LBAP, LBAPRing8_Mid) {
//  const int fpb = 16;

//  Speakers sl = CubeLayout();
//  Lbap lbapPanner(sl);

//  int numSpeakers = sl.size();

//  lbapPanner.compile();

//  lbapPanner.print();

//  AudioIOData audioData;
//  audioData.framesPerBuffer(fpb);
//  audioData.framesPerSecond(44100);
//  audioData.channelsIn(0);
//  audioData.channelsOut(8);

//  lbapPanner.prepare(audioData);

//  float samples[fpb];
//  for (int i = 0; i < fpb; i++) {
//    samples[i] = i + 0.5f;
//  }

//  Vec3f pos;

//  auto elevTopDeg = sl[0].elevation;
//  auto elevTopMid = atan(2);

//  float diag = sl[4].radius;
//  pos = Vec3f(0, 0.5 * tan(0.5 * elevTopMid), -0.5); // Front Center
//  audioData.zeroOut();
//  lbapPanner.renderBuffer(audioData, pos, samples, fpb);
//  for (unsigned int i = 0; i < fpb; i++) {
//    for (unsigned int chan = 0; chan < numSpeakers; chan++) {
//      if (chan == 0 || chan == 3 || chan == 4 || chan == 7) {
//        EXPECT_FLOAT_EQ(audioData.out(chan, i),
//                        (i + 0.5f) * cos(M_PI * 0.25) * cos(M_PI * 0.25));
//      } else {
//        EXPECT_FLOAT_EQ(audioData.out(chan, i), 0.0f);
//      }
//    }
//  }

//  pos = Vec3f(0.5, diag * tan(0.5 * (elevTopDeg * M_2PI / 360.0)),
//              -0.5); // Front Right
//  audioData.zeroOut();
//  lbapPanner.renderBuffer(audioData, pos, samples, fpb);
//  for (unsigned int i = 0; i < fpb; i++) {
//    for (unsigned int chan = 0; chan < numSpeakers; chan++) {
//      if (chan == 0 || chan == 4) {
//        EXPECT_FLOAT_EQ(audioData.out(chan, i), (i + 0.5f) * cos(M_PI *
//        0.25));
//      } else {
//        EXPECT_NEAR(audioData.out(chan, i), 0.0f, 1e-10);
//      }
//    }
//  }

//  pos = Vec3f(-0.5, diag * tan(0.5 * (elevTopDeg * M_2PI / 360.0)),
//              -0.5); // Front Left
//  audioData.zeroOut();
//  lbapPanner.renderBuffer(audioData, pos, samples, fpb);
//  for (unsigned int i = 0; i < fpb; i++) {
//    for (unsigned int chan = 0; chan < numSpeakers; chan++) {
//      if (chan == 3) {
//        EXPECT_FLOAT_EQ(audioData.out(chan, i), (i + 0.5f));
//      } else {
//        EXPECT_NEAR(audioData.out(chan, i), 0.0f, 1e-10);
//      }
//    }
//  }

//  pos = Vec3f(0.5, diag * tan(0.5 * (elevTopDeg * M_2PI / 360.0)),
//              0.5); // Back Right
//  audioData.zeroOut();
//  lbapPanner.renderBuffer(audioData, pos, samples, fpb);
//  for (unsigned int i = 0; i < fpb; i++) {
//    for (unsigned int chan = 0; chan < numSpeakers; chan++) {
//      if (chan == 1) {
//        EXPECT_FLOAT_EQ(audioData.out(chan, i), (i + 0.5f));
//      } else {
//        EXPECT_NEAR(audioData.out(chan, i), 0.0f, 1e-10);
//      }
//    }
//  }

//  pos = Vec3f(-4, 0.5, 1 * cos(M_PI * 0.25)); // Back Left
//  audioData.zeroOut();
//  lbapPanner.renderBuffer(audioData, pos, samples, fpb);
//  for (unsigned int i = 0; i < fpb; i++) {
//    for (unsigned int chan = 0; chan < numSpeakers; chan++) {
//      if (chan == 2) {
//        EXPECT_FLOAT_EQ(audioData.out(chan, i), (i + 0.5f));
//      } else {
//        EXPECT_NEAR(audioData.out(chan, i), 0.0f, 1e-10);
//      }
//    }
//  }

//  pos = Vec3f(4, 20, 0); // hard right
//  audioData.zeroOut();
//  lbapPanner.renderBuffer(audioData, pos, samples, fpb);
//  for (int i = 0; i < fpb; i++) {
//    for (int chan = 0; chan < numSpeakers; chan++) {
//      if (chan == 0 || chan == 1) {
//        EXPECT_FLOAT_EQ(audioData.out(chan, i), (i + 0.5f) * cos(M_PI *
//        0.25));
//      } else {
//        EXPECT_FLOAT_EQ(audioData.out(chan, i), 0.0f);
//      }
//    }
//  }

//  pos = Vec3f(-4, 20, 0); // hard left
//  audioData.zeroOut();
//  lbapPanner.renderBuffer(audioData, pos, samples, fpb);
//  for (int i = 0; i < fpb; i++) {
//    for (int chan = 0; chan < numSpeakers; chan++) {
//      if (chan == 2 || chan == 3) {
//        EXPECT_FLOAT_EQ(audioData.out(chan, i), (i + 0.5f) * cos(M_PI *
//        0.25));
//      } else {
//        EXPECT_FLOAT_EQ(audioData.out(chan, i), 0.0f);
//      }
//    }
//  }
//}

TEST(LBAP, LBAPRing8_Top) {
  const int fpb = 16;

  Speakers sl = CubeLayout();
  Lbap lbapPanner(sl);

  int numSpeakers = sl.size();

  lbapPanner.compile();

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
        EXPECT_NEAR(audioData.out(chan, i), 0.0f, 1e-10);
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
        EXPECT_NEAR(audioData.out(chan, i), 0.0f, 1e-10);
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
        EXPECT_NEAR(audioData.out(chan, i), 0.0f, 1e-10);
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
        EXPECT_NEAR(audioData.out(chan, i), 0.0f, 1e-10);
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

TEST(LBAP, LBAPRing8_Bottom) {
  const int fpb = 16;

  Speakers sl = CubeLayout();
  Lbap lbapPanner(sl);

  int numSpeakers = sl.size();

  lbapPanner.compile();

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
        EXPECT_NEAR(audioData.out(chan, i), 0.0f, 1e-10);
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
        EXPECT_NEAR(audioData.out(chan, i), 0.0f, 1e-10);
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
        EXPECT_NEAR(audioData.out(chan, i), 0.0f, 1e-10);
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
        EXPECT_NEAR(audioData.out(chan, i), 0.0f, 1e-10);
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
