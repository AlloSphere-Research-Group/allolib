#ifndef INCLUDE_LOAD_FONT_MODULE_HPP
#define INCLUDE_LOAD_FONT_MODULE_HPP

#include <vector>
#include <cstdint>

namespace al_stb {

// Wrapper for all the data needed for font rendering
struct FontData {
    struct BakedChar {
        unsigned short x0, y0, x1, y1;
        float xoff, yoff, xadvance;
    }; // to be casted to stbtt_bakedchar in the implementation
       // user will not need to use this struct

    float pixelHeight = -1;
    int width = -1, height = -1; // of bitmap pixel data
    std::vector<uint8_t> bitmap; // 1 channel bitmap data
    std::vector<BakedChar> charData;
};

// info about a chracter, calculated from font data
struct CharData {
    float x0, y0, x1, y1; // packing rect corners, x towards right, y towards down
    float s0, t0, s1, t1; // texcoords
    float xAdvance;       // how much to go horizontally after this character
};

// pixelHeight is max height of each character in font texture
// value larger than 128 might result not all fonts fitting in the texture
FontData loadFont(const char* filename, float pixelHeight, int bitmapSize);

// caching the results of this function might give better performance
// x0, y0, x1, y1, and xAdvance of returned CharData are in fontData.pixelHeight scale
CharData getCharData(FontData* fontData, int charIndex);

}

#endif
