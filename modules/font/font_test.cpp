#include <vector>
#include <cstdint>

namespace font_module {

struct BakedChar {
    unsigned short x0, y0, x1, y1;
    float xoff, yoff, xadvance;
}; // alias for stbtt_backedchar, to be casted in the implementation

struct FontData {
    std::vector<uint8_t> bitmap;
    std::vector<BakedChar> charData;
};

struct CharData {
    float width, height;
    float s0, t0, s1, t1;
};

FontData loadFont(const char* filename, float pixelHeight);
CharData getCharTexcoord(const FontData& fontData, int charIndex);

}

#include "al/core.hpp"
#include <iostream>

using namespace al;
using namespace std;

int main() {
    auto font = font_module::loadFont("data/font.ttf", 18);
    cout << "font loaded? " << font.bitmap.size() << endl;
    auto c = font_module::getCharTexcoord(font, 'a');
    cout << c.width << ", " << c.height << endl;
    cout << '(' << c.s0 << ',' << c.t0 << ')' << endl;
    cout << '(' << c.s1 << ',' << c.t1 << ')' << endl;
}


#define STB_TRUETYPE_IMPLEMENTATION  // force following include to generate implementation
#define STBTT_STATIC
#include "stb_truetype.h"

#define ASCII_FIRST_CHAR 32 // ascii 0 to 31 are special characters
#define CHAR_DATA_LEN 96 // ASCII 32..126 is 95 glyphs
#define FONT_BITMAP_SIZE 512

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
    }
    else {
        cout << "could not open font file" << endl;
    }
    return fontData;
}

font_module::CharData font_module::getCharTexcoord(const FontData& fontData, int charIndex) {
    float x = 0, y = 0; // dummy
    stbtt_aligned_quad q;
    stbtt_GetBakedQuad(reinterpret_cast<const stbtt_bakedchar*>(fontData.charData.data()),
                       FONT_BITMAP_SIZE, FONT_BITMAP_SIZE,
                       charIndex - ASCII_FIRST_CHAR, &x, &y, &q, 1); // 1=opengl & d3d10+, 0=d3d9
    CharData charData;
    charData.width = q.x1 - q.x0;
    charData.height = q.y1 - q.y0;
    charData.s0 = q.s0;
    charData.t0 = q.t0;
    charData.s1 = q.s1;
    charData.t1 = q.t1;
    
    return charData;
}

