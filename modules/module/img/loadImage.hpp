#ifndef INLCUDE_LOAD_IMAGE_HPP
#define INLCUDE_LOAD_IMAGE_HPP

/*
    Description:
      Portable image loading module

    Build:
      Compile loadImage.cpp, include loadImage.hpp
      No dependencies needed, no compile definitions needed
*/

#include <vector>
#include <cstdint>
#include <string>

namespace img_module {

/*
    Always loads image as 8-bit 4 channels RGBA
    Supported formats: PNG, JPG, ...
    Implementation uses stb_image.h >> https://github.com/nothings/stb
*/

struct ImageData {
    int width = 0;
    int height = 0;
    std::vector<uint8_t> data;
};

// returns empty object (width and height 0, data empty) if failed to load
ImageData loadImage(const char* filename);

// returns empty object (width and height 0, data empty) if failed to load
ImageData loadImage(std::string &filename);

}

namespace imgModule = img_module;

#endif
