#include "al/sound/al_Spatializer.hpp"

using namespace  al;

Spatializer::Spatializer(const SpeakerLayout& sl)
{
    size_t numSpeakers = sl.speakers().size();
    for(unsigned i=0;i<numSpeakers;++i){
        mSpeakers.push_back(sl.speakers()[i]);
    }
}
