#include "al/graphics/al_Image.hpp"

#include <cstring>

#include "al_stb_image.hpp"

namespace al {

/*
struct al_StbImageData
{
    int width;
    int height;
    unsigned char* data;
};

// returns empty object (width and height 0, data nullptr) if failed to load
al_StbImageData al_StbLoadImage (const char* filename);
void al_StbFreeImage (al_StbImageData* img);
*/

Image::Image(const std::string &filename) { load(filename); }

bool Image::load(const std::string &filename) {
  al_stbImageData image_data = al_stbLoadImage(filename.c_str());
  if (!image_data.data)
    return false;

  int arr_size = 4 * image_data.width * image_data.height;
  mWidth = image_data.width;
  mHeight = image_data.height;
  mArray.resize(arr_size);
  static_assert(
      sizeof(unsigned char) == sizeof(uint8_t),
      "size of stb image's unsigned char and size of uint8_t is different");
  std::memcpy(mArray.data(), image_data.data, sizeof(unsigned char) * arr_size);
  al_stbFreeImage(&image_data);

  mLoaded = true;
  mFilename = filename;
  return true;
}

bool Image::save(const std::string &filename) {
  // TODO
  if (mLoaded)
    mFilename = filename;
  return mLoaded; // why?
}

bool Image::saveImage(std::string fileName, unsigned char *pixs, int width,
                      int height, bool flipVertically, int numComponents) {

  al_stbSetFlipVertically(flipVertically);
  return al_stbWriteImage(fileName.c_str(), pixs, width, height, numComponents);
}

} // namespace al
