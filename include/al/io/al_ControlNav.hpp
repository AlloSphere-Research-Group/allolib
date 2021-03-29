#ifndef INCLUDE_AL_CONTROL_NAV_HPP
#define INCLUDE_AL_CONTROL_NAV_HPP

/*	Allocore --
        Multimedia / virtual environment application class library

        Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology,
   UCSB. Copyright (C) 2012. The Regents of the University of California. All
   rights reserved.

        Redistribution and use in source and binary forms, with or without
        modification, are permitted provided that the following conditions are
   met:

                Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

                Redistributions in binary form must reproduce the above
   copyright notice, this list of conditions and the following disclaimer in the
                documentation and/or other materials provided with the
   distribution.

                Neither the name of the University of California nor the names
   of its contributors may be used to endorse or promote products derived from
                this software without specific prior written permission.

        THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
   IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
        IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
   PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
   OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
   OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
   ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


        File description:
        Key/Mouse events to control 3D navigation

        File author(s):
        Lance Putnam, 2010, putnam.lance@gmail.com
        Graham Wakefield, 2010, grrrwaaa@gmail.com
    Keehong Youn, 2017, younkeehong@gmail.com

*/

#include "al/io/al_Window.hpp"
#include "al/spatial/al_Pose.hpp"

namespace al {

///	This manipulates a Pose with smooth angular and positional velocities.
/// The smoothing is done using a one-pole low-pass filter which
/// produces an exponential ease-out type of transition.
///
/// @ingroup IO

class Nav : public Pose {
public:
  /// @param[in] pos    Initial position
  /// @param[in] smooth  Motion smoothing amount in [0,1)
  Nav(const Vec3d &pos = Vec3d(0), double smooth = 0);

  /// Copy constructor
  Nav(const Nav &nav);

  /// Get smoothing amount
  double smooth() const { return mSmooth; }

  /// Get right unit vector
  const Vec3d &ur() const { return mUR; }

  /// Get up unit vector
  const Vec3d &uu() const { return mUU; }

  /// Get forward unit vector
  const Vec3d &uf() const { return mUF; }

  /// Get current linear and angular velocities as a Pose
  Pose vel() const { return Pose(mMove1, Quatd().fromEuler(mSpin1)); }

  double velScale() const { return mVelScale; }

  /// Set smoothing amount in [0,1)
  Nav &smooth(double v) {
    mSmooth = v;
    return *this;
  }

  Nav &view(double azimuth, double elevation, double bank);

  Nav &view(const Quatd &v);

  /// Turn to face a given world-coordinate point
  void faceToward(const Vec3d &p, double amt = 1.);

  /// Turn to face a given world-coordinate point, while maintaining an up
  /// vector
  void faceToward(const Vec3d &point, const Vec3d &up, double amt = 1.);

  /// Move toward a given world-coordinate point
  void nudgeToward(const Vec3d &p, double amt = 1.);

  /// Set linear velocity
  void move(double dr, double du, double df) {
    moveR(dr);
    moveU(du);
    moveF(df);
  }
  template <class T> void move(const Vec<3, T> &dp) {
    move(dp[0], dp[1], dp[2]);
  }

  /// Set linear velocity along right vector
  void moveR(double v) { mMove0[0] = v; }

  /// Set linear velocity long up vector
  void moveU(double v) { mMove0[1] = v; }

  /// Set linear velocity long forward vector
  void moveF(double v) { mMove0[2] = v; }

  Vec3d &move() { return mMove0; }

  /// Move by a single increment
  void nudge(double dr, double du, double df) {
    nudgeR(dr);
    nudgeU(du);
    nudgeF(-df);
  }
  template <class T> void nudge(const Vec<3, T> &dp) {
    nudge(dp[0], dp[1], dp[2]);
  }

  void nudgeR(double amount) { mNudge[0] += amount; }
  void nudgeU(double amount) { mNudge[1] += amount; }
  void nudgeF(double amount) { mNudge[2] += amount; }

  /// Set angular velocity from azimuth, elevation, and bank differentials, in
  /// radians
  void spin(double da, double de, double db) {
    spinR(de);
    spinU(da);
    spinF(db);
  }
  template <class T> void spin(const Vec<3, T> &daeb) {
    spin(daeb[0], daeb[1], daeb[2]);
  }

  /// Set angular velocity from a unit quaternion (versor)
  void spin(const Quatd &v) { v.toEuler(mSpin1); }

  /// Set angular velocity around right vector (elevation), in radians
  void spinR(double v) { mSpin0[1] = v; }

  /// Set angular velocity around up vector (azimuth), in radians
  void spinU(double v) { mSpin0[0] = v; }

  /// Set angular velocity around forward vector (bank), in radians
  void spinF(double v) { mSpin0[2] = v; }

  /// Set angular velocity directly
  Vec3d &spin() { return mSpin0; }

  /// Turn by a single increment for one step, in radians
  void turn(double az, double el, double ba) {
    turnR(el);
    turnU(az);
    turnF(ba);
  }
  template <class T> void turn(const Vec<3, T> &daeb) {
    turn(daeb[0], daeb[1], daeb[2]);
  }

  /// Turn by a single increment, in radians, around the right vector
  /// (elevation)
  void turnR(double v) { mTurn[1] = v; }

  /// Turn by a single increment, in radians, around the up vector (azimuth)
  void turnU(double v) { mTurn[0] = v; }

  /// Turn by a single increment, in radians, around the forward vector (bank)
  void turnF(double v) { mTurn[2] = v; }

  /// Stop moving and spinning
  Nav &halt();

  /// Go to origin, reset orientation
  Nav &home();

  /// set current nav to be home
  Nav &setHome();

  /// Update coordinate frame basis vectors based on internal quaternion
  void updateDirectionVectors() {
    quat().normalize();
    directionVectors(mUR, mUU, mUF);
  }

  Nav &set(const Pose &v);

  Nav &set(const Nav &v);

  /// Accumulate pose based on velocity
  void step(double dt = 1);

  /// Get pull-back amount
  double pullBack() const { return mPullBack0; }

  /// Set pull-back amount
  Nav &pullBack(double v) {
    mPullBack0 = v > 0. ? v : 0.;
    return *this;
  }

  /// Get transformed pose
  Pose &transformed() { return mTransformed; }

protected:
  Vec3d mMove0, mMove1; // linear velocities (raw, smoothed)
  Vec3d mSpin0, mSpin1; // angular velocities (raw, smoothed)
  Vec3d mTurn;          // orientation increment for one step
  Vec3d mNudge;         // position increment for one step
  Vec3d mUR, mUU, mUF;  // basis vectors of local coordinate frame
  double mSmooth;
  double mVelScale; // velocity scaling factor
  double mPullBack0, mPullBack1;
  Pose mTransformed;
  Pose mHome;
};

/// Mapping from keyboard and mouse controls to a Nav object
///
/// @ingroup allocore
class NavInputControl : public WindowEventHandler {
public:
  NavInputControl(double vscale = 0.125, double tscale = 2.);
  NavInputControl(Nav &nav, double vscale = 0.125, double tscale = 2.);
  NavInputControl(const NavInputControl &v);

  virtual ~NavInputControl() {}

  virtual bool keyDown(const Keyboard &k);
  virtual bool keyUp(const Keyboard &k);
  virtual bool mouseDrag(const Mouse &m);

  void nav(Nav &n) { mNav = &n; }
  Nav &nav() { return *mNav; }
  const Nav &nav() const { return *mNav; }
  // NavInputControl& nav(Nav& v){ mNav=v; return *this; }

  // NavInputControl& target(Pose& pose) {
  //    mNav.target(pose);
  //    return *this;
  // }

  void active(bool b) {
    mActive = b;
    if (!mActive)
      nav().halt();
  }
  bool active() { return mActive; }
  void enable() { active(true); }
  void disable() { active(false); }
  void toggleActive() { active(!active()); }

  double vscale() const { return mVScale; }
  NavInputControl &vscale(double v) {
    mVScale = v;
    return *this;
  }

  double tscale() const { return mTScale; }
  NavInputControl &tscale(double v) {
    mTScale = v;
    return *this;
  }

  void useMouse(bool use) { mUseMouse = use; }

  // void step(double dt=1) {
  // 	mNav.step(dt);
  // }
protected:
  Nav *mNav = nullptr;
  double mVScale, mTScale;
  bool mUseMouse;
  bool mActive = true;
};

} // namespace al

#endif
