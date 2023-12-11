
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

TEST(DynamicScene, SpatializerLbapAllosphere2Voices) {
  al::AudioIOData io;
  io.channelsOut(64);
  io.framesPerBuffer(16);
  io.zeroOut();

  al::DynamicScene scene(0, al::TimeMasterMode::TIME_MASTER_FREE);
  scene.distanceAttenuation().law(al::ATTEN_NONE);
  auto spatializer =
      scene.setSpatializer<al::Lbap>(al::AlloSphereSpeakerLayout());
  auto *voice = scene.getVoice<Voice>();
  voice->useDistanceAttenuation(false);

  auto *voice2 = scene.getVoice<Voice>();
  voice2->useDistanceAttenuation(false);
  scene.triggerOn(voice);
  scene.triggerOn(voice2);
  scene.processVoices();

  // front + back
  voice->setPose(al::Pose({0, 0, -1}));
  voice2->setPose(al::Pose({0, 0, 1}));
  scene.render(io);

  for (int samp = 0; samp < io.framesPerBuffer(); samp++) {
    for (int i = 0; i < 64; i++) {
      if (i == 23 || i == 38) {
        EXPECT_NEAR(io.out(i, samp), 0.3f, 1e-6);
      } else {
        EXPECT_NEAR(io.out(i, samp), 0.0, 1e-6);
      }
    }
  }

  // front top + back bottom
  voice->setPose(al::Pose({0, 2, -1}));
  voice2->setPose(al::Pose({0, -2, 1}));
  io.zeroOut();
  io.frame(0);
  scene.render(io);

  //  for (int samp = 0; samp < io.framesPerBuffer(); samp++) {
  //    for (int i = 0; i < 64; i++) {
  //      if (i == 2 || i == 3 || i == 56 || i == 57) {
  //        EXPECT_NEAR(io.out(i, samp), 0.6f, 1e-6);
  //      } else {
  //        EXPECT_NEAR(io.out(i, samp), 0.0, 1e-6);
  //      }
  //    }
  //  }
}

TEST(DynamicScene, SpatializerLbapAllosphere) {
  al::AudioIOData io;
  io.channelsOut(64);
  io.framesPerBuffer(16);
  io.zeroOut();

  al::DynamicScene scene(0, al::TimeMasterMode::TIME_MASTER_FREE);
  scene.distanceAttenuation().law(al::ATTEN_NONE);
  auto spatializer =
      scene.setSpatializer<al::Lbap>(al::AlloSphereSpeakerLayout());
  auto *voice = scene.getVoice<Voice>();
  voice->useDistanceAttenuation(false);
  scene.triggerOn(voice);
  scene.processVoices();

  // front
  voice->setPose(al::Pose({0, 0, -1}));
  scene.render(io);

  for (int samp = 0; samp < io.framesPerBuffer(); samp++) {
    for (int i = 0; i < 64; i++) {
      if (i == 23) {
        EXPECT_NEAR(io.out(i, samp), 0.3f, 1e-6);
      } else {
        EXPECT_NEAR(io.out(i, samp), 0.0, 1e-6);
      }
    }
  }

  // front top
  io.zeroOut();
  io.frame(0);
  voice->setPose(al::Pose({0, 2, -1}));
  scene.render(io);

  for (int samp = 0; samp < io.framesPerBuffer(); samp++) {
    for (int i = 0; i < 64; i++) {
      if (i == 2 || i == 3) {
        EXPECT_NEAR(io.out(i, samp), 0.3f * cos(M_PI * 0.25), 1e-6);
      } else {
        EXPECT_NEAR(io.out(i, samp), 0.0, 1e-6);
      }
    }
  }

  //  //  back bottom
  //  io.zeroOut();
  //  io.frame(0);
  //  voice->setPose(al::Pose({0, 0, 1}));
  //  scene.render(io);

  //  for (int samp = 0; samp < io.framesPerBuffer(); samp++) {
  //    EXPECT_NEAR(io.out(0, samp), 0.0, 1e-6);
  //    EXPECT_NEAR(io.out(1, samp), 0.0, 1e-6);
  //    EXPECT_NEAR(io.out(2, samp), 0.0, 1e-6);
  //    EXPECT_NEAR(io.out(3, samp), 0.0, 1e-6);
  //    EXPECT_NEAR(io.out(4, samp), 0.0, 1e-6);
  //    EXPECT_NEAR(io.out(5, samp), 0.3f * cos(M_PI * 0.25), 1e-6);
  //    EXPECT_NEAR(io.out(6, samp), 0.3f * cos(M_PI * 0.25), 1e-6);
  //    EXPECT_NEAR(io.out(7, samp), 0.0, 1e-6);
  //  }
}

TEST(DynamicScene, SpatializerLbap3D) {
  al::AudioIOData io;
  io.channelsOut(8);
  io.framesPerBuffer(16);
  io.zeroOut();

  al::DynamicScene scene(0, al::TimeMasterMode::TIME_MASTER_FREE);
  scene.distanceAttenuation().law(al::ATTEN_NONE);
  auto spatializer = scene.setSpatializer<al::Lbap>(al::CubeLayout());
  spatializer->setDispersionThreshold(0.999);
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
  voice->setPose(al::Pose({0, 2, -1}));
  scene.render(io);

  for (int samp = 0; samp < io.framesPerBuffer(); samp++) {
    EXPECT_NEAR(io.out(0, samp), 0.3f * cos(M_PI * 0.25), 1e-6);
    EXPECT_NEAR(io.out(1, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(2, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(3, samp), 0.3f * cos(M_PI * 0.25), 1e-6);
    EXPECT_NEAR(io.out(4, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(5, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(6, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(7, samp), 0.0, 1e-6);
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
  voice->setPose(al::Pose({0, 2, 1}));
  scene.render(io);

  for (int samp = 0; samp < io.framesPerBuffer(); samp++) {
    EXPECT_NEAR(io.out(0, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(1, samp), 0.3f * cos(M_PI * 0.25), 1e-6);
    EXPECT_NEAR(io.out(2, samp), 0.3f * cos(M_PI * 0.25), 1e-6);
    EXPECT_NEAR(io.out(3, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(4, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(5, samp), 0.0, 1e-6);
    EXPECT_NEAR(io.out(6, samp), 0.0, 1e-6);
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
