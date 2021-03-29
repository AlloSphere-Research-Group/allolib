#include "al/graphics/al_Font.hpp"

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <unordered_map>
#include <vector>

#include "al_stb_font.hpp"

namespace al {

struct Font::Impl {
  al_stb::FontData fontData;
  std::unordered_map<int, al_stb::CharData> cachedData;
};

Font::Font() {
  impl = new Impl;
  if (!impl) std::cerr << "Error allocating font implementation\n";
}

Font::Font(Font&& other) noexcept {
  impl = other.impl;
  other.impl = nullptr;
}

Font& Font::operator=(Font&& other) noexcept {
  Impl* temp = impl;
  impl = other.impl;
  other.impl = temp;
  return *this;
}

Font::~Font() { delete impl; }

bool Font::load(const char* filename, int fontSize, int bitmapSize) {
  impl->fontData = al_stb::loadFont(filename, (float)fontSize, bitmapSize);
  if (impl->fontData.charData.size() == 0) {
    return false;
  }
  tex.create2D(impl->fontData.width, impl->fontData.height, GL_R8, GL_RED,
               GL_UNSIGNED_BYTE);
  tex.submit(impl->fontData.bitmap.data());
  tex.filter(GL_LINEAR);
  tex.bind_temp();
  // make `texture` in glsl return (1, 1, 1, r)
  GLint swizzleMask[] = {GL_ONE, GL_ONE, GL_ONE, GL_RED};
  glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
  tex.unbind_temp();
  return true;
}

void Font::write(Mesh& mesh, const char* text, float worldHeight) {
  assert(impl->fontData.charData.size() > 0 && "Error - font not loaded.");
  mesh.reset();

  float xpos = 0;
  float scale = worldHeight / impl->fontData.pixelHeight;

  auto* cache = &(impl->cachedData);
  auto* fontData = &(impl->fontData);

  auto cdata = [cache, fontData](int c) -> const al_stb::CharData& {
    auto search = cache->find(c);
    if (search != cache->end())
      return search->second;
    else
      return ((*cache)[c] = al_stb::getCharData(fontData, c));
  };

  while (*text) {
    int c = *text;
    auto d = cdata(c);

    float x0 = xpos + d.x0 * scale;
    float x1 = xpos + d.x1 * scale;
    float y0 = -d.y0 * scale;
    float y1 = -d.y1 * scale;

    mesh.vertex(x0, y0, 0);
    mesh.vertex(x1, y0, 0);
    mesh.vertex(x0, y1, 0);
    mesh.vertex(x0, y1, 0);
    mesh.vertex(x1, y0, 0);
    mesh.vertex(x1, y1, 0);

    mesh.texCoord(d.s0, d.t0);
    mesh.texCoord(d.s1, d.t0);
    mesh.texCoord(d.s0, d.t1);
    mesh.texCoord(d.s0, d.t1);
    mesh.texCoord(d.s1, d.t0);
    mesh.texCoord(d.s1, d.t1);

    xpos += d.xAdvance * scale;

    ++text;
  }

  float xOffset = xpos * alignFactorX;
  for (auto& v : mesh.vertices()) {
    v.x = v.x + xOffset;
  }
}

std::string Font::defaultFont() {
#ifdef AL_WINDOWS
  std::string fontDir = "C:/Windows/Fonts";
  std::string fontPath = fontDir + "/" + "Arial.ttf";
#elif defined(AL_OSX)
  std::string fontDir = "/Library/Fonts";
  std::string fontPath = fontDir + "/" + "Arial Unicode.ttf";
#else
  std::string fontDir = "/usr/share/fonts/truetype";
  std::string fontPath = fontDir + "/dejavu/DejaVuSansMono.ttf";
#endif

  return fontPath;
}

#if 0
float Font::ascender() const { return mImpl->ascender(); }

float Font::descender() const { return mImpl->descender(); }

void Font::align(float xfrac, float yfrac){
    mAlign[0] = xfrac;
    mAlign[1] = yfrac;
}

float Font::width(const char* text) const {
  float total = 0.f;
  size_t len = std::strlen(text);
  for (size_t i=0; i < len; i++) {
    total += mChars[ (int)text[i] ].width;
  }
  return total;
}
#endif

}  // namespace al
