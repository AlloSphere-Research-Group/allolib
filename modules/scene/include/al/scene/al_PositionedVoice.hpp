
#ifndef AL_POSITIONEDVOICE_HPP
#define AL_POSITIONEDVOICE_HPP

/*	Allocore --
        Multimedia / virtual environment application class library

   Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology,
   UCSB. Copyright (C) 2012-2022. The Regents of the University of California.
        All rights reserved.

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
        Dynamic Scene Voice

        File author(s):
        Andrés Cabrera mantaraya36@gmail.com
*/

#include "al/math/al_Vec.hpp"
#include "al/scene/al_SynthSequencer.hpp"
#include "al/sound/al_Speaker.hpp"
#include "al/sound/al_StereoPanner.hpp"
#include "al/spatial/al_DistAtten.hpp"
#include "al/spatial/al_Pose.hpp"

namespace al {

/**
 * @brief A PositionedVoice is a rendering class that can have a position and
 * size.
 * @ingroup Scene
 *
 */
class PositionedVoice : public SynthVoice {
public:
  const Pose pose() { return mPose.get(); }

  float size() { return mSize.get(); }

  void setPose(Pose pose) { mPose.set(pose); }

  void setSize(float size) { mSize.set(size); }

  ParameterPose &parameterPose() { return mPose; }

  Parameter &parameterSize() { return mSize; }

  bool useDistanceAttenuation() { return mUseDistAtten; }
  void useDistanceAttenuation(bool atten) { mUseDistAtten = atten; }

  std::vector<Vec3f> &audioOutOffsets() { return mAudioOutPositionOffsets; }

  /**
   * @brief Test whether voice is primary or a replica
   *
   * Replica voices should not perform certain update computation, and should
   * get state and values from primary node. This test allows writing code
   * specific to one or the other type of voices. Only really important if using
   * DistributedScene.
   */
  bool isPrimary() { return !mIsReplica; }

  /**
   * @brief Set the position offset for each of the audio outputs for this voice
   * @param offsets The size of offsets must be equal to the number of outputs
   */
  void audioOutOffsets(const std::vector<Vec3f> &offsets) {
    mAudioOutPositionOffsets = offsets;
  }

  /**
   * @brief Override this function to apply transformations after the internal
   * transformations of the voice has been applied
   */
  virtual void preProcess(Graphics & /*g*/) {}

  /**
   * @brief For PositionedVoice, the pose (7 floats) and the size are appended
   * to the pfields
   */
  virtual bool setTriggerParams(float *pFields, int numFields = -1) override;

  /**
   * @brief Set parameter values
   * @param pFields std::vector<float> containing the values
   * @return true if able to set the fields
   */
  virtual bool setTriggerParams(const std::vector<float> &pFields,
                                bool noCalls = false) override;

  /**
   * @brief Set parameter values
   * @param pFields std::vector<float> containing the values
   * @return true if able to set the fields
   */
  virtual bool setTriggerParams(const std::vector<VariantValue> &pFields,
                                bool noCalls = false) override;

  /**
   * @brief For PositionedVoice, the pose (7 floats) and the size are appended
   * to the pfields
   */
  virtual std::vector<VariantValue> getTriggerParams() override {
    std::vector<VariantValue> pFields = SynthVoice::getTriggerParams();
    pFields.reserve(pFields.size() + 8);
    Pose currentPose = pose();
    pFields.insert(pFields.end(), currentPose.vec().begin(),
                   currentPose.vec().end());

    auto *comps = currentPose.quat().components;

    pFields.push_back(*comps++);
    pFields.push_back(*comps++);
    pFields.push_back(*comps++);
    pFields.push_back(*comps);

    pFields.push_back(mSize.get());
    return pFields;
  }

  /**
   * @brief Apply translation, rotation and scaling for this PositionedVoice
   * @param g
   */
  virtual void applyTransformations(Graphics &g);

protected:
  ParameterPose mPose{"_pose"};
  Parameter mSize{"_size", "", 1.0};
  //    ParameterPose mPose {"_pose"};
  //    Parameter mSize {"_size", "", 1.0};
  std::vector<Vec3f>
      mAudioOutPositionOffsets; // This vector is added to the voice's position
                                // to determine the specific position of the
                                // audio out

  bool mUseDistAtten{true};
};

} // namespace al

#endif // AL_POSITIONEDVOICE_HPP
