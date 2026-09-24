#ifndef AL_VOICE_ENTITY_HPP
#define AL_VOICE_ENTITY_HPP

/**
 * @file al_VoiceEntity.hpp
 * @brief Bridge SynthVoice parameter lists into ParamEntity / World.
 */

#include <string>

#include "al/parameter/al_Entity.hpp"
#include "al/parameter/al_ParamState.hpp"
#include "al/parameter/al_Scene.hpp"

namespace al {

class SynthVoice;

void bindVoiceParams(ParamEntity &entity, SynthVoice &voice);

ParamEntity &bindVoiceEntity(World &world, SynthVoice &voice,
                             std::string entityName = "");

ParamState captureVoiceState(SynthVoice &voice);
void applyVoiceState(SynthVoice &voice, const ParamState &state);

} // namespace al

#endif
