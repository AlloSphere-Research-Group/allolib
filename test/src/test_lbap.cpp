#include <math.h>

#include "al/io/al_AudioIO.hpp"
#include "al/math/al_Functions.hpp"
#include "al/sound/al_Lbap.hpp"
#include "al/sphere/al_AlloSphereSpeakerLayout.hpp"
#include "catch.hpp"

using namespace al;

TEST_CASE("LBAP Allosphere") {
  const int fpb = 16;

  Speakers sl = AlloSphereSpeakerLayout();
  Lbap lbapPanner(sl);

  lbapPanner.compile();

  lbapPanner.print();

  AudioIOData audioData;
  audioData.framesPerBuffer(fpb);
  audioData.framesPerSecond(44100);
  audioData.channelsIn(0);
  audioData.channelsOut(sl.size());

  lbapPanner.prepare(audioData);

  float samples[fpb];
  for (int i = 0; i < fpb; i++) {
    samples[i] = i + 0.5f;
  }

  Pose pose;
  pose.pos(0, 0, -4);  // Front Center
  audioData.zeroOut();
  lbapPanner.renderBuffer(audioData, pose, samples, fpb);
  for (unsigned int i = 0; i < fpb; i++) {
    for (unsigned int chan = 0; chan < 54; chan++) {
      if (chan == 23) {
        REQUIRE(aeq(audioData.out(23, i), (i + 0.5f)));
      } else {
        REQUIRE(aeq(audioData.out(chan, i), 0.0f));
      }
    }
  }

  //  pose.pos(4, 0, 0);  // hard right (between speakers 30 and 31)
  //  audioData.zeroOut();
  //  lbapPanner.renderBuffer(audioData, pose, samples, fpb);
  //  for (int i = 0; i < fpb; i++) {
  //    for (int chan = 0; chan < 54; chan++) {
  //      if (chan == 30 || chan == 31) {
  //        REQUIRE(aeq((float)audioData.out(chan, i),
  //                    (float)cos(0.25f * M_PI) * (i + 0.5f)));
  //      } else {
  //        REQUIRE(aeq(audioData.out(chan, i), 0.0f));
  //      }
  //    }
  //  }

  //  pose.pos(-1, tan(2 * M_PI * 20.5 / 360.0),
  //           0);  // hard left between two rings (between speakers 16 and 45
  //                // middle ring and 0 and 11 on top ring)
  //  audioData.zeroOut();
  //  lbapPanner.renderBuffer(audioData, pose, samples, fpb);
  //  for (int i = 0; i < fpb; i++) {
  //    for (int chan = 0; chan < 54; chan++) {
  //      if (chan == 16 || chan == 45 || chan == 0 || chan == 11) {
  //        REQUIRE(aeq((float)audioData.out(chan, i), (float)0.5 * (i +
  //        0.5f)));
  //      } else {
  //        REQUIRE(aeq(audioData.out(chan, i), 0.0f));
  //      }
  //    }
  //  }
}
