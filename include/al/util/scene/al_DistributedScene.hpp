#ifndef AL_DISTRIBUTEDSCENE_HPP
#define AL_DISTRIBUTEDSCENE_HPP

#include "al/core/protocol/al_OSC.hpp"
#include "al/util/scene/al_DynamicScene.hpp"
#include "al/util/ui/al_ParameterServer.hpp"


namespace al
{
/**
 * @brief The DistributedScene class
 */
class DistributedScene :
        public DynamicScene,
        public osc::MessageConsumer
{
public:
    DistributedScene(std::string name = "scene", int threadPoolSize = 0, TimeMasterMode masterMode = TIME_MASTER_CPU);

    DistributedScene(TimeMasterMode masterMode = TIME_MASTER_CPU)
        : DistributedScene("scene", 0, masterMode) {}

    
    std::string name() {return mName;}

    void registerNotifier(OSCNotifier &notifier) {mNotifier = &notifier;}

    virtual void allNotesOff() override;

    virtual bool consumeMessage(osc::Message& m, std::string rootOSCPath = "") override;

protected:

private:
    OSCNotifier *mNotifier {nullptr};
    std::string mName;
};

DistributedScene::DistributedScene(std::string name, int threadPoolSize, PolySynth::TimeMasterMode masterMode)
    : DynamicScene (threadPoolSize, masterMode)
{
    mName = name;

    PolySynth::registerTriggerOnCallback(
                [this](SynthVoice *voice, int offsetFrames, int id, void *userData) {
        if (this->mNotifier) {
          osc::Packet p;
          std::string prefix = this->name() + "/";
          p.beginMessage("/" + prefix + "triggerOn");
          offsetFrames = 0;
          p << offsetFrames << id;
          std::string voiceName = demangle(typeid(*voice).name());
          p<<voiceName;
          auto fields = voice->getTriggerParams();
          for (auto field: fields) {
              if (field.type() == ParameterField::FLOAT) {
                  p << field.get<float>();
              } else {
                  p << field.get<std::string>();
              }
          }
          p.endMessage();

          if (verbose()) {
            std::cout << "Sending trigger on message" << std::endl;
          }
          this->mNotifier->send(p);
        }
        return true;
    });

    PolySynth::registerTriggerOffCallback(
                [this](int id, void *userData) {
        if (this->mNotifier) {
          osc::Packet p;
          std::string prefix = this->name() + "/";
          p.beginMessage("/" + prefix + "triggerOff");
          p << id;
          p.endMessage();

          if (verbose()) {
            std::cout << "Sending trigger off message" << std::endl;
          }
          this->mNotifier->send(p);
        }
        return true;

    });

    PolySynth::registerAllocateCallback(
                [this](SynthVoice *voice, void *userData) {
        if (verbose()) {
          std::cout << "voice allocated " << std::endl;
        }
        for (auto *param : voice->triggerParameters()) {
            if (strcmp(typeid(*param).name(), typeid(Parameter).name()) == 0) {
                dynamic_cast<Parameter *>(param)->registerChangeCallback(
                            [this, param, voice](float value) {
                    if (this->mNotifier) {
                        this->mNotifier->notifyListeners("/" + this->name() + "/" + std::to_string(voice->id()) + param->getFullAddress(),
                                                         param);
                    }
                    //                                std::cout << voice->id() << " parameter " << param->getName() << "-> " << value << std::endl;
                });
            }
        }
    });

}

void DistributedScene::allNotesOff()
{

  PolySynth::allNotesOff();
  osc::Packet p;
  p.beginMessage("/allNotesOff");
  p.endMessage();

  if (verbose()) {
    std::cout << "Sending all notes off message" << std::endl;
  }
  if (this->mNotifier) {
      this->mNotifier->send(p);
  }

}

bool DistributedScene::consumeMessage(osc::Message &m, std::string rootOSCPath) {
    if (verbose()) {
      m.print();
    }
    std::string address = m.addressPattern();
    if (rootOSCPath.size() > 0) {
      if (address.find(rootOSCPath, 0) == 1) {
        address = address.substr(rootOSCPath.size() + 1);
      } else {
        if (verbose()) {
          std::cout << "Prefix " << rootOSCPath << " not matched." <<  std::endl;
          return false;
        }
      }
    }

    if (address == "/triggerOn") {
        if (m.typeTags().size() > 2
                && m.typeTags()[0] == 'i'
                && m.typeTags()[1] == 'i'
                && m.typeTags()[2] == 's') {
            int offset, id;
            std::string voiceName;
            m >> offset >> id >> voiceName;
            auto *voice = getVoice(voiceName);
            if (voice) {
                std::vector<ParameterField> params;
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
                    std::cerr << "ERROR: Unsupported parameter type for scene trigger" << std::endl;
                    params.emplace_back(0.0f);
                  }
                }
                voice->setTriggerParams(params);
                triggerOn(voice, offset, id);
                if (verbose()) {
                  std::cout << "trigger on received" <<std::endl;
                }
                return true;
            } else {
                std::cerr << "Can't get free voice of type: " << voiceName<< std::endl;
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
              std::cout << "trigger off received " << id <<std::endl;
            }
            return true;
        }
    } else if (address == "/allNotesOff") {
      allNotesOff();
    } else {
        std::string addr = address;
        int start = ("/" + name() + "/").size();
        if (addr.compare(0, start, "/" + name() + "/") == 0) {
            std::string number = addr.substr(start, addr.find('/',  start + 1) - start);
            std::string subAddr = addr.substr(start + number.size());
            SynthVoice *voice = mActiveVoices;
            while (voice) {
                if (voice->id() == std::stoi(number)) {
                    for (auto *param: voice->triggerParameters()) {
                        if (ParameterServer::setParameterValueFromMessage(param, subAddr, m)) {
                            // We assume no two parameters have the same address, so we can break the
                            // loop. Perhaps this should be checked by ParameterServer on registration?
                            break;
                        }
                    }
                }
                voice = voice->next;
            }
            return true;
        }
    }
    return false;
}

}

#endif // AL_DISTRIBUTEDSCENE_HPP
