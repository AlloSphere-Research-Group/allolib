#ifndef AL_SPEAKERADJUSTMENT
#define AL_SPEAKERADJUSTMENT

#include <vector>

#include "al/io/al_AudioIOData.hpp"
#include "al/sound/al_Speaker.hpp"

namespace al {

/**
 * @brief Adjust the gain of speakers based on distance
 *
 * The function to determine gains is:
 * std::pow(speaker.radius, expon) / std::pow(min_distance, expon);
 *
 * where min_distance is the closest speaker distance. This way, the closest
 * loudspeaker reatins 1.0 gain.
 */
class SpeakerDistanceGainAdjustment {
 public:
  void configure(Speakers layout, double expon = 2.0);

  void processGains(AudioIOData& io);

 public:
  std::vector<float> mGains;
  Speakers mLayout;
};

/**
 * @brief This class is added for convenience to append it to AudioIO processing
 *
 * You will want to use it like:
 * @code
 * SpeakerDistanceGainAdjustmentProcessor gainAdjustment;
 * gainAdjustment.configure(speakerLayout, 2.0);
 * audioIO().append(gainAdjustment);
 * @endcode
 *
 * This will do the level processing after the AudioIO main callback function
 * has been processed. This will keep your audio processing function clean of
 * this optional post-processing
 */
class SpeakerDistanceGainAdjustmentProcessor
    : public AudioCallback,
      public SpeakerDistanceGainAdjustment {
 public:
  virtual void onAudioCB(AudioIOData& io) { this->processGains(io); }

 private:
  // Hide this function to users of this class
  using SpeakerDistanceGainAdjustment::processGains;
};

// FIXME implemement time adjustment for speaker distances
class SpeakerDistanceTimeAdjustment {
 public:
  //  void configure(Speakers layout, uint64_t framesPerBuffer);

  void processDelays(AudioIOData& io);

 public:
  //  std::vector<float> mGains;
  Speakers mLayout;
};

class SpeakerDistanceTimeAdjustmentProcessor : public AudioCallback,
                                               SpeakerDistanceTimeAdjustment {
 public:
  virtual void onAudioCB(AudioIOData& io) { this->processDelays(io); }

 private:
  // Hide this function to users of this class
  using SpeakerDistanceTimeAdjustment::processDelays;
};

}  // namespace al

#endif  // AL_SPEAKERADJUSTMENT
