#include "gtest/gtest.h"

#include "al/protocol/al_OSC.hpp"
#include "al/ui/al_ParamCodec.hpp"
#include "al/ui/al_Parameter.hpp"
#include "al/ui/al_ParameterServer.hpp"

using namespace al;

TEST(ParamConstraint, NoClampByDefault) {
  Parameter p{"gain", "", 0.5f, 0.0f, 1.0f};
  p.set(2.5f);
  EXPECT_FLOAT_EQ(p.get(), 2.5f);
}

TEST(ParamConstraint, UseMinMaxConstraint) {
  Parameter p{"gain", "", 0.5f, 0.0f, 1.0f};
  p.useMinMaxConstraint();
  p.set(2.5f);
  EXPECT_FLOAT_EQ(p.get(), 1.0f);
  p.set(-1.0f);
  EXPECT_FLOAT_EQ(p.get(), 0.0f);
}

TEST(ParamConstraint, NumericIntConstraint) {
  ParameterInt n{"count", "", 0, 0, 10};
  n.set(99);
  EXPECT_EQ(n.get(), 99);
  n.useMinMaxConstraint();
  n.set(99);
  EXPECT_EQ(n.get(), 10);
}

TEST(ParamCodec, RegistryHasBuiltins) {
  EXPECT_NE(paramCodecs().find<Parameter>(), nullptr);
  EXPECT_NE(paramCodecs().find<ParameterInt>(), nullptr);
  EXPECT_NE(paramCodecs().find<ParameterBool>(), nullptr);
  EXPECT_NE(paramCodecs().find<ParameterString>(), nullptr);
  EXPECT_NE(paramCodecs().find<ParameterPose>(), nullptr);
  EXPECT_NE(paramCodecs().find<Trigger>(), nullptr);
}

TEST(ParamCodec, AttachNotifyRoundtrip) {
  Parameter p{"freq", "synth", 440.f, 20.f, 20000.f};
  float notified = -1.f;
  int callbacks = 0;

  const ParamCodec *codec = paramCodecs().find(p);
  ASSERT_NE(codec, nullptr);
  ASSERT_TRUE(static_cast<bool>(codec->attach));

  // Attach wires a change callback; value is also passed to typed callbacks.
  // (set stores after callbacks, so get() inside notify still sees the old value.)
  p.registerChangeCallback([&](float v, ValueSource * /*src*/) {
    ++callbacks;
    notified = v;
  });

  p.set(880.f);
  EXPECT_EQ(callbacks, 1);
  EXPECT_FLOAT_EQ(notified, 880.f);
  EXPECT_FLOAT_EQ(p.get(), 880.f);
}

TEST(ParamCodec, MetaSetViaFields) {
  Parameter a{"a", "", 0.25f};
  Parameter b{"b", "", 0.0f};
  b.set(&a);
  EXPECT_FLOAT_EQ(b.get(), 0.25f);

  ParameterInt ia{"ia", "", 7};
  ParameterInt ib{"ib", "", 0};
  ib.set(&ia);
  EXPECT_EQ(ib.get(), 7);
}

namespace {

osc::Message makeOscMessage(const std::string &address, float value) {
  osc::Packet packet;
  packet.addMessage(address, value);
  return osc::Message(packet.data(), static_cast<int>(packet.size()));
}

osc::Message makeOscMessage(const std::string &address, int value) {
  osc::Packet packet;
  packet.addMessage(address, value);
  return osc::Message(packet.data(), static_cast<int>(packet.size()));
}

osc::Message makeOscMessage(const std::string &address,
                            const std::string &value) {
  osc::Packet packet;
  packet.addMessage(address, value);
  return osc::Message(packet.data(), static_cast<int>(packet.size()));
}

} // namespace

TEST(ParamCodec, FromOscFloat) {
  Parameter p{"freq", "synth", 440.f};
  const ParamCodec *codec = paramCodecs().find(p);
  ASSERT_NE(codec, nullptr);

  osc::Message m = makeOscMessage(p.getFullAddress(), 880.f);
  EXPECT_TRUE(codec->fromOsc(&p, m.addressPattern(), m, nullptr));
  EXPECT_FLOAT_EQ(p.get(), 880.f);
}

TEST(ParamCodec, FromOscInt) {
  ParameterInt p{"count", "", 0};
  const ParamCodec *codec = paramCodecs().find(p);
  ASSERT_NE(codec, nullptr);

  osc::Message m = makeOscMessage(p.getFullAddress(), 42);
  EXPECT_TRUE(codec->fromOsc(&p, m.addressPattern(), m, nullptr));
  EXPECT_EQ(p.get(), 42);
}

TEST(ParamCodec, FromOscString) {
  ParameterString p{"name", "", "x"};
  const ParamCodec *codec = paramCodecs().find(p);
  ASSERT_NE(codec, nullptr);

  osc::Message m = makeOscMessage(p.getFullAddress(), std::string("hello"));
  EXPECT_TRUE(codec->fromOsc(&p, m.addressPattern(), m, nullptr));
  EXPECT_EQ(p.get(), "hello");
}

TEST(ParamCodec, FromOscWrongAddress) {
  Parameter p{"freq", "", 1.f};
  const ParamCodec *codec = paramCodecs().find(p);
  ASSERT_NE(codec, nullptr);

  osc::Message m = makeOscMessage("/other", 2.f);
  EXPECT_FALSE(codec->fromOsc(&p, m.addressPattern(), m, nullptr));
  EXPECT_FLOAT_EQ(p.get(), 1.f);
}

TEST(ParamPath, HierarchicalGroupForEntity) {
  Parameter radius{"radius", "collage/e42", 1.f};
  EXPECT_EQ(radius.getFullAddress(), "/collage/e42/radius");

  radius.setGroup("collage/e99");
  EXPECT_EQ(radius.getFullAddress(), "/collage/e99/radius");
  EXPECT_EQ(radius.getGroup(), "collage/e99");

  radius.setPath("opacity", "scene/actor1");
  EXPECT_EQ(radius.getName(), "opacity");
  EXPECT_EQ(radius.getFullAddress(), "/scene/actor1/opacity");
}

TEST(ParamPath, PathChangeCallback) {
  Parameter p{"x", "a", 0.f};
  std::string oldA, newA;
  int calls = 0;
  p.registerPathChangeCallback(
      [&](ParameterMeta *, const std::string &o, const std::string &n) {
        ++calls;
        oldA = o;
        newA = n;
      });
  p.setGroup("b");
  EXPECT_EQ(calls, 1);
  EXPECT_EQ(oldA, "/a/x");
  EXPECT_EQ(newA, "/b/x");
}

TEST(ParamCallbacks, GetSeesNewValue) {
  Parameter p{"x", "", 0.f};
  float seen = -1.f;
  p.registerChangeCallback([&](float /*v*/) { seen = p.get(); });
  p.set(3.5f);
  EXPECT_FLOAT_EQ(seen, 3.5f);
  EXPECT_FLOAT_EQ(p.getPrevious(), 0.f);
}
