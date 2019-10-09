#define _CRT_SECURE_NO_WARNINGS

#include "al_stb_font.hpp"
#include <cstddef>  // offsetof
#include <iostream>

#define STB_TRUETYPE_IMPLEMENTATION  // force following include to generate
                                     // implementation
#include "stb_truetype.h"

#define ASCII_FIRST_CHAR 32  // ascii 0 to 31 are special characters
#define CHAR_DATA_LEN 95     // ASCII 32..126 is 95 glyphs

al_stb::FontData al_stb::loadFont(const char* filename, float pixelHeight,
                                  int bitmapSize) {
  FontData fontData;
  FILE* fontFile = fopen(filename, "rb");
  if (fontFile) {
    std::vector<uint8_t> buffer(1 << 20);
    size_t bytesRead =
        fread(buffer.data(), sizeof(uint8_t), buffer.size(), fontFile);
    fclose(fontFile);
    if (!bytesRead) {
      std::cerr << "[al_stb::loadFont] no bytes read from font file: "
                << filename << '\n';
      return fontData;
    }
    fontData.charData.resize(CHAR_DATA_LEN);
    fontData.bitmap.resize(bitmapSize * bitmapSize);
    // no guarantee this fits!
    stbtt_BakeFontBitmap(buffer.data(), 0, pixelHeight, fontData.bitmap.data(),
                         bitmapSize, bitmapSize, ASCII_FIRST_CHAR,
                         CHAR_DATA_LEN,
                         (stbtt_bakedchar*)fontData.charData.data());
    fontData.width = bitmapSize;
    fontData.height = bitmapSize;
    fontData.pixelHeight = pixelHeight;
  } else {
    std::cerr << "[al_stb::loadFont] could not open font file: " << filename
              << '\n';
  }
  return fontData;
}

// mod ver of stbtt_GetBakedQuad
// assumes opengl use (no d3d)
static void GetBakedChar(const stbtt_bakedchar* bakedchars, int pw, int ph,
                         int char_index, al_stb::CharData* c) {
  float ipw = 1.0f / pw, iph = 1.0f / ph;
  const stbtt_bakedchar* b = bakedchars + char_index;
  float round_x = (float)STBTT_ifloor(b->xoff + 0.5f);
  float round_y = (float)STBTT_ifloor(b->yoff + 0.5f);

  c->x0 = round_x;
  c->y0 = round_y;
  c->x1 = round_x + b->x1 - b->x0;
  c->y1 = round_y + b->y1 - b->y0;

  c->s0 = b->x0 * ipw;
  c->t0 = b->y0 * iph;
  c->s1 = b->x1 * ipw;
  c->t1 = b->y1 * iph;

  c->xAdvance = b->xadvance;
}

al_stb::CharData al_stb::getCharData(FontData* fontData, int charIndex) {
  al_stb::CharData charData;
  GetBakedChar((stbtt_bakedchar*)fontData->charData.data(), fontData->width,
               fontData->height, charIndex - ASCII_FIRST_CHAR, &charData);
  return charData;
}
