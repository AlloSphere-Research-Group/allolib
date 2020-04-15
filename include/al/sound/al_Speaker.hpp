#ifndef INCLUDE_AL_SOUND_SPEAKER_HPP
#define INCLUDE_AL_SOUND_SPEAKER_HPP

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

                Redistributions in binary form must reproduce the above
   copyright notice, this list of conditions and the following disclaimer in the
                documentation and/or other materials provided with the
   distribution.

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
        Abstraction of a loudspeaker used for sound spatialization algorithms

        File author(s):
        Lance Putnam, 2006, putnam.lance@gmail.com
*/

#include <cmath>
#include <vector>

#include "al/math/al_Constants.hpp"
#include "al/math/al_Vec.hpp"

namespace al {

/// Spatial definition of a speaker in a listening space
///
/// @ingroup Sound
class Speaker {
public:
  unsigned int deviceChannel; ///< Index in the output device channels array
  float azimuth;              ///< Angle from forward to right vector (i.e. CW)
  float elevation; ///< Angle from forward-right plane to up vector (0 is
                   ///< "horizon")
  int group;       ///< Group identifier
  float radius;    ///< Distance from center of listening space
  float gain;      ///< Gain of speaker

  /// @param[in] deviceChan		audio device output channel
  /// @param[in] az				azimuth of speaker
  /// @param[in] el				elevation of speaker
  /// @param[in] radius			radius of speaker
  /// @param[in] gain				gain of speaker
  Speaker(unsigned int deviceChan = 0, float az = 0.f, float el = 0.f,
          int group = 0, float radius = 1.f, float gain = 1.f);

  /// Get position in Cartesian coordinate (in audio space)
  template <class T> Speaker &posCart(T *xyz) {
    using namespace std;
    float elr = toRad(elevation);
    float azr = toRad(azimuth);
    float cosel = cos(elr);
    xyz[0] = -cos(azr) * cosel * radius;
    xyz[1] = sin(azr) * cosel * radius;
    xyz[2] = sin(elr) * radius;
    return *this;
  }

  void posCart2(Vec3d xyz);

  /// Get position as Cartesian coordinate (in audio space)
  Vec3d vec() const;

  /// Get position as Cartesian coordinate (in graphics space)
  Vec3d vecGraphics() const;

  static double toRad(double d) { return d * M_PI / 180.; }
  static float toRad(float d) { return d * float(M_PI) / 180.f; }
};

/// A set of speakers
typedef std::vector<Speaker> Speakers;

/// Generic layout of N speakers spaced equidistantly in a ring
///
/// @ingroup Sound
///
/// @param[in] deviceChannelStart	starting index of device channel
/// @param[in] phase				starting phase of first speaker,
/// in degrees
/// @param[in] radius				radius of all speakers
/// @param[in] gain					gain of all speakers
template <int N>
Speakers SpeakerRingLayout(unsigned int deviceChannelStart = 0,
                           float phase = 0.f, float radius = 1.f,
                           float gain = 1.f) {
  Speakers mSpeakers;
  mSpeakers.reserve(N);
  for (unsigned int i = 0; i < N; ++i) {
    mSpeakers.emplace_back(Speaker(
        i + deviceChannelStart, (360.f / N) * i + phase, 0.f, 0, radius, gain));
  }
  return mSpeakers;
};

/// Headset speaker layout
///
/// @ingroup allocore
Speakers HeadsetSpeakerLayout(unsigned int deviceChannelStart = 0,
                              float radius = 1.f, float gain = 1.f);

/// Stereo speaker layout
///
/// @ingroup Sound
Speakers StereoSpeakerLayout(unsigned int deviceChannelStart = 0,
                             float angle = 30.f, float distance = 1.f,
                             float gain = 1.f);

/// Octophonic ring speaker layout
///
/// @ingroup Sound
Speakers OctalSpeakerLayout(unsigned int deviceChannelStart = 0,
                            float phase = 0.f, float radius = 1.f,
                            float gain = 1.f);

/// Generic layout of 8 speakers arranged in a cube with listener in the middle
///
/// @ingroup allocore
Speakers CubeLayout(unsigned int deviceChannelStart = 0);

} // namespace al
#endif
