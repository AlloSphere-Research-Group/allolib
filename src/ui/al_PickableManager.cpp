#include "al/ui/al_PickableManager.hpp"

using namespace al;

PickableManager &PickableManager::registerPickable(Pickable &p) {
  mPickables.push_back(&p);
  return *this;
}
Hit PickableManager::intersect(Rayd r) {
  Hit hmin = Hit(false, r, 1e10, NULL);
  for (Pickable *p : mPickables) {
    Hit h = p->intersect(r);
    if (h.hit && h.t < hmin.t) {
      hmin = h;
    }
  }
  return hmin;
}

void PickableManager::event(PickEvent e) {
  Hit h = intersect(e.ray);
  for (Pickable *p : mPickables) {
    if (p == h.p || p->selected.get() || e.type == Unpick || e.type == Point) {
      p->event(e);

      if (e.type == Point)
        mLastPoint = h;
      if (e.type == Pick)
        mLastPick = h;
    } // else p->clearSelection(); //unpick?
  }
}

void PickableManager::unhighlightAll() {
  for (Pickable *p : mPickables) {
    if (p->hover.get())
      p->hover = false;
  }
}

void PickableManager::onMouseMove(Graphics &g, const Mouse &m, int w, int h) {
  Rayd r = getPickRay(g, m.x(), m.y(), w, h);
  event(PickEvent(Point, r));
}

void PickableManager::onMouseDown(Graphics &g, const Mouse &m, int w, int h) {
  x = m.x();
  y = m.y();
  Rayd r = getPickRay(g, m.x(), m.y(), w, h);
  event(PickEvent(Pick, r));
}

void PickableManager::onMouseDrag(Graphics &g, const Mouse &m, int w, int h) {
  int dx = m.x() - x;
  int dy = m.y() - y;
  Rayd r = getPickRay(g, m.x(), m.y(), w, h);
  if (ctrlKey && m.left()) {
    event(PickEvent(Scale, r, -m.dy()));
  } else if (m.right()) {
    event(PickEvent(RotateTurntable, r, Pose(Vec3f(), g.viewMatrix().toQuat()),
                    Vec3f(dx, dy, 0)));
  } else if (m.middle()) {

    event(PickEvent(Scale, r, -m.dy()));
  } else {
    event(PickEvent(TranslateRay, r));
  }
}

void PickableManager::onMouseUp(Graphics &g, const Mouse &m, int w, int h) {
  Rayd r = getPickRay(g, m.x(), m.y(), w, h);
  event(PickEvent(Unpick, r));
}

void PickableManager::onKeyDown(const Keyboard &k) {
  switch (k.key()) {
  default:
    break;
  }
  ctrlKey = k.ctrl();
}

void PickableManager::onKeyUp(const Keyboard &k) {
  switch (k.key()) {
  default:
    break;
  }
  if (k.key() == 0) {
    // Assumes 0 means a control key...
    // There seems to be no better way...
    ctrlKey = false;
  }
}
