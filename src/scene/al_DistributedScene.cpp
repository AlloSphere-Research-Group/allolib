#include "al/scene/al_DistributedScene.hpp"

using namespace al;

DistributedScene::DistributedScene(std::string name, int threadPoolSize,
                                   TimeMasterMode masterMode)
    : DynamicScene(threadPoolSize, masterMode) {
  mName = name;

  PolySynth::registerTriggerOnCallback([this](SynthVoice *voice,
                                              int offsetFrames, int id,
                                              void *userData) {
    if (this->mNotifier) {
      osc::Packet p;
      std::string prefix = "/" + this->name();
      if (prefix.size() == 1) {
        prefix = "";
      }
      p.beginMessage(prefix + "/triggerOn");
      offsetFrames = 0;
      p << offsetFrames << id;
      std::string voiceName = demangle(typeid(*voice).name());
      p << voiceName;
      auto fields = voice->getTriggerParams();
      for (const auto &field : fields) {
        if (field.type() == VariantType::VARIANT_FLOAT) {
          p << field.get<float>();
        } else if (field.type() == VariantType::VARIANT_DOUBLE) {
          p << field.get<double>();
        } else if (field.type() == VariantType::VARIANT_INT32) {
          p << field.get<int32_t>();
        } else if (field.type() == VariantType::VARIANT_UINT64) {
          p << field.get<uint64_t>();
        } else if (field.type() == VariantType::VARIANT_STRING) {
          p << field.get<std::string>();
        } else {
          assert(1 == 0);
          std::cerr << "ERROR type not implemented for distributed scene sync"
                    << std::endl;
        }
      }
      p.endMessage();

      if (verbose()) {
        std::cout << "Sending trigger on message for voice " << id << std::endl;
      }
      this->mNotifier->send(p);
    }
    return true;
  });

  PolySynth::registerTriggerOffCallback([this](int id, void *userData) {
    if (this->mNotifier) {
      osc::Packet p;
      std::string prefix = "/" + this->name();
      if (prefix.size() == 1) {
        prefix = "";
      }
      p.beginMessage(prefix + "/triggerOff");
      p << id;
      p.endMessage();

      if (verbose()) {
        std::cout << "Sending trigger off message" << std::endl;
      }
      this->mNotifier->send(p);
    }
    return true;
  });

  PolySynth::registerFreeCallback([this](int id, void *userData) {
    if (this->mNotifier) {
      osc::Packet p;
      std::string prefix = "/" + this->name();
      if (prefix.size() == 1) {
        prefix = "";
      }
      p.beginMessage(prefix + "/remove");
      p << id;
      p.endMessage();
      if (verbose()) {
        std::cout << " -- Sending free message " << id << std::endl;
      }
      this->mNotifier->send(p);
    }
    return true;
  });

  PolySynth::registerAllocateCallback([this](SynthVoice *voice,
                                             void *userData) {
    if (verbose()) {
      std::cout << "voice allocated " << std::endl;
    }
    if (!this->mNotifier) {
      return;
    }
    for (ParameterMeta *param : voice->triggerParameters()) {
      if (strcmp(typeid(*param).name(), typeid(Parameter).name()) == 0) {
        dynamic_cast<Parameter *>(param)->registerChangeCallback(
            [this, param, voice](float value) {
              if (this->mNotifier) {
                std::string prefix = "/" + this->name();
                if (prefix.size() == 1) {
                  prefix = "";
                }
                this->mNotifier->notifyListeners(
                    prefix + "/" + std::to_string(voice->id()) +
                        param->getFullAddress(),
                    param, nullptr);
              }
              //                                std::cout << voice->id() << "
              //                                parameter " << param->getName()
              //                                << "-> " << value << std::endl;
            });
      }
    }
    // register callbacks for internal parameters
    for (auto *param : voice->parameters()) {
      if (strcmp(typeid(*param).name(), typeid(Parameter).name()) ==
          0) { // Parameter
        Parameter *p = dynamic_cast<Parameter *>(param);
        p->registerChangeCallback([&, p, voice](float value) {
          if (this->mNotifier) {
            std::string prefix = "/" + this->name() + "/voice";
            if (prefix.size() == 1) {
              prefix = "";
            }
            this->mNotifier->notifyListeners(prefix + "/" +
                                                 std::to_string(voice->id()) +
                                                 p->getFullAddress(),
                                             p, nullptr);
          }
        });
      }
      if (strcmp(typeid(*param).name(), typeid(ParameterPose).name()) ==
          0) { // Parameter
        ParameterPose *p = dynamic_cast<ParameterPose *>(param);
        p->registerChangeCallback([&, p, voice](Pose value) {
          if (this->mNotifier) {
            std::string prefix = "/" + this->name() + "/voice";
            if (prefix.size() == 1) {
              prefix = "";
            }
            this->mNotifier->notifyListeners(prefix + "/" +
                                                 std::to_string(voice->id()) +
                                                 p->getFullAddress(),
                                             p, nullptr);
          }
        });
      }
    }
  });
}

void al::DistributedScene::allNotesOff() {
  PolySynth::allNotesOff();
  osc::Packet p;
  std::string prefix = "/" + this->name();
  if (prefix.size() == 1) {
    prefix = "";
  }
  p.beginMessage(prefix + "/allNotesOff");
  p.endMessage();

  if (verbose()) {
    std::cout << "Sending all notes off message" << std::endl;
  }
  if (this->mNotifier) {
    this->mNotifier->send(p);
  }
}

bool al::DistributedScene::consumeMessage(osc::Message &m,
                                          std::string rootOSCPath) {
  std::string address = m.addressPattern();
  if (rootOSCPath.size() > 0) {
    if (address.find(rootOSCPath, 0) == 1) {
      address = address.substr(rootOSCPath.size() + 1);
    } else {
      if (verbose()) {
        std::cout << "Prefix " << rootOSCPath << " not matched." << std::endl;
        return false;
      }
    }
  }

  if (address == "/triggerOn") {
    if (m.typeTags().size() > 2 && m.typeTags()[0] == 'i' &&
        m.typeTags()[1] == 'i' && m.typeTags()[2] == 's') {
      int offset, id;
      std::string voiceName;
      m >> offset >> id >> voiceName;
      auto *voice = getVoice(voiceName);
      if (voice) {
        std::vector<VariantValue> params;
        for (unsigned int i = 3; i < m.typeTags().size(); i++) {
          if (m.typeTags()[i] == 'f') {
            float value;
            m >> value;
            params.emplace_back(value);
          } else if (m.typeTags()[i] == 's') {
            std::string value;
            m >> value;
            params.emplace_back(value);
          } else {
            std::cerr << "ERROR: Unsupported parameter type for scene trigger"
                      << std::endl;
            params.emplace_back(0.0f);
          }
        }
        voice->setTriggerParams(params);
        if (verbose()) {
          std::cout << "trigger on received " << std::endl;
        }
        triggerOn(voice, offset, id);
        return true;
      } else {
        std::cerr << "Can't get free voice of type: " << voiceName << std::endl;
      }
    } else {
      std::cerr << "Unexpected type for /triggerOn name" << std::endl;
    }
  } else if (address == "/triggerOff") {
    if (m.typeTags() == "i") {
      int id;
      m >> id;
      triggerOff(id);
      if (verbose()) {
        std::cout << "trigger off received " << id << std::endl;
      }
      return true;
    }
  } else if (address == "/remove") {
    if (m.typeTags() == "i") {
      int id;
      m >> id;
      mVoiceIdsToFree.write((const char *)&id, sizeof(int));
      if (verbose()) {
        std::cout << "FREE received " << id << std::endl;
      }
      return true;
    }
  } else if (address == "/allNotesOff") {
    allNotesOff();
  } else {
    std::string addr = address;
    int start = std::string("/voice/").size();
    if (addr.compare(0, start, "/voice/") == 0) {
      std::string number =
          addr.substr(start, addr.find('/', start + 1) - start);
      std::string subAddr = addr.substr(start + number.size());
      SynthVoice *voice = dynamic_cast<DistributedScene *>(this)->mActiveVoices;
      while (voice) {
        if (voice->id() == std::stoi(number)) {
          for (auto *param : voice->triggerParameters()) {
            if (ParameterServer::setParameterValueFromMessage(param, subAddr,
                                                              m)) {
              // We assume no two parameters have the same address, so we can
              // break the loop. Perhaps this should be checked by
              // ParameterServer on registration?
              return true;
            }
          }
          for (auto *param : voice->parameters()) {
            if (ParameterServer::setParameterValueFromMessage(param, subAddr,
                                                              m)) {
              // We assume no two parameters have the same address, so we can
              // break the loop. Perhaps this should be checked by
              // ParameterServer on registration?
              return true;
            }
          }
        }
        voice = voice->next;
      }
      std::cerr << " -- Can't match voice id " << number << std::endl;
    }
  }
  if (verbose()) {
    m.print();
  }
  return false;
}
