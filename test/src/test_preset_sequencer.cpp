
#include "gtest/gtest.h"

#include "al/ui/al_PresetHandler.hpp"
#include "al/ui/al_PresetSequencer.hpp"

#include <fstream>

TEST(Presets, ParameterValuesSeqMorph) {
  al::Parameter p{"param", "group", 0.5f, 0.0, 1.0};
  al::ParameterInt pint{"paramint", "group", 3, 1, 10};
  al::ParameterColor pcolor{"paramcolor", "group", al::Color(0.1f, 0.1f, 0.1f)};
  al::ParameterPose pose{"parampose", "group"};

  pose.set(al::Pose({1, 1, 1}));
  al::PresetHandler ph{al::TimeMasterMode::TIME_MASTER_FREE};
  ph << p << pint << pcolor;
  ph << pose;

  std::string seqFile = R"(
+0.0:/group/param:0.6:0.0
+0.15:/group/param:0.7:0.3
+0.45:/group/paramint:6:0.3
+0.45:/group/paramcolor:0.3,0.4,0.5:0.3
+0.45:/group/parampose:0.3,0.4,0.5:0.3
)";
  const std::string seqFileName = "test.sequence";
  std::ofstream f(ph.getRootPath() + seqFileName);
  assert(f.good());

  f << seqFile;
  f.close();

  al::PresetSequencer seq{al::TimeMasterMode::TIME_MASTER_FREE};
  // To use parameters in sequencer, they must be registered explicitly
  seq << p << pint << pcolor;
  seq << pose;
  seq << ph;

  seq.playSequence(seqFileName);
  ph.setMorphStepTime(0.1f);

  EXPECT_FLOAT_EQ(p.get(), 0.6f);

  seq.stepSequencer(0.1f);
  ph.stepMorphing();
  EXPECT_FLOAT_EQ(p.get(), 0.6f);
  seq.stepSequencer(0.1f); // morph starts
  ph.stepMorphing();
  EXPECT_FLOAT_EQ(p.get(), 0.6f);
  seq.stepSequencer(0.1f);
  ph.stepMorphing();
  EXPECT_FLOAT_EQ(p.get(), 0.63333333f);
  seq.stepSequencer(0.1f);
  ph.stepMorphing();
  EXPECT_FLOAT_EQ(p.get(), 0.666666666f);
  seq.stepSequencer(0.1f);
  ph.stepMorphing();
  EXPECT_EQ(pint.get(), 3);
  EXPECT_FLOAT_EQ(p.get(), 0.7f);
  seq.stepSequencer(0.1f);
  ph.stepMorphing();
  EXPECT_EQ(pint.get(), 3);
  EXPECT_FLOAT_EQ(p.get(), 0.7f);
  seq.stepSequencer(0.1f);
  ph.stepMorphing();
  EXPECT_EQ(pint.get(), 4);
  seq.stepSequencer(0.1f);
  ph.stepMorphing();
  EXPECT_EQ(pint.get(), 5);
  seq.stepSequencer(0.1f);
  ph.stepMorphing();
  EXPECT_EQ(pint.get(), 6);
}

TEST(Presets, SeqPresets) {
  al::Parameter p{"param", "group", 0.5f, 0.0, 1.0};
  al::ParameterInt pint{"paramint", "group", 3, 1, 10};
  al::ParameterColor pcolor{"paramcolor", "group", al::Color(0.1f, 0.1f, 0.1f)};
  al::ParameterPose pose{"parampose", "group"};

  pose.set(al::Pose({1, 1, 1}));
  al::PresetHandler ph /*{al::TimeMasterMode::TIME_MASTER_FREE}*/;
  ph << p << pint << pcolor;
  ph << pose;

  p.set(0.2f);
  ph.storePreset("p1");

  p.set(0.4f);
  ph.storePreset("p2");

  p.set(0.5f);
  std::string seqFile = R"(
p1:0:0.15
p2:0:0.15
::
)";

  const std::string seqFileName = "test_preset.sequence";
  std::ofstream f(ph.getRootPath() + seqFileName);
  assert(f.good());

  f << seqFile;
  f.close();

  al::PresetSequencer seq{al::TimeMasterMode::TIME_MASTER_FREE};
  seq << ph;

  EXPECT_FLOAT_EQ(p.get(), 0.5f);
  seq.playSequence(seqFileName);
  al::al_sleep(0.05);

  EXPECT_FLOAT_EQ(p.get(), 0.2f);
  seq.stepSequencer(0.2);
  al::al_sleep(0.05);
  EXPECT_FLOAT_EQ(p.get(), 0.4f);
}

TEST(Presets, ParameterValuesSeq) {
  al::Parameter p{"param", "group", 0.5f, 0.0, 1.0};
  al::ParameterInt pint{"paramint", "group", 3, 1, 10};
  al::ParameterColor pcolor{"paramcolor", "group", al::Color(0.1f, 0.1f, 0.1f)};
  al::ParameterPose pose{"parampose", "group"};

  pose.set(al::Pose({1, 1, 1}));
  al::PresetHandler ph /*{al::TimeMasterMode::TIME_MASTER_FREE}*/;
  ph << p << pint << pcolor;
  ph << pose;

  std::string seqFile = R"(
+0.0:/group/param:0.6:0.0
+0.15:/group/param:0.7:0.0
+0.15:/group/paramint:5:0.0
+0.15:/group/paramcolor:0.3,0.4,0.5:0.0
+0.15:/group/parampose:0.3,0.4,0.5:0.0
)";
  const std::string seqFileName = "test.sequence";
  std::ofstream f(ph.getRootPath() + seqFileName);
  assert(f.good());

  f << seqFile;
  f.close();

  al::PresetSequencer seq{al::TimeMasterMode::TIME_MASTER_FREE};
  // To use parameters in sequencer, they must be registered explicitly
  seq << p << pint << pcolor;
  seq << pose;
  seq << ph;

  seq.playSequence(seqFileName);

  EXPECT_FLOAT_EQ(p.get(), 0.6f);
  seq.stepSequencer(0.2);
  al::al_sleep(0.05);
  EXPECT_FLOAT_EQ(p.get(), 0.7f);
  seq.stepSequencer(0.2);
  al::al_sleep(0.05);
  EXPECT_EQ(pint.get(), 5);
  EXPECT_FLOAT_EQ(pcolor.get().r, 0.1);
  EXPECT_FLOAT_EQ(pcolor.get().g, 0.1);
  EXPECT_FLOAT_EQ(pcolor.get().b, 0.1);
  seq.stepSequencer(0.2);
  al::al_sleep(0.05);
  EXPECT_FLOAT_EQ(pcolor.get().r, 0.3);
  EXPECT_FLOAT_EQ(pcolor.get().g, 0.4);
  EXPECT_FLOAT_EQ(pcolor.get().b, 0.5);
  EXPECT_FLOAT_EQ(pose.get().x(), 1);
  EXPECT_FLOAT_EQ(pose.get().y(), 1);
  EXPECT_FLOAT_EQ(pose.get().z(), 1);
  seq.stepSequencer(0.2);
  al::al_sleep(0.05);
  EXPECT_FLOAT_EQ(pose.get().x(), 0.3);
  EXPECT_FLOAT_EQ(pose.get().y(), 0.4);
  EXPECT_FLOAT_EQ(pose.get().z(), 0.5);
}
