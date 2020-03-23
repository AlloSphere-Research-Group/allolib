#include <math.h>

#include "al/io/al_AudioIO.hpp"
#include "al/sound/al_Vbap.hpp"
#include "al/sphere/al_AlloSphereSpeakerLayout.hpp"
#include "catch.hpp"

using namespace al;

bool almostEqual(float a, float b, float tol = 0.000001) {
  return fabs(a - b) < tol;
}

TEST_CASE("VBAP 8 speakers") {
  const int fpb = 16;

  Speakers sl = OctalSpeakerLayout();
  Vbap vbapPanner(sl);

  //    vbapPanner.print();
  vbapPanner.compile();

  AudioIOData audioData;
  audioData.framesPerBuffer(fpb);
  audioData.framesPerSecond(44100);
  audioData.channelsIn(0);
  audioData.channelsOut(sl.size());

  float samples[fpb];
  for (int i = 0; i < fpb; i++) {
    samples[i] = i + 0.5;
  }

  Pose pose;
  pose.pos(0, 0, -4); // Center
  audioData.zeroOut();
  vbapPanner.renderBuffer(audioData, pose, samples, fpb);

  audioData.frame(0);
  for (int i = 0; i < fpb; i++) {
    REQUIRE(audioData.out(0, i) == i + 0.5);
    for (int chan = 1; chan < 8; chan++) {
      REQUIRE(audioData.out(chan, i) == 0.0f);
    }
  }

  pose.pos(-2, 0, 0); // Hard Left
  audioData.zeroOut();
  vbapPanner.renderBuffer(audioData, pose, samples, fpb);

  audioData.frame(0);
  for (unsigned int i = 0; i < fpb; i++) {
    for (unsigned int chan = 0; chan < 6; chan++) {
      REQUIRE(almostEqual(audioData.out(chan, i), 0.0f));
    }
    REQUIRE(almostEqual(audioData.out(6, i), i + 0.5f));
    REQUIRE(almostEqual(audioData.out(7, i), 0.0f));
  }

  pose.pos(1, 0, -1); // 45 deg. front right
  audioData.zeroOut();
  vbapPanner.renderBuffer(audioData, pose, samples, fpb);

  audioData.frame(0);
  for (unsigned int i = 0; i < fpb; i++) {
    REQUIRE(almostEqual(audioData.out(0, i), 0.0f));
    REQUIRE(almostEqual(audioData.out(1, i), i + 0.5));
    for (unsigned int chan = 2; chan < 8; chan++) {
      REQUIRE(almostEqual(audioData.out(chan, i), 0.0f));
    }
  }

  pose.pos(1 * tan(M_PI * 0.125), 0, -1); // 22.5 deg. front right
  audioData.zeroOut();
  vbapPanner.renderBuffer(audioData, pose, samples, fpb);

  audioData.frame(0);
  for (unsigned int i = 0; i < fpb; i++) {
    REQUIRE(almostEqual(audioData.out(0, i), (i + 0.5) * sin(M_PI * 0.25)));
    REQUIRE(almostEqual(audioData.out(1, i), (i + 0.5) * sin(M_PI * 0.25)));
    for (unsigned int chan = 2; chan < 8; chan++) {
      REQUIRE(almostEqual(audioData.out(chan, i), 0.0f));
    }
  }
}

TEST_CASE("VBAP 3D tetrahedron") {
  const int fpb = 16;

  Speakers sl;
  sl.emplace_back(Speaker(0, 0, -30));
  sl.emplace_back(Speaker(1, 120, -30));
  sl.emplace_back(Speaker(2, -120, -30));
  sl.emplace_back(Speaker(3, 0, 90));
  Vbap vbapPanner(sl);

  vbapPanner.set3D(true);
  vbapPanner.setOptions(
      Vbap::KEEP_SAME_ELEVATION); // Curently needed to preserve tetrahedron
                                  // triplets
  vbapPanner.compile();
  //    vbapPanner.print();

  AudioIOData audioData;
  audioData.framesPerBuffer(fpb);
  audioData.framesPerSecond(44100);
  audioData.channelsIn(0);
  audioData.channelsOut(sl.size());

  float samples[fpb];
  for (unsigned int i = 0; i < fpb; i++) {
    samples[i] = i + 0.5;
  }

  Pose pose;
  pose.pos(0, 4 * tan(M_PI / 6.0), -4); // Center
  audioData.zeroOut();
  vbapPanner.renderBuffer(audioData, pose, samples, fpb);
  for (unsigned int i = 0; i < fpb; i++) {
    REQUIRE(almostEqual(audioData.out(0, i), (i + 0.5) * sin(M_PI * 0.25)));
    REQUIRE(almostEqual(audioData.out(3, i), (i + 0.5) * sin(M_PI * 0.25)));
    for (unsigned int chan = 1; chan < 2; chan++) {
      REQUIRE(almostEqual(audioData.out(chan, i), 0.0f));
    }
  }

  pose.pos(cos(M_PI / 6.0), 0.0, -sin(M_PI / 6.0)); // Middle of right face
  audioData.zeroOut();
  vbapPanner.renderBuffer(audioData, pose, samples, fpb);
  for (unsigned int i = 0; i < fpb; i++) {
    REQUIRE(almostEqual(audioData.out(0, i), (i + 0.5) * sqrt(1 / 3.0)));
    REQUIRE(almostEqual(audioData.out(1, i),
                        (i + 0.5) * sqrt(1 / 3.0f))); // 0.225427702
    REQUIRE(almostEqual(audioData.out(3, i),
                        (i + 0.5) * sqrt(1 / 3.0))); // 0.323040754

    REQUIRE(almostEqual(audioData.out(2, i), 0.0));
  }
}

TEST_CASE("VBAP 3D Allosphere") {
  const int fpb = 16;

  Speakers sl = AlloSphereSpeakerLayout();
  Vbap vbapPanner(sl);

  vbapPanner.set3D(true);
  //    vbapPanner.compile();
  ////    vbapPanner.print();

  //    AudioIOData audioData;
  //    audioData.framesPerBuffer(fpb);
  //    audioData.framesPerSecond(44100);
  //    audioData.channelsIn(0);
  //    audioData.channelsOut(sl.numSpeakers());

  //    float samples[fpb];
  //    for (int i = 0; i < fpb; i++) {
  //        samples[i] = i + 0.5;
  //    }

  //    Pose pose;
  //    pose.pos(0,4*tan(M_PI/6.0),-4); // Center
  //    audioData.zeroOut();

  //    vbapPanner.renderBuffer(audioData, pose, samples, fpb);
  //    for (int i = 0; i < fpb; i++) {
  //        REQUIRE(almostEqual(audioData.out(0,i) , (i + 0.5) *
  //        sin(M_PI*0.25))); REQUIRE(almostEqual(audioData.out(3,i) , (i + 0.5)
  //        * sin(M_PI*0.25))); for (int chan = 1; chan < 2; chan ++) {
  //            REQUIRE(almostEqual(audioData.out(chan,i),0.0f));
  //        }
  //    }

  //    pose.pos(cos(M_PI/6.0),0.0,-sin(M_PI/6.0)); // Middle of right face
  //    audioData.zeroOut();
  //    vbapPanner.renderBuffer(audioData, pose, samples, fpb);
  //    for (int i = 0; i < fpb; i++) {
  //        REQUIRE(almostEqual(audioData.out(0,i) , (i + 0.5) * sqrt(1/3.0)));
  //        REQUIRE(almostEqual(audioData.out(2,i), (i + 0.5) * sqrt(1/3.0)));
  //        // 0.225427702 REQUIRE(almostEqual(audioData.out(3,i), (i + 0.5) *
  //        sqrt(1/3.0))); // 0.323040754

  //        REQUIRE(almostEqual(audioData.out(1,i) , 0.0));

  //    }
}
