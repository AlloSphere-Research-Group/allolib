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

*/

#include "al/graphics/al_Lens.hpp"
#include "al/spatial/al_Pose.hpp"

namespace al {

Matrix4f view_mat(Pose const&);

/// A framed area on a display screen
/// @ingroup Graphics
struct Viewport {
  int l, b, w, h;  ///< left, bottom, width, height

  /// @param[in] w  width
  /// @param[in] h  height
  Viewport(int w = 640, int h = 480) : l(0), b(0), w(w), h(h) {}

  /// @param[in] l  left edge coordinate
  /// @param[in] b  bottom edge coordinate
  /// @param[in] w  width
  /// @param[in] h  height
  Viewport(int l, int b, int w, int h) : l(l), b(b), w(w), h(h) {}

  ///
  Viewport(const Viewport& cpy) : l(cpy.l), b(cpy.b), w(cpy.w), h(cpy.h) {}

  /// Get aspect ratio (width divided by height)
  float aspect() const { return (h != 0 && w != 0) ? float(w) / h : 1; }

  /// Set dimensions
  void set(int l_, int b_, int w_, int h_) {
    l = l_;
    b = b_;
    w = w_;
    h = h_;
  }
  void set(const Viewport& cpy) {
    l = cpy.l;
    b = cpy.b;
    w = cpy.w;
    h = cpy.h;
  }

  bool isEqual(Viewport const& v) {
    return (l == v.l) && (b == v.b) && (w == v.w) && (h == v.h);
  }
};

/// Viewpoint within a scene

/// A viewpoint is an aggregation of a lens with a pointer to the pose
/// (3D pos and orientation) that it is attached to
///
/// @ingroup allocore
class Viewpoint {
 public:
  Viewpoint() {}
  Viewpoint(Pose& p) : mPose(&p) {}

  const Lens& lens() const { return mLens; }
  Lens& lens() { return mLens; }
  Viewpoint& lens(Lens const& v) {
    mLens = v;
    return *this;
  }

  const Pose& pose() const { return *mPose; }
  Pose& pose() { return *mPose; }
  Viewpoint& pose(Pose& p) {
    mPose = &p;
    return *this;
  }

  Matrix4f viewMatrix() const;
  Matrix4f projMatrix(float aspect_ratio) const;
  Matrix4f projMatrix(float width, float height) const {
    return projMatrix(width / height);
  }

 private:
  Lens mLens;
  Pose* mPose = nullptr;

 public:
  enum SpecialType {
    IDENTITY,
    ORTHO_FOR_2D,
    UNIT_ORTHO,           // fits [-1:1] X [-1:1] inside
    UNIT_ORTHO_INCLUSIVE  // fits in [-1:1] X [-1:1]
  };
};

}  // namespace al

#endif
