#ifndef INCLUDE_AL_VIEWPOINT_HPP
#define INCLUDE_AL_VIEWPOINT_HPP

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
  

  viewpoint: Pose(view matrix) + Lens(projection matrix) + viewport

*/

#include "al/core/spatial/al_Pose.hpp"
#include "al/core/gl/al_Lens.hpp"
#include "al/core/math/al_Frustum.hpp"

namespace al {

/// A framed area on a display screen
/// @ingroup allocore
struct Viewport {
  float l, b, w, h; ///< left, bottom, width, height

  /// @param[in] w  width
  /// @param[in] h  height
  Viewport(float w=640, float h=480): l(0), b(0), w(w), h(h) {}

  /// @param[in] l  left edge coordinate
  /// @param[in] b  bottom edge coordinate
  /// @param[in] w  width
  /// @param[in] h  height
  Viewport(float l, float b, float w, float h): l(l), b(b), w(w), h(h) {}

  ///
  Viewport(const Viewport& cpy): l(cpy.l), b(cpy.b), w(cpy.w), h(cpy.h) {}

  /// Get aspect ratio (width divided by height)
  float aspect() const { return (h!=0 && w!=0) ? float(w)/h : 1; }

  /// Set dimensions
  void set(float l_, float b_, float w_, float h_){ l=l_; b=b_; w=w_; h=h_; }
};

/// Viewpoint within a scene

/// A viewpoint is an aggregation of a viewport (screen region), a pose
/// (3D position and orientation), and a lens.
///
/// @ingroup allocore
class Viewpoint : public Pose {
public:

  // Viewpoint(const Pose& transform = Pose::identity());
  Viewpoint(Viewport const& vp = Viewport(0, 0, 0, 0), Lens const& lens = Lens());

  float anchorX() const { return mAnchorX; }
  float anchorY() const { return mAnchorY; }
  float stretchX() const { return mStretchX; }
  float stretchY() const { return mStretchY; }

  /// Set anchoring factors relative to bottom-left corner of window

  /// @param[in] ax anchor factor relative to left edge of window, in [0,1]
  /// @param[in] ay anchor factor relative to bottom edge of window, in [0,1]
  Viewpoint& anchor(float ax, float ay);

  /// Set stretch factors relative to bottom-left corner of window

  /// @param[in] sx stretch factor relative to left edge of window, in [0,1]
  /// @param[in] sy stretch factor relative to bottom edge of window, in [0,1]
  Viewpoint& stretch(float sx, float sy);

  // bool hasLens() const { return NULL != mLens; }

  /// Get lens
  const Lens& lens() const { return mLens; }
  Lens& lens() { return mLens; }
  Viewpoint& lens(Lens const& v){ mLens=v; return *this; }

  /// Get parent transform
  // const Pose* parentTransform() const { return mParentTransform; }
  // Viewpoint& parentTransform(Pose& v){ mParentTransform =&v; return *this; }
  // Viewpoint& parentTransform(Pose* v){ mParentTransform = v; return *this; }

  /// Get local transform
  const Pose& transform() const { return *this; }
  Pose& transform(){ return *this; }
  Viewpoint& transform(const Pose& v){ set(v); return *this; }

  // Pose worldTransform() const { return mParentTransform ? (*mParentTransform) * transform() : transform(); }

  /// Get screen viewport
  const Viewport& viewport() const { return mViewport; }
  Viewport& viewport(){ return mViewport; }
  Viewpoint& viewport(Viewport const& vp){ mViewport = vp; return *this; }

  /// Get calculated viewing frustum
  Frustumd frustum() const;

  /// Call to update viewport using stretch/anchor amounts when parent dimensions change
  void onParentResize(int w, int h);

  Matrix4f viewMatrix() {
    return Matrix4f::lookAt(
      ur(), // right
      uu(), // up
      uf(), // forward
      pos() // eyePos
    );
  }

  Matrix4f projMatrix() {
    return Matrix4f::perspective(
      mLens.fovy(), mViewport.aspect(), mLens.near(), mLens.far()
    );
  }

private:
  // Pose* mTransform;           // local transform
  Lens mLens;
  Viewport mViewport;         // screen display region
  // Pose* mParentTransform;     // parent transform, nullptr if none
  float mAnchorX, mAnchorY;   // viewport anchor factors relative to parent window
  float mStretchX, mStretchY; // viewport stretch factors relative to parent window
};

} // al::

#endif
