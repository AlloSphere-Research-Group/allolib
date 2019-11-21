#ifndef INCLUDE_AL_PANNING_LBAP
#define INCLUDE_AL_PANNING_LBAP

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
        Layer Based Amplitude Panning

        File author(s):
        Andres Cabrera 2018 mantaraya36@gmail.com
*/

#include <map>
#include <memory>

#include "al/math/al_Vec.hpp"
#include "al/sound/al_Speaker.hpp"
#include "al/sound/al_Vbap.hpp"
#include "al/spatial/al_DistAtten.hpp"
#include "al/spatial/al_Pose.hpp"

#define RAD_2_DEG_SCALE 57.29577951308232  // 360/(2*pi)

namespace al {

class LdapRing {
 public:
  LdapRing(Speakers &sl) {
    vbap = std::make_shared<Vbap>(sl);
    elevation = 0;
    for (auto speaker : sl) {
      elevation += speaker.elevation;
    }
    elevation /= sl.size();  // store average elevation
    vbap->compile();
  }

  std::shared_ptr<Vbap> vbap;
  float elevation;
};

/// Layer-based amplitude panner
///
/// @ingroup Sound
class Lbap : public Spatializer {
 public:
  typedef enum {
    KEEP_SAME_ELEVATION =
        0x1,  // Don't discard triplets that have the same elevation
  } VbapOptions;

  /// @param[in] sl	A speaker layout
  Lbap(const Speakers &sl) : Spatializer(sl) {}

  virtual ~Lbap() override {
    if (buffer) {
      free(buffer);
    }
  }

  void compile() override;

  void prepare(AudioIOData &io) override;

  void renderSample(AudioIOData &io, const Pose &reldir, const float &sample,
                    const unsigned int &frameIndex) override;

  void renderBuffer(AudioIOData &io, const Pose &listeningPose,
                    const float *samples,
                    const unsigned int &numFrames) override;

  void print(std::ostream &stream = std::cout) override;

 private:
  std::vector<LdapRing> mRings;
  float *buffer{nullptr};  // Two consecutive buffers (non-interleaved)
  int bufferSize{0};
};

}  // namespace al
#endif
