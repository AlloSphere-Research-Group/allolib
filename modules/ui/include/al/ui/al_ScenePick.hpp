#ifndef AL_SCENE_PICK_HPP
#define AL_SCENE_PICK_HPP

/**
 * @file al_ScenePick.hpp
 * @brief Component-oriented pick system for al::Scene.
 *
 * Successor path for ECS apps. Syncs PickableComponent entities to PickableBB
 * boxes (legacy hit-test primitives). Old Pickable / PickableManager APIs
 * remain available for non-ECS code — do not remove yet.
 *
 * Collision / drag constraints are app-level for now (override later).
 */

#include <algorithm>
#include <cmath>
#include <memory>
#include <unordered_map>
#include <vector>

#include "al/graphics/al_Graphics.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/math/al_Matrix4.hpp"
#include "al/math/al_Ray.hpp"
#include "al/scene/al_PickableComponent.hpp"
#include "al/scene/al_Scene.hpp"
#include "al/ui/al_Pickable.hpp"
#include "al/ui/al_PickableManager.hpp"

namespace al {

struct ScenePickContext {
  Mesh wireBox;
  PickableManager manager;
  std::unordered_map<int, std::unique_ptr<PickableBB>> boxes;
  int draggingId{-1};
  Hit lastHit;

  void init() {
    addWireBox(wireBox, 1.f);
    wireBox.primitive(Mesh::LINES);
  }

  void clear() {
    boxes.clear();
    manager.clear();
    draggingId = -1;
  }
};

namespace scene_pick {

inline void extentsFor(Entity &e, PickableComponent &pc, float &widthM,
                       float &heightM) {
  if (pc.worldWidth > 1e-4f && pc.worldHeight > 1e-4f) {
    widthM = pc.worldWidth;
    heightM = pc.worldHeight;
    return;
  }
  const float s = e.size();
  widthM = s * pc.aspect;
  heightM = s;
}

inline void syncBoxToEntity(PickableBB &bb, Entity &e, PickableComponent &pc,
                            int selectedId) {
  const Vec3f pos = e.pose().pos();
  Quatf orient = e.pose().quat();
  bb.pose.set(Pose(pos, orient));

  float widthM = 1.f;
  float heightM = 1.f;
  extentsFor(e, pc, widthM, heightM);
  const float depth =
      std::max(0.06f, std::min(widthM, heightM) * pc.depthFrac);
  bb.scaleVec.set(Vec3f(widthM, heightM, depth));

  const bool sel = (e.id() == selectedId) || pc.selected;
  bb.selected.set(sel);
  pc.selected = sel;
  pc.hover = bb.hover.get();
}

/// Sync boxes for active entity ids that have PickableComponent.
inline void sync(Scene &scene, const std::vector<int> &activeIds,
                 ScenePickContext &ctx, int selectedId = -1,
                 int skipPoseEntityId = -1) {
  for (int id : activeIds) {
    Entity *e = scene.findEntityById(id);
    if (!e) {
      continue;
    }
    auto *pc = e->get<PickableComponent>();
    if (!pc || !pc->enabled) {
      continue;
    }
    if (!ctx.boxes.count(id)) {
      auto bb = std::make_unique<PickableBB>();
      bb->set(ctx.wireBox);
      bb->name = "e" + std::to_string(id);
      bb->scale.set(1.f);
      ctx.boxes[id] = std::move(bb);
    }
    if (id != skipPoseEntityId) {
      syncBoxToEntity(*ctx.boxes[id], *e, *pc, selectedId);
    } else {
      ctx.boxes[id]->selected.set(id == selectedId || pc->selected);
      pc->selected = (id == selectedId);
      pc->hover = ctx.boxes[id]->hover.get();
    }
  }

  for (auto it = ctx.boxes.begin(); it != ctx.boxes.end();) {
    if (std::find(activeIds.begin(), activeIds.end(), it->first) ==
        activeIds.end()) {
      it = ctx.boxes.erase(it);
    } else {
      ++it;
    }
  }

  ctx.manager.clear();
  for (auto &pair : ctx.boxes) {
    ctx.manager << pair.second.get();
  }
}

inline void draw(Graphics &g, ScenePickContext &ctx) {
  g.lighting(false);
  g.depthMask(false);
  for (auto &pair : ctx.boxes) {
    PickableBB &bb = *pair.second;
    if (bb.selected.get()) {
      g.color(0.2f, 0.95f, 1.f, 0.55f);
    } else if (bb.hover.get()) {
      g.color(1.f, 0.85f, 0.2f, 0.4f);
    } else {
      g.color(0.55f, 0.55f, 0.65f, 0.22f);
    }
    g.polygonLine();
    bb.pushMatrix(g);
    g.draw(ctx.wireBox);
    bb.popMatrix(g);
  }
  g.polygonFill();
  g.depthMask(true);
}

inline int mouseToFbX(int x, int fbW, int winW) {
  if (winW <= 0) {
    return x;
  }
  return static_cast<int>(
      std::lround(x * (static_cast<double>(fbW) / static_cast<double>(winW))));
}

inline int mouseToFbY(int y, int fbH, int winH) {
  if (winH <= 0) {
    return y;
  }
  return static_cast<int>(
      std::lround(y * (static_cast<double>(fbH) / static_cast<double>(winH))));
}

inline Rayd pickRay(Graphics &g, int mx, int my, int w, int h) {
  Rayd r;
  Vec3d screenPos;
  screenPos.x = (mx * 1. / w) * 2. - 1.;
  screenPos.y = ((h - my) * 1. / h) * 2. - 1.;
  screenPos.z = -1.;
  auto unproject = [&](double px, double py, double pz) {
    Matrix4f mvp = g.projMatrix() * g.viewMatrix();
    Matrix4f inv = Matrix4f::inverse(mvp);
    Vec4f worldPos4 = inv.transform(Vec4f(px, py, pz, 1.f));
    Vec3d out(worldPos4.x, worldPos4.y, worldPos4.z);
    return out / static_cast<double>(worldPos4.w);
  };
  Vec3d origin = unproject(screenPos.x, screenPos.y, screenPos.z);
  r.origin().set(origin);
  screenPos.z = 1.;
  Vec3d farPt = unproject(screenPos.x, screenPos.y, screenPos.z);
  r.direction().set(farPt);
  r.direction() -= r.origin();
  r.direction().normalize();
  return r;
}

inline void dispatch(ScenePickContext &ctx, PickEvent e) {
  ctx.lastHit = ctx.manager.intersect(e.ray);
  for (Pickable *p : ctx.manager.pickables()) {
    if (p == ctx.lastHit.p || p->selected.get() || e.type == Unpick ||
        e.type == Point) {
      p->event(e);
    }
  }
}

/// Map last hit PickableBB → entity id (-1 if none).
inline int entityIdFromHit(const ScenePickContext &ctx) {
  if (!ctx.lastHit.hit || !ctx.lastHit.p) {
    return -1;
  }
  for (const auto &pair : ctx.boxes) {
    if (pair.second.get() == ctx.lastHit.p) {
      return pair.first;
    }
  }
  return -1;
}

} // namespace scene_pick
} // namespace al

#endif
