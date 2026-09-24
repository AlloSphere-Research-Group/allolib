#include "gtest/gtest.h"

#include "al/scene/al_PickableComponent.hpp"
#include "al/scene/al_Scene.hpp"
#include "al/ui/al_ScenePick.hpp"

using namespace al;

TEST(ScenePick, SyncCreatesBoxForPickableComponent) {
  Scene scene("s", TimeMasterMode::TIME_MASTER_UPDATE);
  scene.registerEntity("P", [](Entity &e) {
    e.add<PickableComponent>();
    e.size(2.f);
    e.get<PickableComponent>()->aspect = 1.5f;
  });
  Entity *e = scene.getEntity("P");
  ASSERT_NE(e, nullptr);
  scene.triggerOn(e, 7);
  scene.update(0);

  ScenePickContext ctx;
  ctx.init();
  scene_pick::sync(scene, {7}, ctx, 7);

  ASSERT_EQ(ctx.boxes.count(7), 1u);
  EXPECT_TRUE(ctx.boxes[7]->selected.get());
  const Vec3f sc = ctx.boxes[7]->scaleVec.get();
  EXPECT_NEAR(sc.x, 3.f, 1e-3); // size 2 * aspect 1.5
  EXPECT_NEAR(sc.y, 2.f, 1e-3);
}
