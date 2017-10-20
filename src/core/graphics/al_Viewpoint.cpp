#include "al/core/graphics/al_Viewpoint.hpp"

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

// Viewpoint& Viewpoint::viewport(int left, int bottom, int width, int height) {
//     mViewport.set(left, bottom, width, height);
//     return *this;
// }

Matrix4f Viewpoint::viewMatrix() const
{
    Vec3f ux, uy, uz;
    unitVectors(ux, uy, uz);
    return Matrix4f::lookAt(ux, uy, uz, pos());
}

// Matrix4f Viewpoint::projMatrix() const
// {
//     if (!mIsOrtho) return Matrix4f::perspective(
//       mLens.fovy(), mViewport.aspect(), mLens.near(), mLens.far()
//     );

//     else return Matrix4f::ortho(l, r, b, t, mLens.near(), mLens.far());
// }

Matrix4f Viewpoint::projMatrix(float aspect_ratio) const
{
    return Matrix4f::perspective(
        mLens.fovy(), aspect_ratio, mLens.near(), mLens.far()
    );
}

// Matrix4f Viewpoint::projMatrix(float x, float y, float w, float h) const
// {
//     if (!mIsOrtho) return Matrix4f::perspective(
//       mLens.fovy(), w / h, mLens.near(), mLens.far()
//     );

//     else return Matrix4f::ortho(x, x + w, y, y + h, mLens.near(), mLens.far());
// }

// Viewpoint& Viewpoint::ortho(float left, float bottom, float right, float top)
// {
//     ortho(true);
//     l = left;
//     b = bottom;
//     r = right;
//     t = top;
//     return *this;
// }