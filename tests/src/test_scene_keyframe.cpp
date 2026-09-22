#include "gtest/gtest.h"

#include "al/parameter/al_KeyframeTrack.hpp"
#include "al/parameter/al_Scene.hpp"
#include "al/parameter/al_Signal.hpp"
#include "al/scene/al_SynthVoice.hpp"
#include "al/scene/al_VoiceEntity.hpp"

using namespace al;

TEST(SceneWorld, CreateDestroyCapture) {
  World world("stage"); // alias of Scene
  Entity &a = world.create("actor");
  Entity &b = world.create("prop");
  EXPECT_EQ(world.size(), 2u);
  EXPECT_EQ(a.path(), "stage/actor");
  EXPECT_EQ(b.path(), "stage/prop");

  SignalFloat ax{"x", "", 0.f};
  SignalFloat bx{"x", "", 0.f};
  a.addParam(ax);
  b.addParam(bx);
  EXPECT_EQ(ax.getFullAddress(), "/stage/actor/x");
  EXPECT_EQ(bx.getFullAddress(), "/stage/prop/x");

  ax.set(1.f);
  bx.set(2.f);
  ParamState snap = world.capture();
  ax.set(0.f);
  bx.set(0.f);
  world.apply(snap);
  EXPECT_FLOAT_EQ(ax.get(), 1.f);
  EXPECT_FLOAT_EQ(bx.get(), 2.f);

  EXPECT_TRUE(world.destroy(a.id()));
  EXPECT_EQ(world.size(), 1u);
  EXPECT_EQ(world.find(a.id()), nullptr);
  EXPECT_NE(world.findByName("prop"), nullptr);
}

TEST(KeyframeTrack, LerpAndEase) {
  SignalFloat p{"p", "g", 0.f};
  std::vector<ParameterMeta *> targets{&p};

  ParamState s0;
  s0["/g/p"] = {0.f};
  ParamState s1;
  s1["/g/p"] = {10.f};

  KeyframeTrack track("motion");
  track.add(0.0, s0);
  track.add(2.0, s1);

  track.apply(0.0, targets);
  EXPECT_FLOAT_EQ(p.get(), 0.f);
  track.apply(1.0, targets);
  EXPECT_FLOAT_EQ(p.get(), 5.f);
  track.apply(2.0, targets);
  EXPECT_FLOAT_EQ(p.get(), 10.f);

  track.setEase(Ease::Smoothstep);
  // Midpoint with smoothstep is still 0.5 for the unit curve
  EXPECT_DOUBLE_EQ(KeyframeTrack::easeUnit(Ease::Smoothstep, 0.5), 0.5);
  track.apply(1.0, targets);
  EXPECT_FLOAT_EQ(p.get(), 5.f);
}

TEST(VoiceEntity, BindAndCapture) {
  Scene scene("poly");
  SynthVoice voice;
  Parameter freq{"freq", "", 440.f};
  Parameter gain{"gain", "", 0.5f};
  voice << freq; // trigger
  voice.registerParameter(gain);

  Entity &e = bindVoiceEntity(scene, voice, "v0");
  EXPECT_EQ(e.path(), "poly/v0");
  EXPECT_EQ(freq.getFullAddress(), "/poly/v0/freq");
  EXPECT_EQ(gain.getFullAddress(), "/poly/v0/gain");

  freq.set(220.f);
  gain.set(0.25f);
  ParamState snap = scene.capture();
  freq.set(0.f);
  gain.set(0.f);
  scene.apply(snap);
  EXPECT_FLOAT_EQ(freq.get(), 220.f);
  EXPECT_FLOAT_EQ(gain.get(), 0.25f);

  // Voice-level capture/apply without scene
  ParamState voiceSnap = captureVoiceState(voice);
  freq.set(1.f);
  applyVoiceState(voice, voiceSnap);
  EXPECT_FLOAT_EQ(freq.get(), 220.f);
}
