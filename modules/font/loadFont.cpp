#include "loadFont.hpp"
#include <iostream>

#define STB_TRUETYPE_IMPLEMENTATION  // force following include to generate implementation
#define STBTT_STATIC
#include "stb_truetype.h"

#define ASCII_FIRST_CHAR 32 // ascii 0 to 31 are special characters
#define CHAR_DATA_LEN 95 // ASCII 32..126 is 95 glyphs
#define FONT_BITMAP_SIZE 1024 // 512 for speed?

using namespace std;

font_module::FontData font_module::loadFont(const char* filename, float pixelHeight) {
    FontData fontData;
    FILE* fontFile = fopen(filename, "rb");
    if (fontFile) {
        vector<uint8_t> buffer (1 << 20);
        size_t bytesRead = fread(buffer.data(), sizeof(uint8_t), buffer.size(), fontFile);
        fclose(fontFile);
        if (!bytesRead) return fontData;
        fontData.charData.resize(CHAR_DATA_LEN);
        fontData.bitmap.resize(FONT_BITMAP_SIZE * FONT_BITMAP_SIZE);
        stbtt_BakeFontBitmap(buffer.data(), 0, pixelHeight,
                             fontData.bitmap.data(), FONT_BITMAP_SIZE, FONT_BITMAP_SIZE,
                             ASCII_FIRST_CHAR, CHAR_DATA_LEN,
                             reinterpret_cast<stbtt_bakedchar*>(fontData.charData.data())); // no guarantee this fits!
        fontData.width = FONT_BITMAP_SIZE;
        fontData.height = FONT_BITMAP_SIZE;
    }
    else {
        cout << "[font_module::loadFont] could not open font file: " << filename << endl;
    }
    return fontData;
}

// mod ver of stbtt_GetBakedQuad
// assumes opengl use (no d3d)
static void GetBakedChar(const stbtt_bakedchar* bakedchars, int pw, int ph,
                         int char_index, font_module::CharData* c)
{
   float ipw = 1.0f / pw, iph = 1.0f / ph;
   const stbtt_bakedchar* b = bakedchars + char_index;
   int round_x = STBTT_ifloor(b->xoff + 0.5f);
   int round_y = STBTT_ifloor(b->yoff + 0.5f);

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

font_module::CharData font_module::getCharData(const FontData& fontData, int charIndex) {
    font_module::CharData charData;
    GetBakedChar(reinterpret_cast<const stbtt_bakedchar*>(fontData.charData.data()),
                 FONT_BITMAP_SIZE, FONT_BITMAP_SIZE,
                 charIndex - ASCII_FIRST_CHAR, &charData);
    return charData;
}

