
#include "gtest/gtest.h"

#include "al/ui/al_ParameterBundle.hpp"
#include "al/ui/al_PresetHandler.hpp"
#include "al/ui/al_PresetSequencer.hpp"

#include <fstream>

TEST(Presets, ParameterValues) {
  al::Parameter p{"param", "group", 0.5f, 0.0, 1.0};
  al::ParameterInt pint{"paramint", "group", 3, 1, 10};
  al::ParameterColor pcolor{"paramcolor", "group", al::Color(0.1f, 0.1f, 0.1f)};
  al::ParameterPose pose{"parampose", "group"};

  pose.set(al::Pose({1, 1, 1}));
  al::PresetHandler ph{al::TimeMasterMode::TIME_MASTER_FREE};

  ph << p << pint << pcolor;
  ph << pose;

  ph.setMorphTime(0.3f);
  ph.setMorphStepTime(0.1f);
  al::PresetHandler::ParameterStates states;

  states["/group/param"] = {1.0f};
  states["/group/paramint"] = {6};
  states["/group/parampose"] = {4, 0.25, -2};
  ph.morphTo(states, 0.3f);

  EXPECT_FLOAT_EQ(p.get(), 0.5f);
  EXPECT_EQ(pint.get(), 3);
  EXPECT_EQ(pose.get().x(), 1);
  EXPECT_EQ(pose.get().y(), 1);
  EXPECT_EQ(pose.get().z(), 1);
  ph.stepMorphing();
  EXPECT_FLOAT_EQ(p.get(), 0.5f);
  EXPECT_EQ(pint.get(), 3);
  EXPECT_EQ(pose.get().x(), 1);
  EXPECT_EQ(pose.get().y(), 1);
  EXPECT_EQ(pose.get().z(), 1);
  ph.stepMorphing();
  EXPECT_FLOAT_EQ(p.get(), 0.6666667f);
  EXPECT_EQ(pint.get(), 4);
  EXPECT_FLOAT_EQ(pose.get().x(), 2);
  EXPECT_FLOAT_EQ(pose.get().y(), 0.75);
  EXPECT_NEAR(pose.get().z(), 0.0, 1e-6);
  ph.stepMorphing();
  EXPECT_FLOAT_EQ(p.get(), 0.8333333f);
  EXPECT_EQ(pint.get(), 5);
  EXPECT_FLOAT_EQ(pose.get().x(), 3);
  EXPECT_FLOAT_EQ(pose.get().y(), 0.5);
  EXPECT_FLOAT_EQ(pose.get().z(), -1);
  ph.stepMorphing();
  EXPECT_FLOAT_EQ(p.get(), 1.0f);
  EXPECT_EQ(pint.get(), 6);
  EXPECT_FLOAT_EQ(pose.get().x(), 4);
  EXPECT_FLOAT_EQ(pose.get().y(), 0.25);
  EXPECT_FLOAT_EQ(pose.get().z(), -2);
}

TEST(Presets, RecallSynchronous) {

  al::Parameter p{"param", "group", 0.5f, 0.0, 1.0};
  al::ParameterInt pint{"paramint", "group", 3, 1, 10};
  al::ParameterColor pcolor{"paramcolor", "group", al::Color(0.1f, 0.1f, 0.1f)};

  al::PresetHandler ph;
  ph << p << pint << pcolor;

  ph.storePreset("1");
  p.set(0.8f);
  pint.set(9);
  pcolor.set({0.4f, 0.3f, 0.2f});
  ph.storePreset("2");
  p.set(0.1f);
  pint.set(4);
  pcolor.set({0.31f, 0.33f, 0.36f});
  ph.storePreset("3");

  ph.recallPresetSynchronous("1");
  EXPECT_EQ(p.get(), 0.5f);
  EXPECT_EQ(pint.get(), 3);
  EXPECT_EQ(pcolor.get().r, 0.1f);
  EXPECT_EQ(pcolor.get().g, 0.1f);
  EXPECT_EQ(pcolor.get().b, 0.1f);

  ph.recallPresetSynchronous("2");
  EXPECT_EQ(p.get(), 0.8f);
  EXPECT_EQ(pint.get(), 9);
  EXPECT_EQ(pcolor.get().r, 0.4f);
  EXPECT_EQ(pcolor.get().g, 0.3f);
  EXPECT_EQ(pcolor.get().b, 0.2f);

  ph.recallPresetSynchronous("3");
  EXPECT_EQ(p.get(), 0.1f);
  EXPECT_EQ(pint.get(), 4);
  EXPECT_EQ(pcolor.get().r, 0.31f);
  EXPECT_EQ(pcolor.get().g, 0.33f);
  EXPECT_EQ(pcolor.get().b, 0.36f);
}

TEST(Presets, BundleRecallSynchronous) {
  al::Parameter freq{"freq", "osc", 440.f};
  al::ParameterBundle voice{"voice"};
  voice << freq;

  al::PresetHandler ph{al::TimeMasterMode::TIME_MASTER_FREE};
  ph << voice;

  freq.set(220.f);
  ph.storePreset("low");
  freq.set(880.f);
  ph.storePreset("high");

  ph.recallPresetSynchronous("low");
  EXPECT_FLOAT_EQ(freq.get(), 220.f);
  ph.recallPresetSynchronous("high");
  EXPECT_FLOAT_EQ(freq.get(), 880.f);
}

TEST(Presets, NestedBundleRecall) {
  al::Parameter dry{"level", "amp", 0.5f};
  al::Parameter wet{"level", "amp", 0.2f};
  al::ParameterBundle fx{"fx"};
  fx << wet;
  al::ParameterBundle voice{"voice"};
  voice << dry;
  voice.addBundle(fx, "reverb");

  al::PresetHandler ph{al::TimeMasterMode::TIME_MASTER_FREE};
  ph << voice;

  dry.set(0.1f);
  wet.set(0.9f);
  ph.storePreset("nested_a");
  dry.set(0.8f);
  wet.set(0.1f);
  ph.storePreset("nested_b");

  ph.recallPresetSynchronous("nested_a");
  EXPECT_FLOAT_EQ(dry.get(), 0.1f);
  EXPECT_FLOAT_EQ(wet.get(), 0.9f);
  ph.recallPresetSynchronous("nested_b");
  EXPECT_FLOAT_EQ(dry.get(), 0.8f);
  EXPECT_FLOAT_EQ(wet.get(), 0.1f);
}

TEST(Presets, StringWithSpaces) {
  al::ParameterString label{"label", "ui", "hi"};
  al::PresetHandler ph{al::TimeMasterMode::TIME_MASTER_FREE};
  ph << label;

  label.set("hello world");
  ph.storePreset("str_spaces");
  label.set("x");
  ph.recallPresetSynchronous("str_spaces");
  EXPECT_EQ(label.get(), "hello world");
}

TEST(Presets, Vec3AndPoseMorph) {
  al::ParameterVec3 pos{"pos", "obj", al::Vec3f(0, 0, 0)};
  al::ParameterPose pose{"pose", "obj"};
  pose.set(al::Pose({0, 0, 0}));

  al::PresetHandler ph{al::TimeMasterMode::TIME_MASTER_FREE};
  ph << pos << pose;
  ph.setMorphTime(0.3f);
  ph.setMorphStepTime(0.1f);

  al::PresetHandler::ParameterStates states;
  states["/obj/pos"] = {3.f, 6.f, 9.f};
  // Full pose: xyz + quat(wxyz). Identity quat ≈ (1,0,0,0)
  states["/obj/pose"] = {3.f, 0.f, -3.f, 1.f, 0.f, 0.f, 0.f};
  ph.morphTo(states, 0.3f);

  ph.stepMorphing(); // count 0 → often no change yet depending on phase
  ph.stepMorphing();
  // After progress ~1/3: pos ≈ (1,2,3), pose.z ≈ -1
  EXPECT_NEAR(pos.get().x, 1.f, 0.01f);
  EXPECT_NEAR(pos.get().y, 2.f, 0.01f);
  EXPECT_NEAR(pos.get().z, 3.f, 0.01f);
  EXPECT_NEAR(pose.get().z(), -1.0, 0.01);

  ph.stepMorphing();
  ph.stepMorphing();
  EXPECT_NEAR(pos.get().x, 3.f, 0.01f);
  EXPECT_NEAR(pos.get().y, 6.f, 0.01f);
  EXPECT_NEAR(pos.get().z, 9.f, 0.01f);
  EXPECT_NEAR(pose.get().x(), 3.0, 0.01);
  EXPECT_NEAR(pose.get().z(), -3.0, 0.01);
}

TEST(Presets, PresetInterpolation) {

  al::Parameter p{"param", "group", 0.5f, 0.0, 1.0};
  al::ParameterInt pint{"paramint", "group", 3, 1, 10};
  al::ParameterColor pcolor{"paramcolor", "group", al::Color(0.1f, 0.1f, 0.1f)};

  al::PresetHandler ph;
  ph << p << pint << pcolor;

  p.set(0.8f);
  pint.set(9);
  pcolor.set({0.4f, 0.3f, 0.2f});
  ph.storePreset("2");
  p.set(0.1f);
  pint.set(4);
  pcolor.set({0.31f, 0.33f, 0.36f});
  ph.storePreset("3");

  ph.setInterpolatedPreset("2", "3", 0.0f);

  EXPECT_FLOAT_EQ(p.get(), 0.8f);
  EXPECT_EQ(pint.get(), 9);
  EXPECT_FLOAT_EQ(pcolor.get().r, 0.4f);
  EXPECT_FLOAT_EQ(pcolor.get().g, 0.3f);
  EXPECT_FLOAT_EQ(pcolor.get().b, 0.2f);

  ph.setInterpolatedPreset("2", "3", 1.0f);

  EXPECT_EQ(p.get(), 0.1f);
  EXPECT_EQ(pint.get(), 4);
  EXPECT_FLOAT_EQ(pcolor.get().r, 0.31f);
  EXPECT_FLOAT_EQ(pcolor.get().g, 0.33f);
  EXPECT_FLOAT_EQ(pcolor.get().b, 0.36f);

  ph.setInterpolatedPreset("2", "3", 0.5f);

  EXPECT_FLOAT_EQ(p.get(), 0.45f);
  EXPECT_EQ(pint.get(), 6);
  EXPECT_FLOAT_EQ(pcolor.get().r, 0.355f);
  EXPECT_FLOAT_EQ(pcolor.get().g, 0.315f);
  EXPECT_FLOAT_EQ(pcolor.get().b, 0.28f);
}

TEST(Presets, PresetStepMorphing) {

  al::Parameter p{"param", "group", 0.5f, 0.0, 1.0};
  al::ParameterInt pint{"paramint", "group", 3, 1, 10};
  al::ParameterColor pcolor{"paramcolor", "group", al::Color(0.1f, 0.1f, 0.1f)};

  // Disable morph thread
  al::PresetHandler ph{al::TimeMasterMode::TIME_MASTER_FREE};

  ph.setMorphTime(0.3f);
  ph.setMorphStepTime(0.1f); // Should have 3 steps
  ph << p << pint << pcolor;

  p.set(0.1f);
  pint.set(9);
  pcolor.set({0.4f, 0.3f, 0.2f});
  ph.storePreset("2");
  p.set(0.4f);
  pint.set(3);
  pcolor.set({0.55f, 0.33f, 0.5f});
  ph.storePreset("3");

  ph.recallPresetSynchronous("2");
  ph.morphTo("3", 0.3f);
  al::al_sleep(0.2);
  // Ensure no morphing has happened

  EXPECT_FLOAT_EQ(p.get(), 0.1f);
  EXPECT_EQ(pint.get(), 9);
  EXPECT_FLOAT_EQ(pcolor.get().r, 0.4f);
  EXPECT_FLOAT_EQ(pcolor.get().g, 0.3f);
  EXPECT_FLOAT_EQ(pcolor.get().b, 0.2f);

  ph.stepMorphing();

  EXPECT_FLOAT_EQ(p.get(), 0.1f);
  EXPECT_EQ(pint.get(), 9);
  EXPECT_FLOAT_EQ(pcolor.get().r, 0.4f);
  EXPECT_FLOAT_EQ(pcolor.get().g, 0.3f);
  EXPECT_FLOAT_EQ(pcolor.get().b, 0.2f);

  ph.stepMorphing();

  EXPECT_FLOAT_EQ(p.get(), 0.2f);
  EXPECT_EQ(pint.get(), 7);
  EXPECT_FLOAT_EQ(pcolor.get().r, 0.45f);
  EXPECT_FLOAT_EQ(pcolor.get().g, 0.31f);
  EXPECT_FLOAT_EQ(pcolor.get().b, 0.3f);

  ph.stepMorphing();

  EXPECT_FLOAT_EQ(p.get(), 0.3f);
  EXPECT_EQ(pint.get(), 5);
  EXPECT_FLOAT_EQ(pcolor.get().r, 0.5f);
  EXPECT_FLOAT_EQ(pcolor.get().g, 0.32f);
  EXPECT_FLOAT_EQ(pcolor.get().b, 0.4f);
}

TEST(Presets, PresetNoMorphing) {

  al::Parameter p{"param", "group", 0.5f, 0.0, 1.0};
  al::ParameterInt pint{"paramint", "group", 3, 1, 10};
  al::ParameterColor pcolor{"paramcolor", "group", al::Color(0.1f, 0.1f, 0.1f)};

  al::PresetHandler ph{al::TimeMasterMode::TIME_MASTER_FREE};

  ph.setMorphTime(0.0);
  ph.setMorphStepTime(0.1);
  ph << p << pint << pcolor;

  p.set(0.2f);
  pint.set(8);
  pcolor.set({0.3f, 0.2f, 0.4f});
  ph.storePreset("2");

  p.set(0.5f);
  pint.set(4);
  pcolor.set({0.65f, 0.73f, 0.8f});
  ph.storePreset("3");

  ph.recallPreset("2");

  // Should still be on preset 3 values
  EXPECT_FLOAT_EQ(p.get(), 0.5f);
  EXPECT_EQ(pint.get(), 4);
  EXPECT_FLOAT_EQ(pcolor.get().r, 0.65f);
  EXPECT_FLOAT_EQ(pcolor.get().g, 0.73f);
  EXPECT_FLOAT_EQ(pcolor.get().b, 0.8f);

  ph.stepMorphing();

  // Should now be in preset 2
  EXPECT_FLOAT_EQ(p.get(), 0.2f);
  EXPECT_EQ(pint.get(), 8);
  EXPECT_FLOAT_EQ(pcolor.get().r, 0.3f);
  EXPECT_FLOAT_EQ(pcolor.get().g, 0.2f);
  EXPECT_FLOAT_EQ(pcolor.get().b, 0.4f);
}

TEST(Presets, PresetNoMorphingThread) {

  al::Parameter p{"param", "group", 0.5f, 0.0, 1.0};
  al::ParameterInt pint{"paramint", "group", 3, 1, 10};
  al::ParameterColor pcolor{"paramcolor", "group", al::Color(0.1f, 0.1f, 0.1f)};

  al::PresetHandler ph;

  ph.setMorphTime(0.0);
  ph.setMorphStepTime(0.1f);
  ph << p << pint << pcolor;

  p.set(0.2f);
  pint.set(8);
  pcolor.set({0.3f, 0.2f, 0.4f});
  ph.storePreset("2");

  p.set(0.5f);
  pint.set(4);
  pcolor.set({0.65f, 0.73f, 0.8f});
  ph.storePreset("3");

  ph.recallPreset("2");
  std::this_thread::sleep_for(std::chrono::milliseconds(120));

  // Should now be in preset 2
  EXPECT_FLOAT_EQ(p.get(), 0.2f);
  EXPECT_EQ(pint.get(), 8);
  EXPECT_FLOAT_EQ(pcolor.get().r, 0.3f);
  EXPECT_FLOAT_EQ(pcolor.get().g, 0.2f);
  EXPECT_FLOAT_EQ(pcolor.get().b, 0.4f);
}

TEST(Presets, PresetMorphingThread) {
  al::Parameter p{"param", "group", 0.5f, 0.0, 1.0};
  al::ParameterInt pint{"paramint", "group", 3, 1, 10};
  al::ParameterColor pcolor{"paramcolor", "group", al::Color(0.1f, 0.1f, 0.1f)};

  al::PresetHandler ph;

  ph.setMorphTime(0.3f);
  ph.setMorphStepTime(0.1f);
  ph << p << pint << pcolor;

  p.set(0.2f);
  pint.set(8);
  pcolor.set({0.3f, 0.2f, 0.4f});
  ph.storePreset("2");

  p.set(0.5f);
  pint.set(4);
  pcolor.set({0.65f, 0.73f, 0.8f});
  ph.storePreset("3");

  ph.recallPreset("2");
  std::this_thread::sleep_for(std::chrono::milliseconds(150));

  // Should not be in preset 2 yet
  EXPECT_NE(p.get(), 0.2f);
  EXPECT_NE(pint.get(), 8);
  EXPECT_NE(pcolor.get().r, 0.3f);
  EXPECT_NE(pcolor.get().g, 0.2f);
  EXPECT_NE(pcolor.get().b, 0.4f);

  std::this_thread::sleep_for(std::chrono::milliseconds(300));

  // Should now be in preset 2
  EXPECT_FLOAT_EQ(p.get(), 0.2f);
  EXPECT_EQ(pint.get(), 8);
  EXPECT_FLOAT_EQ(pcolor.get().r, 0.3f);
  EXPECT_FLOAT_EQ(pcolor.get().g, 0.2f);
  EXPECT_FLOAT_EQ(pcolor.get().b, 0.4f);
}

TEST(Presets, PresetDifferingParams) {
  al::Parameter p{"param", "group", 0.5f, 0.0, 1.0};
  al::ParameterInt pint{"paramint", "group", 3, 1, 10};
  al::ParameterColor pcolor{"paramcolor", "group", al::Color(0.1f, 0.1f, 0.1f)};

  al::PresetHandler ph;

  ph << p << pint << pcolor;

  // This test also excersizes skipParameter()
  ph.skipParameter("/group/paramint");
  p.set(0.2f);
  pint.set(4);
  pcolor.set({0.3f, 0.2f, 0.4f});
  ph.storePreset("2");

  pint.set(5);
  ph.recallPresetSynchronous("2");
  EXPECT_EQ(pint.get(), 5);

  ph.skipParameter("/group/paramint", false);
  ph.skipParameter("/group/param");
  p.set(0.5f);
  pint.set(4);
  pcolor.set({0.65f, 0.73f, 0.8f});
  ph.storePreset("3");

  p.set(0.2f);
  ph.recallPresetSynchronous("3");
  EXPECT_FLOAT_EQ(p.get(), 0.2f);

  ph.setMorphTime(2);
  ph.setMorphStepTime(0.05f);
  ph.recallPreset("2");

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  ph.setMorphTime(0.1f);
  ph.recallPreset("3");
  std::this_thread::sleep_for(std::chrono::milliseconds(200));
  EXPECT_NE(p.get(), 0.5f);
  EXPECT_EQ(pint.get(), 4);
  EXPECT_FLOAT_EQ(pcolor.get().r, 0.65f);
  EXPECT_FLOAT_EQ(pcolor.get().g, 0.73f);
  EXPECT_FLOAT_EQ(pcolor.get().b, 0.8f);
}
