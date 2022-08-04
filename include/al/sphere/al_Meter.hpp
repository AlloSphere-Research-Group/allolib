#ifndef METER_HPP
#define METER_HPP
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
  Spatial audio metering

  File author(s):
  Andres Cabrera, 2022. mantaraya36@gmail.com
*/

#include "al/graphics/al_Graphics.hpp"
#include "al/graphics/al_Mesh.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/io/al_AudioIOData.hpp"
#include "al/sound/al_Speaker.hpp"

using namespace al;

/**
 * @brief The Meter class simplifies displaying metering of speakers in space
 *
 * You can use this in a distributed system by calculating audio levels on
 * the audio renderer, and then sending the values to renderer nodes.
 *
 * @code

struct SharedState {
    float meterValues[64] = {0};
};

// On initilization
auto sl = al::AlloSphereSpeakerLayout();
mSpatializer = scene.setSpatializer<Lbap>(sl);
mMeter.init(mSpatializer->speakerLayout()); // Requires graphics context to be
available

// On the audio context
    mMeter.processSound(io);

// On graphics context
mMeter.draw(g);

// On simulation/state context
if (isPrimary()) {
  auto &values = mMeter.getMeterValues();
  assert(values.size() < 65);
  memcpy(state().meterValues, values.data(), values.size() * sizeof(float));
} else {
  mMeter.setMeterValues(state().meterValues, 64);
}
@endcode
 *
 */
class Meter {
public:
  /**
   * @brief Call init() to initialize passing the speaker layout.
   * @param sl The speaker layout
   */
  void init(const Speakers &sl);
  /**
   * @brief Call this function on every audio callback
   * @param io the audio IO data
   */
  void processSound(AudioIOData &io);
  /**
   * @brief draw a marker whose size reflects level at the right position.
   * @param g The Graphics object
   */
  void draw(Graphics &g);

  /**
   * @brief get current meter values.
   * @return the meter values
   *
   * Useful to share the values measured to render nodes
   */
  const std::vector<float> &getMeterValues() { return values; }

  /**
   * @brief set meter values to display
   * @param newValues
   * @param count
   *
   * Set from a shared state on renderer nodes.
   */
  void setMeterValues(float *newValues, size_t count);

private:
  Mesh mMesh;
  std::vector<float> values;
  std::vector<float> tempValues;
  Speakers mSl;
};

#endif // METER_HPP
