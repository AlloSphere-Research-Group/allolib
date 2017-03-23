#include "al/core/gl/al_Viewpoint.hpp"

using namespace al;

// Viewpoint::Viewpoint(
//     Pose const& pose, Viewport const& vp, Lens const& lens
// ):
//     Pose(pose),
//     mLens(lens),
//     mViewport(vp)
// {

// }

Viewpoint& Viewpoint::fovy(float deg) {
    mLens.fovy(deg);
    return *this;
}

Viewpoint& Viewpoint::near(float n) {
    mLens.near(n);
    return *this;
}
Viewpoint& Viewpoint::far(float f) {
    mLens.far(f);
    return *this;
}

// Viewpoint& Viewpoint::pos(Vec3f v) {
//     mPose->pos(v);
//     return *this;
// }

// Viewpoint& Viewpoint::faceToward(Vec3f point, Vec3f upvec) {
//     mPose-> faceToward(point, upvec);
//     return *this;
// }


Viewpoint& Viewpoint::viewport(int left, int bottom, int width, int height) {
    mViewport.set(left, bottom, width, height);
    return *this;
}


Matrix4f Viewpoint::viewMatrix() {
    Vec3f ux, uy, uz;
    unitVectors(ux, uy, uz);
    return Matrix4f::lookAt(ux, uy, uz, pos());
}

Matrix4f Viewpoint::projMatrix() {
    if (!isOrtho_) return Matrix4f::perspective(
      mLens.fovy(), mViewport.aspect(), mLens.near(), mLens.far()
    );
    else return Matrix4f::ortho(
        -mViewport.w / 2, // left clipping plane
        mViewport.w / 2, // right
        -mViewport.h / 2, // bottom
        mViewport.h / 2, // top
        mLens.near(),
        mLens.far()
    );
}

Frustumd Viewpoint::frustum() const {
    Frustumd fr;
    lens().frustum(fr, worldTransform(), viewport().aspect());
    return fr;
}