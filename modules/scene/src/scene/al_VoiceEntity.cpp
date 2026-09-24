#include "al/scene/al_VoiceEntity.hpp"

#include "al/scene/al_SynthVoice.hpp"

namespace al {

void bindVoiceParams(ParamEntity &entity, SynthVoice &voice) {
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

ParamEntity &bindVoiceEntity(World &world, SynthVoice &voice,
                             std::string entityName) {
  if (entityName.empty()) {
    entityName = "voice";
  }
  ParamEntity &entity = world.create(std::move(entityName));
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
