#include "gtest/gtest.h"

#include "al/graphics/al_Shapes.hpp"
#include "al/scene/al_Scene.hpp"

using namespace al;

struct MarkerState {
  float value{0.f};
};

struct Marker : StatefulComponent<MarkerState> {
  Parameter amount{"amount", "", 1.f};
  void init() override { entity->registerParameter(amount); }
  void update(double) override { state.value = amount.get(); }
};

struct CountDraw : Component {
  int draws{0};
  void onDraw(Graphics &) override { ++draws; }
};

TEST(EcsScene, ArchetypeComposeAndSwap) {
  Scene scene("test", TimeMasterMode::TIME_MASTER_UPDATE);
  scene.registerEntity("Mark", [](Entity &e) {
    e.add<Marker>();
    e.add<CountDraw>();
  });
  scene.allocatePool("Mark", 4);

  Entity *e = scene.getEntity("Mark");
  ASSERT_NE(e, nullptr);
  e->get<Marker>()->amount.set(3.f);
  int id = scene.triggerOn(e, 42);
  EXPECT_EQ(id, 42);

  scene.update(0.016);
  EXPECT_FLOAT_EQ(e->get<Marker>()->state.value, 3.f);

  // Swap appearance: remove CountDraw, still have Marker
  EXPECT_TRUE(e->remove<CountDraw>());
  EXPECT_EQ(e->get<CountDraw>(), nullptr);
  EXPECT_NE(e->get<Marker>(), nullptr);

  char buf[512];
  size_t n = scene.packState(buf, sizeof(buf));
  EXPECT_GT(n, sizeof(uint32_t));

  Scene replica("test", TimeMasterMode::TIME_MASTER_UPDATE);
  replica.setReplica(true);
  replica.registerEntity("Mark", [](Entity &ent) {
    ent.add<Marker>();
    ent.add<CountDraw>();
  });
  replica.unpackState(buf, n);
  Entity *r = replica.findEntityById(42);
  ASSERT_NE(r, nullptr);
  EXPECT_TRUE(r->isReplica());
  EXPECT_FLOAT_EQ(r->get<Marker>()->state.value, 3.f);
}

TEST(EcsScene, ParamStateFlatten) {
  Scene scene("s");
  scene.registerEntity("P", [](Entity &e) { e.add<Marker>(); });
  Entity *e = scene.getEntity("P");
  e->get<Marker>()->amount.set(7.f);
  // Register happens in init via trigger path — call doInit via allocate
  // amount already registered in allocateEntity -> doInit
  ParamState snap = e->captureParamState();
  EXPECT_FALSE(snap.empty());
  e->get<Marker>()->amount.set(0.f);
  e->applyParamState(snap);
  EXPECT_FLOAT_EQ(e->get<Marker>()->amount.get(), 7.f);
}
