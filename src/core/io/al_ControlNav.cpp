#include "al/core/io/al_ControlNav.hpp"
// #include <iostream>

namespace al {

Nav::Nav(double smooth)
:  mPosePtr(nullptr), mSmooth(smooth), mVelScale(1), mPullBack0(0), mPullBack1(0)
{
  // updateDirectionVectors();
}

Nav::Nav(Pose& pose, double smooth)
:  mPosePtr(&pose), mSmooth(smooth), mVelScale(1), mPullBack0(0), mPullBack1(0)
{
  updateDirectionVectors();
}

Nav::Nav(const Nav& nav)
:  mPosePtr(nav.mPosePtr),
  mMove0(nav.mMove0), mMove1(nav.mMove1),  // linear velocities (raw, smoothed)
  mSpin0(nav.mSpin0), mSpin1(nav.mSpin1),  // angular velocities (raw, smoothed)
  mTurn(nav.mTurn), mNudge(nav.mNudge),      //
  mSmooth(nav.smooth()), mVelScale(nav.mVelScale),
  mPullBack0(nav.mPullBack0), mPullBack1(nav.mPullBack1)
{
  updateDirectionVectors();
}

void Nav::faceToward(const Vec3d& point, double amt){

  /*Vec3d target(p - pos());
  target.normalize();
  Quatd rot = Quatd::getRotationTo(uf(), target);

  // We must pre-multiply the Pose quaternion with our rotation since
  // it was computed in world space.
  if(amt == 1.)  quat() = rot * quat();
  else      quat() = rot.pow(amt) * quat();*/

  mPosePtr->faceToward(point, amt);
  updateDirectionVectors();
}

void Nav::faceToward(const Vec3d& point, const Vec3d& up, double amt){
  mPosePtr->faceToward(point, up, amt);
  updateDirectionVectors();
}

void Nav::nudgeToward(const Vec3d& p, double amt){
  Vec3d rotEuler;
  Vec3d target(p - mPosePtr->pos());
  target.normalize();  // unit vector of direction to move (in world frame)
  // rotate target into local frame:
  target = mPosePtr->quat().rotate(target);
  // push ourselves in that particular direction:
  nudge(target * amt);
}

Nav& Nav::halt(){
  mMove0.set(0);
  mMove1.set(0);
  mSpin0.set(0);
  mSpin1.set(0);
  mTurn.set(0);
  mNudge.set(0);
  updateDirectionVectors();
  return *this;
}

Nav& Nav::home(){
  mPosePtr->quat().identity();
  view(0, 0, 0);
  turn(0, 0, 0);
  spin(0, 0, 0);
  mPosePtr->vec().set(0);
  updateDirectionVectors();
  return *this;
}

Nav& Nav::view(double azimuth, double elevation, double bank) {
  return view(Quatd().fromEuler(azimuth, elevation, bank));
}

Nav& Nav::view(const Quatd& v) {
  mPosePtr->quat(v);
  updateDirectionVectors();
  return *this;
}

Nav& Nav::set(const Pose& v){
  mPosePtr->set(v);
  updateDirectionVectors();
  return *this;
}

Nav& Nav::set(const Nav& v){
  set(v.pose());
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

void Nav::step(double dt){
  mVelScale = dt;

  double amt = 1.-smooth();  // TODO: adjust for dt

  // Low-pass filter velocities
  mMove1.lerp(mMove0*dt + mNudge, amt);
  mSpin1.lerp(mSpin0*dt + mTurn , amt);

  // Turn and nudge are a one-shot increments, so clear each step
  mTurn.set(0);
  mNudge.set(0);

  // Update orientation from smoothed orientation differential
  // Note that vel() returns a smoothed Pose diff from mMove1 and mSpin1.
  // mQuat *= vel().quat();
  mPosePtr->quat() *= vel().quat();
  updateDirectionVectors();

  // Move according to smoothed position differential (mMove1)
  for(int i=0; i<mPosePtr->pos().size(); ++i){
    mPosePtr->pos()[i] += mMove1.dot(Vec3d(ur()[i], uu()[i], uf()[i]));
  }

  mPullBack1 = mPullBack1 + (mPullBack0-mPullBack1)*amt;

  mTransformed = *mPosePtr;
  if(mPullBack1 > 1e-16){
    mTransformed.pos() -= uf() * mPullBack1;
  }
}


NavInputControl::NavInputControl(double vscale, double tscale)
: mNav(), mVScale(vscale), mTScale(tscale), mUseMouse(true)
{}

NavInputControl::NavInputControl(Pose& pose, double vscale, double tscale)
:	mNav(pose), mVScale(vscale), mTScale(tscale), mUseMouse(true)
{}

NavInputControl::NavInputControl(const NavInputControl& v)
: mNav(v.mNav), mVScale(v.vscale()), mTScale(v.tscale()), mUseMouse(true)
{}

bool NavInputControl::keyDown(const Keyboard& k){
	if(k.ctrl()) return true;

	double a = mTScale * M_DEG2RAD;	// rotational speed: rad/sec
	double v = mVScale;				// speed: world units/sec

	if(k.alt()){
		switch(k.key()){
		case Keyboard::UP:  nav().pullBack(nav().pullBack()*0.8); return false;
		case Keyboard::DOWN:nav().pullBack(nav().pullBack()/0.8); return false;
		}
	}

	if(k.alt()) v *= 10;
	if(k.shift()) v *= 0.1;

	switch(k.key()){
		case '`':				nav().halt().home(); return false;
		case 's':				nav().halt(); return false;
		case Keyboard::UP:		nav().spinR( a); return false;
		case Keyboard::DOWN:	nav().spinR(-a); return false;
		case Keyboard::RIGHT:	nav().spinU(-a); return false;
		case Keyboard::LEFT:	nav().spinU( a); return false;
		case 'q': case 'Q':		nav().spinF( a); return false;
		case 'z': case 'Z':		nav().spinF(-a); return false;
		case 'a': case 'A':		nav().moveR(-v); return false;
		case 'd': case 'D':		nav().moveR( v); return false;
		case 'e': case 'E':		nav().moveU( v); return false;
		case 'c': case 'C':		nav().moveU(-v); return false;
		case 'x': case 'X':		nav().moveF(-v); return false;
		case 'w': case 'W':		nav().moveF( v); return false;
		default:;
	}
	return true;
}

bool NavInputControl::keyUp(const Keyboard& k) {
	switch(k.key()){
		case Keyboard::UP:
		case Keyboard::DOWN:	nav().spinR(0); return false;
		case Keyboard::RIGHT:
		case Keyboard::LEFT:	nav().spinU(0); return false;
		case 'q': case 'Q':
		case 'z': case 'Z':		nav().spinF(0); return false;
		case 'a': case 'A':
		case 'd': case 'D':		nav().moveR(0); return false;
		case 'e': case 'E':
		case 'c': case 'C':		nav().moveU(0); return false;
		case 'x': case 'X':
		case 'w': case 'W':		nav().moveF(0); return false;
		default:;
	}
	return true;
}

bool NavInputControl::mouseDrag(const Mouse& m){
	if(mUseMouse){
		if(m.left()){
			nav().turnU(-m.dx() * 0.2 * M_DEG2RAD);
			nav().turnR(-m.dy() * 0.2 * M_DEG2RAD);
			return false;
		}
		else if(m.right()){
			nav().turnF( m.dx() * 0.2 * M_DEG2RAD);
			nav().pullBack(nav().pullBack() + m.dy()*0.02);
			return false;
		}
	}
	return true;
}

// NavInputControlCosm::NavInputControlCosm(Nav& nav, double vscale, double tscale)
// :	NavInputControl(nav, vscale, tscale)
// {}

// bool NavInputControlCosm::onKeyDown(const Keyboard& k){

// 	double a = mTScale * M_DEG2RAD;	// rotational speed: rad/sec
// 	double v = mVScale;				// speed: world units/sec

// 	if(k.ctrl()) v *= 0.1;
// 	if(k.alt()) v *= 10;

// 	if(k.ctrl()) a *= 0.1;
// 	if(k.alt()) a *= 10;

// 	switch(k.key()){
// 		case '`':				nav().halt().home(); return false;
// 		case 'w':				nav().spinR( a); return false;
// 		case 'x':				nav().spinR(-a); return false;
// 		case Keyboard::RIGHT:	nav().spinU( -a); return false;
// 		case Keyboard::LEFT:	nav().spinU( a); return false;
// 		case 'a':				nav().spinF( a); return false;
// 		case 'd':				nav().spinF(-a); return false;
// 		case ',':				nav().moveR(-v); return false;
// 		case '.':				nav().moveR( v); return false;
// 		case '\'':				nav().moveU( v); return false;
// 		case '/':				nav().moveU(-v); return false;
// 		case Keyboard::UP:		nav().moveF( v); return false;
// 		case Keyboard::DOWN:	nav().moveF(-v); return false;
// 		default:;
// 	}
// 	return true;
// }

// bool NavInputControlCosm::onKeyUp(const Keyboard& k) {
// 	switch (k.key()) {
// 		case 'w':
// 		case 'x':				nav().spinR(0); return false;
// 		case Keyboard::RIGHT:
// 		case Keyboard::LEFT:	nav().spinU(0); return false;
// 		case 'a':
// 		case 'd':				nav().spinF(0); return false;
// 		case ',':
// 		case '.':				nav().moveR(0); return false;
// 		case '\'':
// 		case '/':				nav().moveU(0); return false;
// 		case Keyboard::UP:
// 		case Keyboard::DOWN:	nav().moveF(0); return false;
// 		default:;
// 	}
// 	return true;
// }

// bool NavInputControlCosm::onMouseDrag(const Mouse& m){
// 	return true;
// }

} // al::
