#ifndef INCLUDE_AL_GRAPHICS_BUFFEROBJECT_HPP
#define INCLUDE_AL_GRAPHICS_BUFFEROBJECT_HPP

/*  Allocore --
  Multimedia / virtual environment application class library

  Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology, UCSB.
  Copyright (C) 2012. The Regents of the University of California.
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

    Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.

    Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

    Neither the name of the University of California nor the names of its
    contributors may be used to endorse or promote products derived from
    this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.


  File description:
  GPU buffer object helper

  File author(s):
  Lance Putnam, 2010, putnam.lance@gmail.com
  Keehong Youn, 2017, younkeehong@gmail.com

*/

#include "al/graphics/al_GPUObject.hpp"
#include "al/graphics/al_OpenGL.hpp"

#include <vector>
#include <cstddef>

namespace al {

/**
Vertex Buffer Objects (VBOs) create buffer memory for vertex attributes in
high-performance memory (in contrast, Vertex Arrays store buffer memory in the
client CPU, incurring the overhead of data transfer). If the buffer object is
used to store pixel data, it is called Pixel Buffer Object (PBO).

VBOs provide an interface to access these buffers in a similar fashion to vertex
arrays. Hints of 'target' and 'mode' help the implementation determine whether
to use system, AGP or video memory.

Unlike display lists, the data in vertex buffer object can be read and updated
by mapping the buffer into client's memory space.

Another important advantage of VBO is sharing the buffer objects with many
clients, like display lists and textures. Since VBO is on the server's side,
multiple clients will be able to access the same buffer with the corresponding
identifier.

 @ingroup Graphics
*/

class BufferObject : public GPUObject {
 public:
  BufferObject();
  virtual ~BufferObject();

  /// Get buffer store size, in bytes
  int size() const;
  /// Set buffer type
  void bufferType(unsigned int v);  // GL_ARRAY_BUFFER, ...
  /// Set buffer usage
  void usage(unsigned int v);  // GL_DYNAMIC_DRAW, ...

  /// Bind buffer
  void bind();
  /// Unbind buffer
  void unbind() const;

  // size is in bytes, if a std::vector of vec4 with float type is to be sent,
  // the call should be:
  // my_buffer.data(my_vector.size() * 4 * sizeof(float), my_vector.data());
  void data(size_t size, void const* src = NULL);
  void subdata(int offset, int size, void const* src);

  // #ifdef AL_GRAPHICS_USE_OPENGL
  /* Warning: these are not supported in OpenGL ES */

  /// Set map mode
  // void mapMode(unsigned int v);

  /// Map data store to client address space

  /// If successful, returns a valid pointer to the data, otherwise, it returns
  /// NULL.
  /// After using the pointer, call unmap() as soon as possible
  // void* map();

  /// Map data store to client address space

  /// If successful, returns true and sets argument to address of data,
  /// otherwise, returns false and leaves argument unaffected.
  /// After using the pointer, call unmap() as soon as possible
  // bool map(void*& buf);

  /// Unmaps data store from client address
  /// After unmap(), the mapped pointer is invalid
  // bool unmap();
  // #endif

 protected:
  unsigned int mType;
  unsigned int mUsage;
  // unsigned int mMapMode;
  size_t mSize;
  virtual void onCreate();
  virtual void onDestroy();
};

}  // namespace al
#endif
