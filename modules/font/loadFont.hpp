#ifndef INCLUDE_LOAD_FONT_MODULE_HPP
#define INCLUDE_LOAD_FONT_MODULE_HPP

#include <vector>
#include <cstdint>

namespace font_module {

struct FontData {
    struct BakedChar {
        unsigned short x0, y0, x1, y1;
        float xoff, yoff, xadvance;
    }; // to be casted to stbtt_backedchar in the implementation
       // user will not need to use this struct

    float pixelHeight;
    int width, height; // of bitmap pixel data
    std::vector<uint8_t> bitmap; // 1 channel bitmap data
    std::vector<BakedChar> charData;
};

struct CharData {
    float x0, y0, x1, y1; // packing rect corners, x towards right, y towards down
    float s0, t0, s1, t1; // texcoords
    float xAdvance;       // how much to go horizontally after this character
};

FontData loadFont(const char* filename, float pixelHeight);

// caching the results of this function might give better performance
// x0, y0, x1, y1, and xAdvance of returned CharData are in fontData.pixelHeight scale
CharData getCharData(const FontData& fontData, int charIndex);

}

#endif
