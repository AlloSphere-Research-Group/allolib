#ifndef AL_VOICE_ENTITY_HPP
#define AL_VOICE_ENTITY_HPP

/**
 * @file al_VoiceEntity.hpp
 * @brief Bridge SynthVoice parameter lists into Entity / ParamSet / Scene.
 *
 * Compatibility path while SynthVoice / PolySynth remain: expose voice params
 * under an Entity path so ParamState keyframes and Scene capture work without
 * rewriting the voice.
 */

#include <string>

#include "al/parameter/al_Entity.hpp"
#include "al/parameter/al_ParamState.hpp"
#include "al/parameter/al_Scene.hpp"

namespace al {

class SynthVoice;

/// Add trigger + continuous parameters from \p voice onto \p entity.
void bindVoiceParams(Entity &entity, SynthVoice &voice);

/**
 * @brief Create an entity in \p scene and bind \p voice's parameters.
 * @return reference to the new entity
 */
Entity &bindVoiceEntity(Scene &scene, SynthVoice &voice,
                        std::string entityName = "");

ParamState captureVoiceState(SynthVoice &voice);
void applyVoiceState(SynthVoice &voice, const ParamState &state);

} // namespace al

#endif
