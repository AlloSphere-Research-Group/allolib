#ifndef AL_SPATIALIZER_H
#define AL_SPATIALIZER_H

/*	Allocore --
    Multimedia / virtual environment application class library

    Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology,
   UCSB. Copyright (C) 2012-2015. The Regents of the University of California.
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
    Spatializer base class
    File author(s):
    Lance Putnam, 2010, putnam.lance@gmail.com
    Graham Wakefield, 2010, grrrwaaa@gmail.com
    Ryan McGee, 2012, ryanmichaelmcgee@gmail.com
*/

#include <iostream>

#include "al/io/al_AudioIOData.hpp"
#include "al/sound/al_Speaker.hpp"
#include "al/spatial/al_Pose.hpp"

namespace al {

/// Abstract class for all spatializers: Ambisonics, DBAP, VBAP, etc.
///
/// @ingroup Sound
class Spatializer {
public:
  /// @param[in] sl	A speaker layout to use
  Spatializer(const Speakers &sl);

  virtual ~Spatializer() {}

  /// Perform any necessary updates when the speaker layout changes, ex. new
  /// speaker triplets for VBAP Must be called before any calls to prepare(),
  /// renderBuffer(), renderSample() or perform()
  virtual void compile() {}

  /// Called once per listener, before sources are rendered. ex. zero ambisonics
  /// coefficients
  virtual void prepare(AudioIOData &io) {}

  /// Render audio buffer in position
  virtual void renderBuffer(AudioIOData &io, const Vec3f &pos,
                            const float *samples,
                            const unsigned int &numFrames) = 0;

  /// Render audio sample in position
  virtual void renderSample(AudioIOData &io, const Vec3f &pos,
                            const float &sample,
                            const unsigned int &frameIndex) = 0;

  /// Called once per listener, after sources are rendered. ex. ambisonics
  /// decode
  virtual void finalize(AudioIOData &io) {}

  /// Print out information about spatializer
  virtual void print(std::ostream &stream = std::cout) {}

  /// Get number of speakers
  int numSpeakers() const { return int(mSpeakers.size()); }

  // Get speaker layout
  const Speakers &speakerLayout() { return mSpeakers; }

  /// Set number of frames
  virtual void numFrames(unsigned int v) { mNumFrames = v; }

protected:
  Speakers mSpeakers;

  std::vector<float> mBuffer; // temporary frame buffer
  unsigned int mNumFrames{0};
};

} // namespace al

#endif
