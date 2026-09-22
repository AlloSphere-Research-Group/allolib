#include "al/scene/al_VoiceEntity.hpp"

#include "al/scene/al_SynthVoice.hpp"

namespace al {

void bindVoiceParams(Entity &entity, SynthVoice &voice) {
  for (ParameterMeta *p : voice.triggerParameters()) {
    if (p) {
      entity.addParam(*p);
    }
  }
  for (ParameterMeta *p : voice.parameters()) {
    if (p) {
      entity.addParam(*p);
    }
  }
}

Entity &bindVoiceEntity(Scene &scene, SynthVoice &voice,
                        std::string entityName) {
  if (entityName.empty()) {
    entityName = "voice";
  }
  Entity &entity = scene.create(std::move(entityName));
  bindVoiceParams(entity, voice);
  return entity;
}

ParamState captureVoiceState(SynthVoice &voice) {
  std::vector<ParameterMeta *> all = voice.triggerParameters();
  auto continuous = voice.parameters();
  all.insert(all.end(), continuous.begin(), continuous.end());
  return captureParamState(all);
}

void applyVoiceState(SynthVoice &voice, const ParamState &state) {
  std::vector<ParameterMeta *> all = voice.triggerParameters();
  auto continuous = voice.parameters();
  all.insert(all.end(), continuous.begin(), continuous.end());
  applyParamState(state, all);
}

} // namespace al
