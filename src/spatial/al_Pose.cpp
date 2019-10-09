#include "al/spatial/al_Pose.hpp"

namespace al {

Pose::Pose(const Vec3d& v, const Quatd& q)
    : mVec(v),
      mQuat(q)  //, mParentTransform(nullptr)
{}

Pose::Pose(const Pose& p) { set(p); }

void Pose::faceToward(const Vec3d& point, double amt) {
  Vec3d target(point - pos());
  target.normalize();
  Quatd rot = Quatd::getRotationTo(uf(), target);

  // We must pre-multiply the Pose quaternion with our rotation since
  // it was computed in world space.
  if (amt == 1.)
    quat() = rot * quat();
  else
    quat() = rot.pow(amt) * quat();

  /* Apply rotation using Euler angles (can behave erraticly)
  Vec3d aeb1, aeb2;
  quat().toEuler(aeb1);
  (rot * quat()).toEuler(aeb2);
  Vec3d rotEuler = aeb2 - aeb1;
  //for(int i=0; i<3; ++i){ // minimize angles
  //  double& r = rotEuler[i];
  //  if(r > M_PI) r -= 2*M_PI;
  //  else if(r < -M_PI) r += 2*M_PI;
  //}
  rot.toEuler(rotEuler);
  mTurn.set(rotEuler * amt);
  //*/
}

void Pose::faceToward(const Vec3d& point, const Vec3d& up, double amt) {
  // Vec3d target(point - pos());
  // target.normalize();
  // Quatd rot = Quatd::getBillboardRotation(-target, up);
  quat().slerpTo(Quatd::getBillboardRotation((pos() - point).normalize(), up),
                 amt);
}

Mat4d Pose::matrix() const {
  Mat4d m;
  quat().toMatrix(&m[0]);
  m.set(&vec()[0], 3, 12);
  return m;
}

Mat4d Pose::directionMatrix() const {
  Mat4d m = matrix();
  m(0, 2) = -m(0, 2);
  m(1, 2) = -m(1, 2);
  m(2, 2) = -m(2, 2);
  return m;
}

Pose Pose::lerp(const Pose& target, double amt) const {
  Pose r(*this);
  r.pos().lerp(target.pos(), amt);
  r.quat().slerpTo(target.quat(), amt);
  return r;
}

void Pose::toAED(const Vec3d& to, double& az, double& el, double& dist) const {
  Vec3d rel = to - vec();
  dist = rel.mag();

  if (dist > quat().eps() * 2) {
    rel.normalize();

    Vec3d ux, uy, uz;

    quat().toVectorX(ux);
    quat().toVectorY(uy);
    quat().toVectorZ(uz);

    // dot product of A & B vectors is the similarity or cosine:
    double xness = rel.dot(ux);
    double yness = rel.dot(uy);
    double zness = rel.dot(uz);

    az = -atan2(xness, zness);
    el = asin(yness);
  } else {
    // near origin; might as well assume 0 to avoid denormals
    // do not set az/el; they may already have more meaningful values
    dist = 0.0;
  }
}

void Pose::print() const {
  printf("Vec3d(%f, %f, %f);\nQuatd(%f, %f, %f, %f);\n", mVec[0], mVec[1],
         mVec[2], mQuat[0], mQuat[1], mQuat[2], mQuat[3]);
}

SmoothPose::SmoothPose(const Pose& init, double psmooth, double qsmooth)
    : Pose(init), mTarget(init), mPF(psmooth), mQF(qsmooth) {}

}  // namespace al
