#ifndef INCLUDE_AL_POSE_HPP
#define INCLUDE_AL_POSE_HPP

/*  Allocore --
  Multimedia / virtual environment application class library

  Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology, UCSB.
  Copyright (C) 2012. The Regents of the University of California.
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

    Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.

    Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

    Neither the name of the University of California nor the names of its
    contributors may be used to endorse or promote products derived from
    this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.


  File description:
  Representing an oriented point by vector and quaternion

  File author(s):
  Graham Wakefield, 2010, grrrwaaa@gmail.com
  Lance Putnam, 2010, putnam.lance@gmail.com
  Pablo Colapinto, 2010, wolftype@gmail.com
  Wesley Smith, 2010, wesley.hoke@gmail.com
*/

#include "al/math/al_Matrix4.hpp"
#include "al/math/al_Quat.hpp"
#include "al/math/al_Vec.hpp"
#include <iostream>
#include <stdio.h>

namespace al {

///  A local coordinate frame

///  A Pose is a combined position (3-vector) and orientation (quaternion).
/// Local coordinate bases are referred to as r, u, and f which stand for
/// right, up, and forward, respectively.
///
/// @ingroup Spatial
class Pose {
public:
  /// @param[in] pos    Initial position
  /// @param[in] ori    Initial orientation
  Pose(const Vec3d &pos = Vec3d(0), const Quatd &ori = Quatd::identity());

  /// Copy constructor
  Pose(const Pose &p);

  /// Get identity
  static Pose identity() { return Pose().setIdentity(); }

  // Arithmetic operations

  /// Get pose transformed by another pose
  Pose operator*(const Pose &v) const { return Pose(*this) *= v; }

  /// Translate and rotate by argument
  Pose &operator*=(const Pose &v) {
    mVec += v.vec();
    mQuat *= v.quat();
    return *this;
  }

  // Returns true if both vector and quat are equal
  bool operator==(const Pose &v) const {
    return (mVec == v.pos()) && (mQuat == v.quat());
  }

  /// Turn to face a given world-coordinate point
  void faceToward(const Vec3d &p, double amt = 1.);

  /// Turn to face a given world-coordinate point, while maintaining an up
  /// vector
  void faceToward(const Vec3d &point, const Vec3d &up, double amt = 1.);

  /// Get "position" vector
  Vec3d &pos() { return mVec; }
  const Vec3d &pos() const { return mVec; }

  /// Get vector component
  Vec3d &vec() { return mVec; }
  const Vec3d &vec() const { return mVec; }

  /// Get quaternion component (represents orientation)
  Quatd &quat() { return mQuat; }
  const Quatd &quat() const { return mQuat; }

  double x() const { return mVec[0]; }
  double y() const { return mVec[1]; }
  double z() const { return mVec[2]; }

  /// Convert to 4x4 projection space matrix
  Mat4d matrix() const;

  /// Convert to 4x4 direction matrix
  Mat4d directionMatrix() const;

  /// Get the azimuth, elevation & distance from this to another point
  void toAED(const Vec3d &to, double &azimuth, double &elevation,
             double &distance) const;

  /// Get world space X unit vector
  Vec3d ux() const { return quat().toVectorX(); }

  /// Get world space Y unit vector
  Vec3d uy() const { return quat().toVectorY(); }

  /// Get world space Z unit vector
  Vec3d uz() const { return quat().toVectorZ(); }

  /// Get world space unit vectors
  template <class T>
  void unitVectors(Vec<3, T> &ux, Vec<3, T> &uy, Vec<3, T> &uz) const {
    quat().toVectorX<T>(ux);
    quat().toVectorY<T>(uy);
    quat().toVectorZ<T>(uz);
  }

  /// Get local right, up, and forward unit vectors
  template <class T>
  void directionVectors(Vec<3, T> &ur, Vec<3, T> &uu, Vec<3, T> &uf) const {
    unitVectors(ur, uu, uf);
    uf = -uf;
  }

  /// Get right unit vector
  Vec3d ur() const { return ux(); }

  /// Get up unit vector
  Vec3d uu() const { return uy(); }

  /// Get forward unit vector (negative of Z)
  Vec3d uf() const { return -uz(); }

  /// Get a linear-interpolated Pose between this and another
  // (useful ingredient for smooth animations, estimations, etc.)
  Pose lerp(const Pose &target, double amt) const;

  // Setters

  /// Copy all attributes from another Pose
  Pose &set(Pose &src) {
    mVec = src.pos();
    mQuat = src.quat();
    // mParentTransform = &(src.parentTransform());
    return *this;
  }

  /// Set state from another Pose
  Pose &set(const Pose &src) {
    mVec = src.vec();
    mQuat = src.quat();
    // mParentTransform = &(src.parentTransform());
    return *this;
  }

  /// Set to identity transform
  Pose &setIdentity() {
    quat().setIdentity();
    vec().set(0);
    // mParentTransform = nullptr;
    return *this;
  }

  /// Set position
  template <class T> Pose &pos(const Vec<3, T> &v) { return vec(v); }

  /// Set position from individual components
  Pose &pos(double x, double y, double z) { return vec(Vec3d(x, y, z)); }

  /// Set vector component
  template <class T> Pose &vec(const Vec<3, T> &v) {
    mVec.set(v);
    return *this;
  }

  /// Set quaternion component
  template <class T> Pose &quat(const Quat<T> &v) {
    quat() = v;
    return *this;
  }

  // Overloaded cast operators
  operator Vec3d() { return pos(); }
  operator Quatd() { return quat(); }

  // Pose const& parentTransform() const { return *mParentTransform; }
  // Pose& parentTransform(Pose const& v) {
  //   mParentTransform = &v;
  //   return *this;
  // }
  // Pose worldTransform() const {
  //   return mParentTransform ? mParentTransform->worldTransform() * (*this)
  //                           : (*this);
  // }

  /// Print to standard output
  void print() const;

protected:
  Vec3d mVec;  // position in 3-space
  Quatd mQuat; // orientation of reference frame as a quaternion (relative to
               // global axes)
  // const Pose* mParentTransform;  // parent transform, nullptr if none
};

/// A Smoothed Pose

/// This Pose approaches the stored target Pose exponentially
/// with a curvature determined by psmooth and qsmooth
///
/// @ingroup Spatial
class SmoothPose : public Pose {
public:
  SmoothPose(const Pose &init = Pose(), double psmooth = 0.9,
             double qsmooth = 0.9);

  // step toward the target:
  SmoothPose &operator()() {
    pos().lerp(mTarget.pos(), 1. - mPF);
    quat().slerpTo(mTarget.quat(), 1. - mQF);
    return *this;
  }

  // set and update:
  SmoothPose &operator()(const Pose &p) {
    target(p);
    return (*this)();
  }

  // set the target to smoothly interpolate to:
  Pose &target() { return mTarget; }
  void target(const Pose &p) { mTarget.set(p); }
  void target(const Vec3d &p) { mTarget.pos().set(p); }
  void target(const Quatd &p) { mTarget.quat().set(p); }

  // set immediately (without smoothing):
  void jump(Pose &p) {
    target(p);
    set(p);
  }
  void jump(Vec3d &v) {
    target(v);
    pos().set(v);
  }
  void jump(Quatd &q) {
    target(q);
    quat().set(q);
  }

protected:
  Pose mTarget;
  double mPF, mQF;
};

} // namespace al

#endif
