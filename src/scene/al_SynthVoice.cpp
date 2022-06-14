#include "al/scene/al_PolySynth.hpp"

#include <memory>

using namespace al;

// --------- SynthVoice

bool SynthVoice::setTriggerParams(float *pFields, int numFields) {
  if (numFields < (int)mTriggerParams.size()) {
    // std::cout << "Pfield size mismatch. Ignoring all." << std::endl;
    return false;
  }
  for (auto &param : mTriggerParams) {
    param->fromFloat(*pFields++);
  }
  return true;
}

bool SynthVoice::setTriggerParams(const std::vector<float> &pFields,
                                  bool noCalls) {
  if (pFields.size() < mTriggerParams.size()) {
    // std::cout << "pField count mismatch. Ignoring." << std::endl;
    return false;
  }
  auto it = pFields.begin();
  if (noCalls) {
    for (auto &param : mTriggerParams) {
      static_cast<Parameter *>(param)->setNoCalls(*it++);
    }
  } else {
    for (auto &param : mTriggerParams) {
      static_cast<Parameter *>(param)->set(*it++);
    }
  }
  return true;
}

bool SynthVoice::setTriggerParams(const std::vector<VariantValue> &pFields,
                                  bool noCalls) {
  if (pFields.size() < mTriggerParams.size()) {
    // std::cout << "pField count mismatch. Ignoring." << std::endl;
    return false;
  }
  auto it = pFields.begin();
  // Trigger parameters should not trigger callbacks when set through
  // this function as these values are initial "construction" values
  // If you need the callbacks to propagate, set the parameter values
  // directly instead of through these functions.
  if (noCalls) {
    for (auto *param : mTriggerParams) {
      assert(param);
      switch (it->type()) {

      case (VariantType::VARIANT_FLOAT): {
        Parameter *p = nullptr;
        if ((p = dynamic_cast<Parameter *>(param))) {
          p->setNoCalls(it->get<float>());
        } else {
          std::cout << __FILE__ << ":" << __LINE__
                    << " ERROR field data type and parameter type mismatch"
                    << std::endl;
        }
      } break;
      case (VariantType::VARIANT_DOUBLE): {
        Parameter *p = nullptr;
        if ((p = dynamic_cast<Parameter *>(param))) {
          p->setNoCalls(it->get<double>());
        } else {
          std::cout << __FILE__ << ":" << __LINE__
                    << " ERROR field data type and parameter type mismatch"
                    << std::endl;
        }
      } break;
      case (VariantType::VARIANT_INT32): {
        ParameterInt *p = nullptr;
        ParameterMenu *pm = nullptr;
        if ((p = dynamic_cast<ParameterInt *>(param))) {
          p->setNoCalls(it->get<int32_t>());
        } else if ((pm = dynamic_cast<ParameterMenu *>(param))) {
          pm->setNoCalls(it->get<int32_t>());
        } else {
          std::cout << __FILE__ << ":" << __LINE__
                    << " ERROR field data type and parameter type mismatch"
                    << std::endl;
        }
      } break;
      case (VariantType::VARIANT_UINT32): {
        ParameterUInt32 *p = nullptr;
        if ((p = dynamic_cast<ParameterUInt32 *>(param))) {
          p->setNoCalls(it->get<uint32_t>());
        } else {
          std::cout << __FILE__ << ":" << __LINE__
                    << " ERROR field data type and parameter type mismatch"
                    << std::endl;
        }
      } break;
      case (VariantType::VARIANT_INT16): {
        ParameterInt16 *p = nullptr;
        if ((p = dynamic_cast<ParameterInt16 *>(param))) {
          p->setNoCalls(it->get<int16_t>());
        } else {
          std::cout << __FILE__ << ":" << __LINE__
                    << " ERROR field data type and parameter type mismatch"
                    << std::endl;
        }
      } break;
      case (VariantType::VARIANT_UINT16): {
        ParameterUInt16 *p = nullptr;
        if ((p = dynamic_cast<ParameterUInt16 *>(param))) {
          p->setNoCalls(it->get<uint16_t>());
        } else {
          std::cout << __FILE__ << ":" << __LINE__
                    << " ERROR field data type and parameter type mismatch"
                    << std::endl;
        }
      } break;
      case (VariantType::VARIANT_INT8): {
        ParameterInt8 *p = nullptr;
        if ((p = dynamic_cast<ParameterInt8 *>(param))) {
          p->setNoCalls(it->get<int8_t>());
        } else {
          std::cout << __FILE__ << ":" << __LINE__
                    << " ERROR field data type and parameter type mismatch"
                    << std::endl;
        }
      } break;
      case (VariantType::VARIANT_UINT8): {
        ParameterUInt8 *p = nullptr;
        if ((p = dynamic_cast<ParameterUInt8 *>(param))) {
          p->setNoCalls(it->get<uint8_t>());
        } else {
          std::cout << __FILE__ << ":" << __LINE__
                    << " ERROR field data type and parameter type mismatch"
                    << std::endl;
        }
      } break;
      case (VariantType::VARIANT_INT64): {
        ParameterInt64 *p = nullptr;
        if ((p = dynamic_cast<ParameterInt64 *>(param))) {
          p->setNoCalls(it->get<int64_t>());
        } else {
          std::cout << __FILE__ << ":" << __LINE__
                    << " ERROR field data type and parameter type mismatch"
                    << std::endl;
        }
      } break;
      case (VariantType::VARIANT_UINT64): {
        ParameterUInt64 *p = nullptr;
        if ((p = dynamic_cast<ParameterUInt64 *>(param))) {
          p->setNoCalls(it->get<uint64_t>());
        } else {
          std::cout << __FILE__ << ":" << __LINE__
                    << " ERROR field data type and parameter type mismatch"
                    << std::endl;
        }
      } break;
      case (VariantType::VARIANT_STRING): {
        ParameterString *p = nullptr;
        ParameterMenu *pm = nullptr;
        if ((p = dynamic_cast<ParameterString *>(param))) {
          p->setNoCalls(it->get<std::string>());
        } else if ((pm = dynamic_cast<ParameterMenu *>(param))) {
          pm->setCurrent(it->get<std::string>(), true);
        } else {
          std::cerr << "ERROR: p-field string setting parameter. Invalid "
                       "parameter type for parameter "
                    << param->getFullAddress() << std::endl;
        }
      } break;

      case (VariantType::VARIANT_NONE):
      case (VariantType::VARIANT_CHAR):
      case (VariantType::VARIANT_BOOL):
      case (VariantType::VARIANT_VARIANT_VECTOR):
      case (VariantType::VARIANT_VECTOR_OFFSET):
        std::cout << "Ignoring VARIANT parameter field" << std::endl;
      }

      it++;
    }
  } else {
    for (auto *param : mTriggerParams) {
      assert(param);
      if (it->type() == VariantType::VARIANT_FLOAT) {
        if (strcmp(typeid(*param).name(), typeid(Parameter).name()) == 0) {
          static_cast<Parameter *>(param)->set(it->get<float>());
        } else if (strcmp(typeid(*param).name(), typeid(ParameterInt).name()) ==
                   0) {
          static_cast<ParameterInt *>(param)->set(it->get<float>());
        } else if (strcmp(typeid(*param).name(),
                          typeid(ParameterMenu).name()) == 0) {
          static_cast<ParameterMenu *>(param)->set(it->get<float>());
        } else if (strcmp(typeid(*param).name(),
                          typeid(ParameterString).name()) == 0) {
          static_cast<ParameterString *>(param)->set(
              std::to_string(it->get<float>()));
        } else {
          std::cerr << "ERROR: p-field string not setting parameter. Invalid "
                       "parameter type for parameter "
                    << param->getFullAddress() << std::endl;
        }
      } else if (it->type() == VariantType::VARIANT_STRING) {
        if (strcmp(typeid(*param).name(), typeid(ParameterString).name()) ==
            0) {
          static_cast<ParameterString *>(param)->set(it->get<std::string>());
        } else if (strcmp(typeid(*param).name(),
                          typeid(ParameterMenu).name()) == 0) {
          static_cast<ParameterMenu *>(param)->setCurrent(
              it->get<std::string>(), noCalls);
        } else {
          std::cerr << "ERROR: p-field string not setting parameter. Invalid "
                       "parameter type for parameter "
                    << param->getFullAddress() << std::endl;
        }
      } else {
        std::cerr << "ERROR: Unexpected parameter field value type"
                  << std::endl;
      }
      it++;
    }
  }
  return true;
}

int SynthVoice::getTriggerParams(float *pFields, int maxParams) {
  std::vector<VariantValue> pFieldsVector = getTriggerParams();
  if (maxParams == -1) {
    assert(pFieldsVector.size() < INT_MAX);
    maxParams = int(pFieldsVector.size());
  }
  int count = 0;
  for (auto param : pFieldsVector) {
    if (count == maxParams) {
      break;
    }
    if (param.type() == VariantType::VARIANT_FLOAT) {
      *pFields++ = param.get<float>();
    } else {
      *pFields++ = 0.0f; // Ignore strings...
    }
    count++;
  }
  return count;
}

std::vector<VariantValue> SynthVoice::getTriggerParams() {
  std::vector<VariantValue> pFields;
  pFields.reserve(mTriggerParams.size());
  for (auto param : mTriggerParams) {
    if (param) {
      if (strcmp(typeid(*param).name(), typeid(ParameterString).name()) == 0) {
        pFields.push_back(static_cast<ParameterString *>(param)->get());
      } else if (strcmp(typeid(*param).name(), typeid(ParameterMenu).name()) ==
                 0) {
        pFields.push_back(static_cast<ParameterMenu *>(param)->getCurrent());
      } else {
        pFields.push_back(param->toFloat());
      }
    }
  }
  return pFields;
}

void SynthVoice::triggerOn(int offsetFrames) {
  mOnOffsetFrames = offsetFrames;
  mActive = true;
  onTriggerOn();
}

void SynthVoice::triggerOff(int offsetFrames) {
  mOffOffsetFrames =
      offsetFrames; // TODO implement offset frames for trigger off.
  // Currently ignoring and turning off at start of buffer
  onTriggerOff();
}

int SynthVoice::getStartOffsetFrames(unsigned int framesPerBuffer) {
  int frames = mOnOffsetFrames;
  mOnOffsetFrames -= framesPerBuffer;
  if (mOnOffsetFrames < 0) {
    mOnOffsetFrames = 0;
  }
  return frames;
}

int SynthVoice::getEndOffsetFrames(unsigned int framesPerBuffer) {
  int frames = mOffOffsetFrames;
  mOffOffsetFrames -= framesPerBuffer;
  if (mOffOffsetFrames < 0) {
    mOffOffsetFrames = 0;
  }
  return frames;
}

std::shared_ptr<Parameter>
SynthVoice::createInternalTriggerParameter(std::string name, float defaultValue,
                                           float minValue, float maxValue) {
  mInternalParameters[name] =
      std::make_shared<Parameter>(name, defaultValue, minValue, maxValue);
  registerTriggerParameter(*mInternalParameters[name]);
  return mInternalParameters[name];
}

Parameter &SynthVoice::getInternalParameter(std::string name) {
  if (mInternalParameters.find(name) != mInternalParameters.end()) {
    return *mInternalParameters[name];
  }
  std::cerr << "Parameter not found! Aborting: " << name << std::endl;
  throw "Invalid parameter name";
}

float SynthVoice::getInternalParameterValue(std::string name) {
  if (mInternalParameters.find(name) != mInternalParameters.end()) {
    return mInternalParameters[name]->get();
  }
  return 0.0;
}

void SynthVoice::setInternalParameterValue(std::string name, float value) {
  if (mInternalParameters.find(name) != mInternalParameters.end()) {
    mInternalParameters[name]->set(value);
  }
}
