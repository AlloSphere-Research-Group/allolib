#include "gtest/gtest.h"

#include "al/parameter/al_Entity.hpp"
#include "al/parameter/al_ParamSet.hpp"
#include "al/parameter/al_ParamState.hpp"
#include "al/parameter/al_Signal.hpp"

using namespace al;

TEST(ParamState, CaptureApplyLerp) {
  SignalFloat a{"x", "g", 0.f};
  SignalFloat b{"y", "g", 10.f};
  std::vector<ParameterMeta *> params{&a, &b};

  a.set(0.f);
  b.set(10.f);
  ParamState start = captureParamState(params);
  a.set(10.f);
  b.set(0.f);
  ParamState end = captureParamState(params);

  ParamState mid = lerpParamState(start, end, 0.5);
  applyParamState(mid, params);
  EXPECT_FLOAT_EQ(a.get(), 5.f);
  EXPECT_FLOAT_EQ(b.get(), 5.f);
}

TEST(ParamSet, EntityPathBinding) {
  SignalFloat radius{"radius", "", 1.f};
  ParamSet set("collage/e7");
  set << radius;
  EXPECT_EQ(radius.getFullAddress(), "/collage/e7/radius");

  radius.set(3.f);
  ParamState snap = set.capture();
  radius.set(0.f);
  set.apply(snap);
  EXPECT_FLOAT_EQ(radius.get(), 3.f);
}

TEST(Entity, ParamsAndPose) {
  Entity e{42, "sprite"};
  EXPECT_EQ(e.path(), "sprite");

  SignalFloat alpha{"alpha", "", 1.f};
  e.addParam(alpha);
  EXPECT_EQ(alpha.getFullAddress(), "/sprite/alpha");

  e.pose().pos() = Vec3d(1, 2, 3);
  EXPECT_DOUBLE_EQ(e.pose().x(), 1.0);

  e.setPath("world/e42");
  EXPECT_EQ(alpha.getFullAddress(), "/world/e42/alpha");
}

TEST(Signal, AliasesAreParameterTypes) {
  SignalFloat f{"f", "", 0.5f};
  SignalInt i{"i", "", 3};
  SignalVec3 v{"v", "", Vec3f(1, 0, 0)};
  EXPECT_FLOAT_EQ(f.get(), 0.5f);
  EXPECT_EQ(i.get(), 3);
  EXPECT_FLOAT_EQ(v.get().x, 1.f);
}
