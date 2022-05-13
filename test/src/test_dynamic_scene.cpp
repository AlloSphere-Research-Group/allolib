
#include "gtest/gtest.h"

#include "al/scene/al_DynamicScene.hpp"
#include "al/sound/al_Lbap.hpp"
#include "al/sphere/al_AlloSphereSpeakerLayout.hpp"

#include <fstream>

class Voice : public al::PositionedVoice {
public:
  void onProcess(al::AudioIOData &io) override {
    while (io()) {
      io.out(0) = 0.3f;
    }
  }
};

TEST(DynamicScene, SpatializerLbap3D) {
  al::AudioIOData io;
  io.channelsOut(8);
  io.framesPerBuffer(16);
  io.zeroOut();

  al::DynamicScene scene(0, al::TimeMasterMode::TIME_MASTER_FREE);
  scene.distanceAttenuation().law(al::ATTEN_NONE);
  auto spatializer = scene.setSpatializer<al::Lbap>(al::CubeLayout());
  auto *voice = scene.getVoice<Voice>();
  voice->useDistanceAttenuation(false);
  scene.triggerOn(voice);
  scene.processVoices();

  //    // front bottom
  voice->setPose(al::Pose({0, 0, -1}));
  scene.render(io);

  for (int samp = 0; samp < io.framesPerBuffer(); samp++) {
    EXPECT_NEAR(io.out(0, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(1, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(2, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(3, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(4, samp), 0.3f * cos(M_PI * 0.25), 1e-6);
    EXPECT_NEAR(io.out(5, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(6, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(7, samp), 0.3f * cos(M_PI * 0.25), 1e-6);
  }

  // front top
  io.zeroOut();
  io.frame(0);
  voice->setPose(al::Pose({0, 15, -1}));
  scene.render(io);

  for (int samp = 0; samp < io.framesPerBuffer(); samp++) {
    EXPECT_NEAR(io.out(0, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(1, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(2, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(3, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(4, samp), 0.3f * cos(M_PI * 0.25), 1e-6);
    EXPECT_NEAR(io.out(5, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(6, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(7, samp), 0.3f * cos(M_PI * 0.25), 1e-6);
  }

  //  back bottom
  io.zeroOut();
  io.frame(0);
  voice->setPose(al::Pose({0, 0, 1}));
  scene.render(io);

  for (int samp = 0; samp < io.framesPerBuffer(); samp++) {
    EXPECT_NEAR(io.out(0, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(1, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(2, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(3, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(4, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(5, samp), 0.3f * cos(M_PI * 0.25), 1e-6);
    EXPECT_NEAR(io.out(6, samp), 0.3f * cos(M_PI * 0.25), 1e-6);
    EXPECT_NEAR(io.out(7, samp), 0.0, 1e-6);
  }

  //  back top
  io.zeroOut();
  io.frame(0);
  voice->setPose(al::Pose({0, 5, 1}));
  scene.render(io);

  for (int samp = 0; samp < io.framesPerBuffer(); samp++) {
    EXPECT_NEAR(io.out(0, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(1, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(2, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(3, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(4, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(5, samp), 0.3f * cos(M_PI * 0.25), 1e-6);
    EXPECT_NEAR(io.out(6, samp), 0.3f * cos(M_PI * 0.25), 1e-6);
    EXPECT_NEAR(io.out(7, samp), 0.0, 1e-6);
  }
}

TEST(DynamicScene, SpatializerLbap2D) {
  al::AudioIOData io;
  io.channelsOut(8);
  io.framesPerBuffer(16);
  io.zeroOut();

  al::DynamicScene scene(0, al::TimeMasterMode::TIME_MASTER_FREE);
  scene.distanceAttenuation().law(al::ATTEN_NONE);
  auto spatializer = scene.setSpatializer<al::Lbap>(al::OctalSpeakerLayout());
  auto *voice = scene.getVoice<Voice>();
  voice->useDistanceAttenuation(false);
  scene.triggerOn(voice);

  // front
  voice->setPose(al::Pose({0, 0, -1}));
  scene.processVoices();
  scene.render(io);

  for (int samp = 0; samp < io.framesPerBuffer(); samp++) {
    EXPECT_NEAR(io.out(0, samp), 0.3, 1e-6);
    EXPECT_NEAR(io.out(1, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(2, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(3, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(4, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(5, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(6, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(7, samp), 0.0, 1e-6);
  }

  // right front
  io.zeroOut();
  io.frame(0);
  voice->setPose(al::Pose({1, 0, -1}));
  scene.render(io);

  for (int samp = 0; samp < io.framesPerBuffer(); samp++) {
    EXPECT_NEAR(io.out(0, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(1, samp), 0.3, 1e-6);
    EXPECT_NEAR(io.out(2, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(3, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(4, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(5, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(6, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(7, samp), 0.0, 1e-6);
  }

  //  right
  io.zeroOut();
  io.frame(0);
  voice->setPose(al::Pose({1, 0, 0}));
  scene.render(io);

  for (int samp = 0; samp < io.framesPerBuffer(); samp++) {
    EXPECT_NEAR(io.out(0, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(1, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(2, samp), 0.3, 1e-6);
    EXPECT_NEAR(io.out(3, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(4, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(5, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(6, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(7, samp), 0.0, 1e-6);
  }

  //  back right
  io.zeroOut();
  io.frame(0);
  voice->setPose(al::Pose({1, 0, 1}));
  scene.render(io);

  for (int samp = 0; samp < io.framesPerBuffer(); samp++) {
    EXPECT_NEAR(io.out(0, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(1, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(2, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(3, samp), 0.3, 1e-6);
    EXPECT_NEAR(io.out(4, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(5, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(6, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(7, samp), 0.0, 1e-6);
  }

  //  back
  io.zeroOut();
  io.frame(0);
  voice->setPose(al::Pose({0, 0, 1}));
  scene.render(io);

  for (int samp = 0; samp < io.framesPerBuffer(); samp++) {
    EXPECT_NEAR(io.out(0, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(1, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(2, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(3, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(4, samp), 0.3, 1e-6);
    EXPECT_NEAR(io.out(5, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(6, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(7, samp), 0.0, 1e-6);
  }

  //  back left
  io.zeroOut();
  io.frame(0);
  voice->setPose(al::Pose({-1, 0, 1}));
  scene.render(io);

  for (int samp = 0; samp < io.framesPerBuffer(); samp++) {
    EXPECT_NEAR(io.out(0, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(1, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(2, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(3, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(4, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(5, samp), 0.3, 1e-6);
    EXPECT_NEAR(io.out(6, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(7, samp), 0.0, 1e-6);
  }

  //  left
  io.zeroOut();
  io.frame(0);
  voice->setPose(al::Pose({-1, 0, 0}));
  scene.render(io);

  for (int samp = 0; samp < io.framesPerBuffer(); samp++) {
    EXPECT_NEAR(io.out(0, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(1, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(2, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(3, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(4, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(5, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(6, samp), 0.3, 1e-6);
    EXPECT_NEAR(io.out(7, samp), 0.0, 1e-6);
  }

  //  front left
  io.zeroOut();
  io.frame(0);
  voice->setPose(al::Pose({-1, 0, -1}));
  scene.render(io);

  for (int samp = 0; samp < io.framesPerBuffer(); samp++) {
    EXPECT_NEAR(io.out(0, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(1, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(2, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(3, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(4, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(5, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(6, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(7, samp), 0.3, 1e-6);
  }
}
