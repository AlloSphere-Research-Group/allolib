#include "al/core/gl/al_Viewpoint.hpp"

using namespace al;

// Viewpoint::Viewpoint(const Pose& transform)
// :	mViewport(0,0,0,0),
// 	mParentTransform(NULL),
// 	mTransform(&transform),
// 	mAnchorX(0), mAnchorY(0), mStretchX(1), mStretchY(1)
// {}

Viewpoint::Viewpoint(Viewport const& vp, Lens const& lens)
:	Pose(Pose::identity()),
	mLens(lens),
	mViewport(vp),
	// mTransform(&transform),
	mAnchorX(0), mAnchorY(0), mStretchX(1), mStretchY(1)
{}

Viewpoint& Viewpoint::anchor(float ax, float ay){
	mAnchorX=ax; mAnchorY=ay; return *this;
}

Viewpoint& Viewpoint::stretch(float sx, float sy){
	mStretchX=sx; mStretchY=sy; return *this;
}

Frustumd Viewpoint::frustum() const {
	Frustumd fr;
	lens().frustum(fr, worldTransform(), viewport().aspect());
	return fr;
}

void Viewpoint::onParentResize(int w, int h){
	mViewport.l = w * anchorX();
	mViewport.b = h * anchorY();
	mViewport.w = w * stretchX();
	mViewport.h = h * stretchY();
}