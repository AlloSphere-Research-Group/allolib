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

  Keehong Youn, 2017, younkeehong@gmail.com
*/

#include "al/core/spatial/al_Pose.hpp"
#include "al/core/graphics/al_Lens.hpp"

namespace al {

/// A framed area on a display screen
/// @ingroup allocore
struct Viewport {
  int l, b, w, h; ///< left, bottom, width, height

  /// @param[in] w  width
  /// @param[in] h  height
  Viewport(int w=640, int h=480): l(0), b(0), w(w), h(h) {}

  /// @param[in] l  left edge coordinate
  /// @param[in] b  bottom edge coordinate
  /// @param[in] w  width
  /// @param[in] h  height
  Viewport(int l, int b, int w, int h): l(l), b(b), w(w), h(h) {}

  ///
  Viewport(const Viewport& cpy): l(cpy.l), b(cpy.b), w(cpy.w), h(cpy.h) {}

  /// Get aspect ratio (width divided by height)
  float aspect() const { return (h!=0 && w!=0) ? float(w)/h : 1; }

  /// Set dimensions
  void set(int l_, int b_, int w_, int h_){ l=l_; b=b_; w=w_; h=h_; }
  void set(const Viewport& cpy){ l=cpy.l; b=cpy.b; w=cpy.w; h=cpy.h; }

  bool isEqual(Viewport const& v) {
    return (l == v.l) && (b ==  v.b) && (w == v.w) && (h == v.h);
  }
};

/// Viewpoint within a scene

/// A viewpoint is an aggregation of a viewport (screen region) and a lens, 
/// with a pointer to the pose (3D pos and orientation) that it is attached to
///
/// @ingroup allocore
class Viewpoint : public Pose {
public:

  const Lens& lens() const { return mLens; }
  Lens& lens() { return mLens; }
  Viewpoint& lens(Lens const& v){ mLens=v; return *this; }
  Viewpoint& fovy(float deg);
  Viewpoint& near(float n);
  Viewpoint& far(float f);

  // const Pose& pose() const { return *mPose; }
  // Pose& pose(){ return *mPose; }
  Viewpoint& pose(Pose& v){ set(v); return *this; }
  Viewpoint& pose(Pose const& v){ set(v); return *this; }
  // Viewpoint& faceToward(Vec3f point, Vec3f upvec);

  // const Viewport& viewport() const { return mViewport; }
  // Viewport& viewport(){ return mViewport; }
  // Viewpoint& viewport(Viewport const& vp){ mViewport = vp; return *this; }
  // Viewpoint& viewport(int left, int bottom, int width, int height);

  Matrix4f viewMatrix() const;
  // Matrix4f projMatrix() const;
  Matrix4f projMatrix(float aspect_ratio) const;
  Matrix4f projMatrix(float width, float height) const {
    return projMatrix(width / height);
  }
  // Matrix4f projMatrix(float x, float y, float w, float h) const;
  // Matrix4f projMatrix(float w, float h) const {
  //   return projMatrix(-w / 2, -h / 2, w, h);
  // };


private:
  Lens mLens;
  // Viewport mViewport; // screen display region

public:
  enum SpecialType {
    IDENTITY,
    ORTHO_FOR_2D,
    UNIT_ORTHO
  };
};

} // al::

#endif
