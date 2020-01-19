#include "al/io/al_ControlNav.hpp"
// #include <iostream>

namespace al {

Pose defaultHome() {
  Pose p;
  p.quat().identity();
  p.vec().set(0);
  return p;
}

Nav::Nav(const Vec3d& pos, double smooth)
    : Pose(pos),
      mSmooth(smooth),
      mVelScale(1),
      mPullBack0(0),
      mPullBack1(0),
      mHome(defaultHome()) {
  updateDirectionVectors();
}

Nav::Nav(const Nav& nav)
    : Pose(nav.pos(), nav.quat()),
      mMove0(nav.mMove0),
      mMove1(nav.mMove1),  // linear velocities (raw, smoothed)
      mSpin0(nav.mSpin0),
      mSpin1(nav.mSpin1),  // angular velocities (raw, smoothed)
      mTurn(nav.mTurn),
      mNudge(nav.mNudge),  //
      mSmooth(nav.smooth()),
      mVelScale(nav.mVelScale),
      mPullBack0(nav.mPullBack0),
      mPullBack1(nav.mPullBack1),
      mHome(defaultHome()) {
  updateDirectionVectors();
}

void Nav::faceToward(const Vec3d& point, double amt) {
  /*Vec3d target(p - pos());
  target.normalize();
  Quatd rot = Quatd::getRotationTo(uf(), target);

  // We must pre-multiply the Pose quaternion with our rotation since
  // it was computed in world space.
  if(amt == 1.)  quat() = rot * quat();
  else      quat() = rot.pow(amt) * quat();*/

  Pose::faceToward(point, amt);
  updateDirectionVectors();
}

void Nav::faceToward(const Vec3d& point, const Vec3d& up, double amt) {
  Pose::faceToward(point, up, amt);
  updateDirectionVectors();
}

void Nav::nudgeToward(const Vec3d& p, double amt) {
  Vec3d rotEuler;
  Vec3d target(p - pos());
  target.normalize();  // unit vector of direction to move (in world frame)
  // rotate target into local frame:
  target = quat().rotate(target);
  // push ourselves in that particular direction:
  nudge(target * amt);
}

Nav& Nav::halt() {
  mMove0.set(0);
  mMove1.set(0);
  mSpin0.set(0);
  mSpin1.set(0);
  mTurn.set(0);
  mNudge.set(0);
  updateDirectionVectors();
  return *this;
}

Nav& Nav::home() {
  Pose::set(mHome);
  move(0, 0, 0);
  spin(0, 0, 0);
  updateDirectionVectors();
  return *this;
}

Nav& Nav::setHome() {
  mHome.set(*this);
  return *this;
}

Nav& Nav::view(double azimuth, double elevation, double bank) {
  return view(Quatd().fromEuler(azimuth, elevation, bank));
}

Nav& Nav::view(const Quatd& v) {
  quat(v);
  updateDirectionVectors();
  return *this;
}

Nav& Nav::set(const Pose& v) {
  Pose::set(v);
  updateDirectionVectors();
  return *this;
}

Nav& Nav::set(const Nav& v) {
  Pose::set(v);
  mMove0 = v.mMove0;
  mMove1 = v.mMove1;
  mSpin0 = v.mSpin0;
  mSpin1 = v.mSpin1;
  mTurn = v.mTurn;
  mUR = v.mUR;
  mUU = v.mUU;
  mUF = v.mUF;
  mSmooth = v.mSmooth;
  updateDirectionVectors();
  return *this;
}

void Nav::step(double dt) {
  mVelScale = dt;

  double amt = 1. - smooth();  // TODO: adjust for dt

  // Low-pass filter velocities
  mMove1.lerp(mMove0 * dt + mNudge, amt);
  mSpin1.lerp(mSpin0 * dt + mTurn, amt);

  // Turn and nudge are a one-shot increments, so clear each step
  mTurn.set(0);
  mNudge.set(0);

  // Update orientation from smoothed orientation differential
  // Note that vel() returns a smoothed Pose diff from mMove1 and mSpin1.
  // mQuat *= vel().quat();
  mQuat *= vel().quat();
  updateDirectionVectors();

  // Move according to smoothed position differential (mMove1)
  for (int i = 0; i < pos().size(); ++i) {
    pos()[i] += mMove1.dot(Vec3d(ur()[i], uu()[i], uf()[i]));
  }

  mPullBack1 = mPullBack1 + (mPullBack0 - mPullBack1) * amt;

  mTransformed = *this;
  if (mPullBack1 > 1e-16) {
    mTransformed.pos() -= uf() * mPullBack1;
  }
}

NavInputControl::NavInputControl(double vscale, double tscale)
    : mNav(), mVScale(vscale), mTScale(tscale), mUseMouse(false) {}

NavInputControl::NavInputControl(Nav& nav, double vscale, double tscale)
    : mNav(&nav), mVScale(vscale), mTScale(tscale), mUseMouse(false) {}

NavInputControl::NavInputControl(const NavInputControl& v)
    : mNav(v.mNav), mVScale(v.vscale()), mTScale(v.tscale()), mUseMouse(false) {}

bool NavInputControl::keyDown(const Keyboard& k) {
  if (!mActive) return true;
  if (k.ctrl()) return true;

  double a = mTScale * M_DEG2RAD;  // rotational speed: rad/sec
  double v = mVScale;              // speed: world units/sec

  if (k.alt()) {
    switch (k.key()) {
      case Keyboard::UP:
        nav().pullBack(nav().pullBack() * 0.8);
        return false;
      case Keyboard::DOWN:
        nav().pullBack(nav().pullBack() / 0.8);
        return false;
    }
  }

  if (k.alt()) v *= 10;
  if (k.shift()) v *= 0.1;

  switch (k.key()) {
    case '`':
      nav().halt().home();
      return false;
    case 's':
      nav().halt();
      return false;
    case Keyboard::UP:
      nav().spinR(a);
      return false;
    case Keyboard::DOWN:
      nav().spinR(-a);
      return false;
    case Keyboard::RIGHT:
      nav().spinU(-a);
      return false;
    case Keyboard::LEFT:
      nav().spinU(a);
      return false;
    case 'q':
    case 'Q':
      nav().spinF(a);
      return false;
    case 'z':
    case 'Z':
      nav().spinF(-a);
      return false;
    case 'a':
    case 'A':
      nav().moveR(-v);
      return false;
    case 'd':
    case 'D':
      nav().moveR(v);
      return false;
    case 'e':
    case 'E':
      nav().moveU(v);
      return false;
    case 'c':
    case 'C':
      nav().moveU(-v);
      return false;
    case 'x':
    case 'X':
      nav().moveF(-v);
      return false;
    case 'w':
    case 'W':
      nav().moveF(v);
      return false;
    default:;
  }
  return true;
}

bool NavInputControl::keyUp(const Keyboard& k) {
  if (!mActive) return true;
  // keyUp is for stopping, so no need to skip even if mActive is false
  switch (k.key()) {
    case Keyboard::UP:
    case Keyboard::DOWN:
      nav().spinR(0);
      return false;
    case Keyboard::RIGHT:
    case Keyboard::LEFT:
      nav().spinU(0);
      return false;
    case 'q':
    case 'Q':
    case 'z':
    case 'Z':
      nav().spinF(0);
      return false;
    case 'a':
    case 'A':
    case 'd':
    case 'D':
      nav().moveR(0);
      return false;
    case 'e':
    case 'E':
    case 'c':
    case 'C':
      nav().moveU(0);
      return false;
    case 'x':
    case 'X':
    case 'w':
    case 'W':
      nav().moveF(0);
      return false;
    default:;
  }
  return true;
}

bool NavInputControl::mouseDrag(const Mouse& m) {
  if (!mActive) return true;
  if (mUseMouse) {
    if (m.left()) {
      nav().turnU(-m.dx() * 0.2 * M_DEG2RAD);
      nav().turnR(-m.dy() * 0.2 * M_DEG2RAD);
      return false;
    } else if (m.right()) {
      nav().turnF(m.dx() * 0.2 * M_DEG2RAD);
      nav().pullBack(nav().pullBack() + m.dy() * 0.02);
      return false;
    }
  }
  return true;
}

}  // namespace al
