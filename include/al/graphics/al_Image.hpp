#ifndef INCLUDE_AL_IMAGE_HPP
#define INCLUDE_AL_IMAGE_HPP

/*	Allocore --
        Multimedia / virtual environment application class library

        Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology,
   UCSB. Copyright (C) 2012. The Regents of the University of California. All
   rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:

   Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

   Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

   Neither the name of the University of California nor the names
   of its contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
   IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
   PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
   OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
   OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
   ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


   File description:
    Loads and saves images

   File author(s):
    Graham Wakefield, 2010, grrrwaaa@gmail.com
    Keehong Youn, 2019, younkeehong@gmail.com
*/

#include <cstdint>
#include <string>
#include <vector>

namespace al {

/**
@brief Interface for loading image files

@ingroup Graphics

Loads image as 8-bit 4 channels RGBA. Supported formats: PNG, JPG, ...
Implementation uses stb_image.h >> https://github.com/nothings/stb
*/

struct Image {
  struct RGBAPix {
    uint8_t r, g, b, a;
  };

  unsigned int mWidth = 0, mHeight = 0;
  std::vector<uint8_t> mArray;
  std::string mFilename = "";
  int mCompression = 50;
  bool mLoaded = false;

  Image() = default;
  Image(const Image &) = default;
  Image(Image &&) = default;
  Image &operator=(const Image &) = default;
  Image &operator=(Image &&) = default;
  ~Image() = default;

  Image(const std::string &filename);

  /// Load image from disk

  /// @param[in] filePath		File to load. Image type determined by
  /// file
  ///							extension.
  /// \returns true for success or print error message and return false
  bool load(const std::string &filePath);

  /// Save image to disk

  /// @param[in] filePath		File to save. Image type determined by
  /// file
  ///							extension.
  /// \returns true for success or print error message and return false
  bool save(const std::string &filePath);

  /// File path to image
  const std::string &filepath() const { return mFilename; }

  /// Whether image was loaded from file
  bool loaded() const { return mLoaded; }

  /// Get pixels as an Array
  std::vector<uint8_t> &array() { return mArray; }

  /// Get pixels as an Array (read-only)
  const std::vector<uint8_t> &array() const { return mArray; }

  /// Get pointer to pixels
  template <typename T = RGBAPix> T *pixels() { return (T *)(mArray.data()); }

  /// Get pointer to pixels (read-only)
  template <typename T = RGBAPix> const T *pixels() const {
    return (const T *)(mArray.data());
  }

  /**
   * @brief saveImage
   * @param fileName
   * @param pixs
   * @param width
   * @param height
   * @param flipVertically
   * @param numComponents use 3 for RGB and 4 for RGBA
   * @return
   */
  static bool saveImage(std::string fileName, unsigned char *pixs, int width,
                        int height, bool flipVertically = false,
                        int numComponents = 3);

  /// Get number of bytes per pixel
  // unsigned bytesPerPixel() const { return allo_type_size(array().type()) *
  // array().components(); }

  /// Get pixel format
  // Format format() const;

  /// Get width, in pixels
  unsigned width() const { return mWidth; }

  /// Get height, in pixels
  unsigned height() const { return mHeight; }

  /// Get compression flags for saving
  int compression() const { return mCompression; }

  /// Set compression flags for saving

  /// The flags consist of a bitwise-or of the level of compression in [0,100]
  /// and other flags which may be specific to the image format.
  Image &compression(int flags) {
    mCompression = flags;
    return *this;
  }

  /// Get read-only reference to a pixel

  /// Warning: doesn't check that Pix has matching type/component count.
  /// Warning: no bounds checking performed on x and y.
  template <typename Pix = RGBAPix>
  const Pix &at(unsigned x, unsigned y) const {
    auto pixel_index = x + y * width();
    return *reinterpret_cast<const RGBAPix *>(mArray.data() + 4 * pixel_index);
    // return *array().cell<Pix>(x, y);
  }

  /// Get mutable reference to a pixel

  /// Warning: doesn't check that Pix has matching type/component count.
  /// Warning: no bounds checking performed on x and y.
  template <typename Pix = RGBAPix> Pix &at(unsigned x, unsigned y) {
    auto pixel_index = x + y * width();
    return *reinterpret_cast<RGBAPix *>(mArray.data() + 4 * pixel_index);
    // return *array().cell<Pix>(x, y);
  }

  /// Write a pixel to an Image

  /// Warning: doesn't check that Pix has matching type/component count
  /// Warning: no bounds checking performed on x and y
  template <typename Pix = RGBAPix>
  void write(const Pix &pix, unsigned x, unsigned y) {
    auto pixel_index = x + y * width();
    mArray[4 * pixel_index + 0] = pix.r;
    mArray[4 * pixel_index + 1] = pix.g;
    mArray[4 * pixel_index + 2] = pix.b;
    mArray[4 * pixel_index + 3] = pix.a;
    // array().write(&pix.r, x, y);
  }

  /// Read a pixel from an Image

  /// Warning: doesn't check that Pix has matching type/component count
  /// Warning: no bounds checking performed on x and y
  template <typename Pix = RGBAPix>
  void read(Pix &pix, unsigned x, unsigned y) const {
    auto pixel_index = x + y * width();
    pix.r = mArray[4 * pixel_index + 0];
    pix.g = mArray[4 * pixel_index + 1];
    pix.b = mArray[4 * pixel_index + 2];
    pix.a = mArray[4 * pixel_index + 3];
    // array().read(&pix.r, x, y);
  }

  /// Resize internal pixel buffer. Erases any existing data.

  /// @param[in] dimX		number of pixels in x direction
  /// @param[in] dimY		number of pixels in y direction
  /// @param[in] format	pixel color format
  /// \returns True on success; false otherwise.
  template <typename T> bool resize(int dimX, int dimY /*, Format format */) {
    mArray.resize(4 * dimX * dimY);
    // mArray.formatAligned(components(format), Array::type<T>(), dimX, dimY,
    // 1);
    return true;
  }

  // void sendToTexture(Texture& tex, Texture::Internal i, Texture::Format f,
  // Texture::DataType d) {
  //    tex.create2D(width(), height(), i, f, d);
  //    tex.submit(pixels<uint8_t>(), f, Texture::UBYTE);
  //}

  // void sendToTexture(Texture& tex) {
  //    auto to_texture_internalformat = [](Format f) {
  //        switch (f) {
  //            case LUMINANCE: return Texture::R8;
  //            case LUMALPHA: return Texture::RG8;
  //            case RGB: return Texture::RGB8;
  //            case RGBA: return Texture::RGBA8;
  //            default: return Texture::RGBA8;
  //        }
  //    };
  //    auto to_texture_format = [](Format f) {
  //        switch (f) {
  //            case LUMINANCE: return Texture::RED;
  //            case LUMALPHA: return Texture::RG;
  //            case RGB: return Texture::RGB;
  //            case RGBA: return Texture::RGBA;
  //            default: return Texture::RGBA;
  //        }
  //    };
  //    sendToTexture(tex, to_texture_internalformat(format()),
  //    to_texture_format(format()), Texture::UBYTE);
  //}

  /// Get number of components per pixel element
  // static int components(Format v);

  // static Format getFormat(int planes);

  // class Impl {
  // public:
  //	virtual ~Impl() {};
  //	virtual bool load(const std::string& filename, Array& lat) = 0;
  //	virtual bool save(const std::string& filename, const Array& lat, int
  // compressFlags) = 0;
  //};
};

} // namespace al
#endif
