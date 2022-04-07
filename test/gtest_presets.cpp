
#include "gtest/gtest.h"

#include "al/ui/al_PresetHandler.hpp"

TEST(Presets, BasicRecall) {

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

  al::PresetHandler ph;

  ph.stopCpuThread(); // to test step morphing function
  ph.setMorphTime(0.3);
  ph.setMorphStepTime(0.1); // Should have 3 steps
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
  ph.morphTo("3", 0.3);
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
