#ifndef INCLUDE_AL_STB_IMAGE_HPP
#define INCLUDE_AL_STB_IMAGE_HPP

struct al_stbImageData {
  int width;
  int height;
  unsigned char *data;
};

// returns empty object (width and height 0, data nullptr) if failed to load
al_stbImageData al_stbLoadImage(const char *filename);
void al_stbFreeImage(al_stbImageData *img);

bool al_stbWriteImage(const char *fileName, const unsigned char *data,
                      int width, int height, int numComponents = 3);

void al_stbSetFlipVertically(bool set);

#endif
