#ifndef AL_ALLOSPHERE_SPEAKER_LAYOUT_H
#define AL_ALLOSPHERE_SPEAKER_LAYOUT_H

#include "al/sound/al_Speaker.hpp"

namespace al {

/// Current arrangement of speakers in AlloSphere
Speakers AlloSphereSpeakerLayout();
Speakers AlloSphereSpeakerLayoutThin();
Speakers AlloSphereSpeakerLayoutExtraThin();

Speakers AlloSphereSpeakerLayoutHorizontal();
Speakers AlloSphereSpeakerLayoutHorizontalThin();
Speakers AlloSphereSpeakerLayoutHorizontalExtraThin();

} // namespace al
#endif
