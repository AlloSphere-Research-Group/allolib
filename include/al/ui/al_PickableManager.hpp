
#ifndef __PICKABLEMANAGER_HPP__
#define __PICKABLEMANAGER_HPP__

#include <vector>
// #include <map>

#include "al/graphics/al_Graphics.hpp"
#include "al/io/al_Window.hpp"
#include "al/math/al_Ray.hpp"
#include "al/ui/al_Pickable.hpp"

namespace al {

/// PickableManager
/// @ingroup UI
class PickableManager {
public:
  PickableManager() {}

  PickableManager &registerPickable(Pickable &p);
  PickableManager &operator<<(Pickable &p) { return registerPickable(p); }
  PickableManager &operator<<(Pickable *p) { return registerPickable(*p); }

  std::vector<Pickable *> pickables() { return mPickables; }

  Hit intersect(Rayd r);

  void event(PickEvent e);

  void unhighlightAll();

  void onMouseMove(Graphics &g, const Mouse &m, int w, int h);
  void onMouseDown(Graphics &g, const Mouse &m, int w, int h);
  void onMouseDrag(Graphics &g, const Mouse &m, int w, int h);
  void onMouseUp(Graphics &g, const Mouse &m, int w, int h);

  void onKeyDown(const Keyboard &k);
  void onKeyUp(const Keyboard &k);

  Hit lastPoint() { return mLastPoint; }
  Hit lastPick() { return mLastPick; }

protected:
  std::vector<Pickable *> mPickables;
  // std::map<int, Hit> mHover;
  // std::map<int, Hit> mSelect;

  Hit mLastPoint;
  Hit mLastPick;
  Vec3d selectOffset;
  int x, y;

  bool ctrlKey{false}; // true if control key

  Vec3d unproject(Graphics &g, Vec3d screenPos, bool view = true) {
    auto v = Matrix4d::identity();
    if (view)
      v = g.viewMatrix();
    auto mvp = g.projMatrix() * v * g.modelMatrix();
    Matrix4d invprojview = Matrix4d::inverse(mvp);
    Vec4d worldPos4 = invprojview.transform(screenPos);
    return worldPos4.sub<3>(0) / worldPos4.w;
  }

  Rayd getPickRay(Graphics &g, int screenX, int screenY, int width, int height,
                  bool view = true) {
    Rayd r;
    Vec3d screenPos;
    screenPos.x = (screenX * 1. / width) * 2. - 1.;
    screenPos.y = ((height - screenY) * 1. / height) * 2. - 1.;
    screenPos.z = -1.;
    Vec3d worldPos = unproject(g, screenPos, view);
    r.origin().set(worldPos);

    screenPos.z = 1.;
    worldPos = unproject(g, screenPos, view);
    r.direction().set(worldPos);
    r.direction() -= r.origin();
    r.direction().normalize();
    return r;
  }
};

} // namespace al

#endif
