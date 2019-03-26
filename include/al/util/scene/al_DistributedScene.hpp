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
    DistributedScene(std::string name = "scene", int threadPoolSize = 0, TimeMasterMode masterMode = TIME_MASTER_AUDIO);

    DistributedScene(TimeMasterMode masterMode = TIME_MASTER_AUDIO)
        : DistributedScene("scene", 0, masterMode) {}

    
    std::string name() {return mName;}

    void registerNotifier(OSCNotifier &notifier) {mNotifier = &notifier;}

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
        osc::Packet p;
        p.beginMessage("/triggerOn");
        offsetFrames = 0;
        p << offsetFrames << id;
        std::string voiceName = demangle(typeid(*voice).name());
        p<<voiceName;
        auto fields = voice->getParamFields();
        for (auto field: fields) {
            if (field.type() == ParameterField::FLOAT) {
                p << field.get<float>();
            } else {
                p << field.get<std::string>();
            }
        }
        p.endMessage();

        if (this->mNotifier) {
            this->mNotifier->send(p);
        }
    });

    PolySynth::registerTriggerOffCallback(
                [this](int id, void *userData) {
        osc::Packet p;
        p.beginMessage("/triggerOff");
        p << id;
        p.endMessage();

        if (this->mNotifier) {
            this->mNotifier->send(p);
        }

    });

    PolySynth::registerAllocateCallback(
                [this](SynthVoice *voice, void *userData) {
        std::cout << "voice allocated " << std::endl;
        for (auto *param : voice->parameterFields()) {
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

bool DistributedScene::consumeMessage(osc::Message &m, std::string rootOSCPath) {
    if (verbose()) {
      m.print();
    }
    if (m.addressPattern() == "/triggerOn") {
        if (m.typeTags().size() > 2
                && m.typeTags()[0] == 'i'
                && m.typeTags()[1] == 'i'
                && m.typeTags()[2] == 's') {
            int offset, id;
            std::string voiceName;
            m >> offset >> id >> voiceName;
            auto *voice = getVoice(voiceName);
            if (voice) {
                std::vector<float> params;
                params.resize(m.typeTags().size() - 3);

                std::cout << m.typeTags()<< " params:" << params.size() <<std::endl;
                for (unsigned int i = 0; i < m.typeTags().size() -3; i++) {
                    m >> params[i];
                }
                voice->setParamFields(params);
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
    } else if (m.addressPattern() == "/triggerOff") {
        if (m.typeTags() == "i") {
            int id;
            m >> id;
            triggerOff(id);
            if (verbose()) {
              std::cout << "trigger off received " << id <<std::endl;
            }
            return true;
        }
    } else {
        std::string addr = m.addressPattern();
        int start = ("/" + name() + "/").size();
        if (addr.compare(0, start, "/" + name() + "/") == 0) {
            std::string number = addr.substr(start, addr.find('/',  start + 1) - start);
            std::string subAddr = addr.substr(start + number.size());
            SynthVoice *voice = mActiveVoices;
            while (voice) {
                if (voice->id() == std::stoi(number)) {
                    for (auto *param: voice->parameterFields()) {
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
