#include "al/graphics/al_Viewpoint.hpp"

using namespace al;

Matrix4f al::view_mat(Pose const& p) {
  Vec3f ux, uy, uz;
  p.unitVectors(ux, uy, uz);
  return Matrix4f::lookAt(ux, uy, uz, p.pos());
}

Matrix4f Viewpoint::viewMatrix() const { return view_mat(*mPose); }

Matrix4f Viewpoint::projMatrix(float aspect_ratio) const {
  return Matrix4f::perspective(mLens.fovy(), aspect_ratio, mLens.near(),
                               mLens.far());
}