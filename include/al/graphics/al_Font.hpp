#ifndef INCLUDE_AL_FONT_HPP
#define INCLUDE_AL_FONT_HPP

/*  Allocore --
    Multimedia / virtual environment application class library

    Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology,
   UCSB. Copyright (C) 2012. The Regents of the University of California. All
   rights reserved.

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
    (previous) Abstraction over the FreeType library
    Abstraction over the stb font library

    File author(s):
    Graham Wakefield, 2010, grrrwaaa@gmail.com
    Keehong Youn, 2019, younkeehong@gmail.com
*/

#include <map>

#include "al/graphics/al_Graphics.hpp"
#include "al/graphics/al_Mesh.hpp"
#include "al/graphics/al_Texture.hpp"

namespace al {

/**
@brief Interface for loading fonts and rendering text
@ingroup Graphics
*/

struct Font {
  struct Impl;
  Impl *impl;
  Texture tex;
  float alignFactorX = 0.0f;

  Font();

  // no copy
  Font(const Font &) = delete;
  Font &operator=(const Font &) = delete;

  // but movable
  Font(Font &&other) noexcept;
  Font &operator=(Font &&other) noexcept;

  ~Font();

  /// Load font from file
  /// \param[in] filename        path to font file
  /// \param[in] fontSize        size of font
  /// \param[in] bitmapSize      size of font bitmap
  /// \returns whether font loaded successfully
  bool load(const char *filename, int fontSize, int bitmapSize);

  /*! Render text geometry
      Render text into geometry for drawing a string of text using the bitmap
      returned by ascii_chars.  Render expects the vertex and texcoord buffers
      to be at least as big as the text length * 4 since each character is
      rendered by a quad.

      Example usage:
      <pre>
          Mesh mesh;
          font.write(mesh, "allocore");

          font.texture().bind();
          g.draw(mesh);
          font.texture().unbind();
      </pre>

  */
  void write(Mesh &mesh, const char *text, float worldHeight);

  /// Returns the width of a text string, in pixels
  // float width(const char* text) const;

  /// Returns the width of a character, in pixels
  // float width(int c) const;

  /// Returns the "above-line" height of the font, in pixels
  // float ascender() const;

  /// Returns the "below-line" height of the font, in pixels
  // float descender() const;

  /// Returns the total height of the font, in pixels
  // float size() const { return (float)mFontSize; }

  /// Set alignment of rendered text strings

  /// \param[in] xfrac    Fraction along text width to render at x=0;
  ///                        0 is left-aligned, 0.5 is centered, 1 is
  ///                        right-aligned
  /// \param[in] yfrac    Fraction along text height to render at y=0
  // void align(float xfrac, float yfrac);

  void alignLeft() { alignFactorX = 0.0f; }
  void alignCenter() { alignFactorX = -0.5f; }
  void alignRight() { alignFactorX = -1.0f; }

  static std::string defaultFont();
};

/**
 * @brief Convenience class to simplify rendering text
 *
 */
struct FontRenderer : public Font {
  Mesh fontMesh;

  void write(const char *text, float worldHeight = 1.0f) {
    Font::write(fontMesh, text, worldHeight);
  }

  static void render(Graphics &g, const char *text, Vec3d position,
                     float worldHeight = 1.0, int fontSize = 24,
                     const char *filename = Font::defaultFont().c_str(),
                     int bitmapSize = 1024) {
    static std::map<std::string, FontRenderer> renderers;
    if (renderers.find(std::string(filename)) == renderers.end()) {
      renderers[std::string(filename)] = FontRenderer();
      renderers[filename].load(filename, fontSize, bitmapSize);
    }
    // FIXME check bitmap size for case when font is reused with different
    // resolution
    FontRenderer &renderer = renderers[filename];
    renderer.write(text, worldHeight);
    renderer.renderAt(g, position);
  }

  void render(Graphics &g) {
    g.blending(true);
    g.blendTrans();
    g.texture();
    tex.bind();
    g.draw(fontMesh);
    tex.unbind();
  }

  void renderAt(Graphics &g, Vec3d position) {
    g.pushMatrix();
    g.translate(position);
    render(g);
    g.popMatrix();
  }
};  // namespace al

}  // namespace al

#endif
