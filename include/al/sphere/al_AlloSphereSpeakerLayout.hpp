#ifndef AL_ALLOSPHERE_SPEAKER_LAYOUT_H
#define AL_ALLOSPHERE_SPEAKER_LAYOUT_H

#include "al/sound/al_Speaker.hpp"

namespace al {

//Computed using teh notebook in repo:
// https://github.com/AlloSphere-Research-Group/allo_speakers

/// Current arrangement of speakers in AlloSphere
Speakers AlloSphereSpeakerLayoutCompensated();

/// Without gain compensation
/// Use previous (wrong) computation for distance/radius.
/// Kept for bakcward compatibility but should not be used
/// (The practical difference is small. Gain compensation has the most
/// significant perceptual effect)
[[deprecated]] Speakers AlloSphereSpeakerLayout();
[[deprecated]] Speakers AlloSphereSpeakerLayoutThin();
[[deprecated]] Speakers AlloSphereSpeakerLayoutExtraThin();

[[deprecated]] Speakers AlloSphereSpeakerLayoutHorizontal();
[[deprecated]] Speakers AlloSphereSpeakerLayoutHorizontalThin();
[[deprecated]] Speakers AlloSphereSpeakerLayoutHorizontalExtraThin();

} // namespace al
#endif
