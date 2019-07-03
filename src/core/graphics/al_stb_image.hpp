#ifndef INCLUDE_AL_STB_IMAGE_HPP
#define INCLUDE_AL_STB_IMAGE_HPP

struct al_stbImageData
{
    int width;
    int height;
    unsigned char* data;
};

// returns empty object (width and height 0, data nullptr) if failed to load
al_stbImageData al_stbLoadImage (const char* filename);
void al_stbFreeImage (al_stbImageData* img);

#endif
