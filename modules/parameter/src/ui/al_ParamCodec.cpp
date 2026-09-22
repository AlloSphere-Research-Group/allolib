#include "al/ui/al_ParamCodec.hpp"

#include <cstdint>
#include <cstring>
#include <type_traits>

#include "al/ui/al_Parameter.hpp"
#include "al/ui/al_ParameterServer.hpp"

using namespace al;

ParamCodecRegistry &ParamCodecRegistry::instance() {
  static ParamCodecRegistry reg;
  reg.ensureBuiltins();
  return reg;
}

void ParamCodecRegistry::add(std::type_index type, ParamCodec codec) {
  mCodecs[type] = std::move(codec);
}

const ParamCodec *ParamCodecRegistry::find(const ParameterMeta &param) const {
  return find(std::type_index(typeid(param)));
}

const ParamCodec *ParamCodecRegistry::find(std::type_index type) const {
  auto it = mCodecs.find(type);
  return it == mCodecs.end() ? nullptr : &it->second;
}

namespace {

template <typename ParamT, typename ValueT>
ParamCodec makeScalarCodec(char oscTag) {
  ParamCodec c;
  c.attach = [](ParameterMeta &param,
                const std::function<void(ValueSource *)> &notify) {
    auto *p = static_cast<ParamT *>(&param);
    p->registerChangeCallback(
        [notify](ValueT /*v*/, ValueSource *src) { notify(src); });
  };
  c.fromOsc = [oscTag](ParameterMeta *param, const std::string &address,
                       osc::Message &m, ValueSource *src) -> bool {
    auto *p = static_cast<ParamT *>(param);
    if (address != p->getFullAddress()) {
      return false;
    }
    if constexpr (std::is_same_v<ValueT, float>) {
      if (m.typeTags() != "f") {
        return false;
      }
      float val;
      m >> val;
      p->set(val, src);
    } else if constexpr (std::is_same_v<ValueT, double>) {
      if (m.typeTags() == "d") {
        double val;
        m >> val;
        p->set(val, src);
      } else if (m.typeTags() == "f") {
        float val;
        m >> val;
        p->set(static_cast<double>(val), src);
      } else {
        return false;
      }
    } else if constexpr (std::is_same_v<ValueT, std::string>) {
      if (m.typeTags() != "s") {
        return false;
      }
      std::string val;
      m >> val;
      p->set(val, src);
    } else if constexpr (std::is_integral_v<ValueT>) {
      if (m.typeTags().size() != 1 || m.typeTags()[0] != oscTag) {
        return false;
      }
      int val;
      m >> val;
      p->set(static_cast<ValueT>(val), src);
    }
    return true;
  };
  c.notify = [](OSCNotifier &notifier, const std::string &address,
                ParameterMeta *param, ValueSource *src) {
    auto *p = static_cast<ParamT *>(param);
    if constexpr (std::is_same_v<ValueT, float>) {
      notifier.notifyListeners(address, p->get(), src);
    } else if constexpr (std::is_same_v<ValueT, double>) {
      notifier.notifyListeners(address, static_cast<float>(p->get()), src);
    } else if constexpr (std::is_same_v<ValueT, std::string>) {
      notifier.notifyListeners(address, p->get(), src);
    } else if constexpr (std::is_integral_v<ValueT>) {
      notifier.notifyListeners(address, static_cast<int>(p->get()), src);
    }
  };
  return c;
}

ParamCodec makePoseCodec() {
  ParamCodec c;
  c.attach = [](ParameterMeta &param,
                const std::function<void(ValueSource *)> &notify) {
    auto *p = static_cast<ParameterPose *>(&param);
    p->registerChangeCallback(
        [notify](Pose /*v*/, ValueSource *src) { notify(src); });
  };
  c.fromOsc = [](ParameterMeta *param, const std::string &address,
                 osc::Message &m, ValueSource *src) -> bool {
    auto *p = static_cast<ParameterPose *>(param);
    if (address == p->getFullAddress() && m.typeTags() == "fffffff") {
      float x, y, z, w, qx, qy, qz;
      m >> x >> y >> z >> w >> qx >> qy >> qz;
      p->set(Pose(Vec3d(x, y, z), Quatd(w, qx, qy, qz)), src);
      return true;
    }
    if (address == p->getFullAddress() + "/pos" && m.typeTags() == "fff") {
      float x, y, z;
      m >> x >> y >> z;
      Pose currentPose = p->get();
      currentPose.pos() = Vec3d(x, y, z);
      p->set(currentPose, src);
      return true;
    }
    if (address == p->getFullAddress() + "/pos/x" && m.typeTags() == "f") {
      float x;
      m >> x;
      Pose currentPose = p->get();
      currentPose.pos().x = x;
      p->set(currentPose, src);
      return true;
    }
    if (address == p->getFullAddress() + "/pos/y" && m.typeTags() == "f") {
      float y;
      m >> y;
      Pose currentPose = p->get();
      currentPose.pos().y = y;
      p->set(currentPose, src);
      return true;
    }
    if (address == p->getFullAddress() + "/pos/z" && m.typeTags() == "f") {
      float z;
      m >> z;
      Pose currentPose = p->get();
      currentPose.pos().z = z;
      p->set(currentPose, src);
      return true;
    }
    return false;
  };
  c.notify = [](OSCNotifier &notifier, const std::string &address,
                ParameterMeta *param, ValueSource *src) {
    notifier.notifyListeners(address, static_cast<ParameterPose *>(param)->get(),
                             src);
  };
  return c;
}

template <typename ParamT, typename VecT, size_t N>
ParamCodec makeVecCodec(const char *tags) {
  ParamCodec c;
  c.attach = [](ParameterMeta &param,
                const std::function<void(ValueSource *)> &notify) {
    auto *p = static_cast<ParamT *>(&param);
    p->registerChangeCallback(
        [notify](VecT /*v*/, ValueSource *src) { notify(src); });
  };
  c.fromOsc = [tags](ParameterMeta *param, const std::string &address,
                     osc::Message &m, ValueSource *src) -> bool {
    auto *p = static_cast<ParamT *>(param);
    if (address != p->getFullAddress() || m.typeTags() != tags) {
      return false;
    }
    VecT v;
    for (size_t i = 0; i < N; ++i) {
      float f;
      m >> f;
      v[i] = f;
    }
    p->set(v, src);
    return true;
  };
  c.notify = [](OSCNotifier &notifier, const std::string &address,
                ParameterMeta *param, ValueSource *src) {
    notifier.notifyListeners(address, static_cast<ParamT *>(param)->get(), src);
  };
  return c;
}

ParamCodec makeColorCodec() {
  ParamCodec c;
  c.attach = [](ParameterMeta &param,
                const std::function<void(ValueSource *)> &notify) {
    auto *p = static_cast<ParameterColor *>(&param);
    p->registerChangeCallback(
        [notify](Color /*v*/, ValueSource *src) { notify(src); });
  };
  c.fromOsc = [](ParameterMeta *param, const std::string &address,
                 osc::Message &m, ValueSource *src) -> bool {
    auto *p = static_cast<ParameterColor *>(param);
    if (address != p->getFullAddress() || m.typeTags() != "ffff") {
      return false;
    }
    float a, b, c, d;
    m >> a >> b >> c >> d;
    p->set(Color(a, b, c, d), src);
    return true;
  };
  c.notify = [](OSCNotifier &notifier, const std::string &address,
                ParameterMeta *param, ValueSource *src) {
    notifier.notifyListeners(address,
                             static_cast<ParameterColor *>(param)->get(), src);
  };
  return c;
}

ParamCodec makeTriggerCodec() {
  ParamCodec c;
  c.attach = [](ParameterMeta &param,
                const std::function<void(ValueSource *)> &notify) {
    auto *p = static_cast<Trigger *>(&param);
    p->registerChangeCallback(
        [notify](bool /*v*/, ValueSource *src) { notify(src); });
  };
  c.fromOsc = [](ParameterMeta *param, const std::string &address,
                 osc::Message &m, ValueSource * /*src*/) -> bool {
    auto *p = static_cast<Trigger *>(param);
    if (address != p->getFullAddress()) {
      return false;
    }
    if (m.typeTags().empty()) {
      p->trigger();
      return true;
    }
    if (m.typeTags() == "f") {
      float val;
      m >> val;
      if (val == 1.0f) {
        p->trigger();
      }
      return true;
    }
    return false;
  };
  c.notify = [](OSCNotifier &notifier, const std::string &address,
                ParameterMeta *param, ValueSource *src) {
    notifier.notifyListeners(address,
                             static_cast<Trigger *>(param)->get() ? 1.f : 0.f,
                             src);
  };
  return c;
}

} // namespace

void ParamCodecRegistry::ensureBuiltins() {
  if (mBuiltinsRegistered) {
    return;
  }
  mBuiltinsRegistered = true;

  add<ParameterBool>(makeScalarCodec<ParameterBool, float>('f'));
  add<Parameter>(makeScalarCodec<Parameter, float>('f'));
  add<Trigger>(makeTriggerCodec());

  add<ParameterInt>(makeScalarCodec<ParameterInt, int32_t>('i'));
  add<ParameterInt8>(makeScalarCodec<ParameterInt8, int8_t>('i'));
  add<ParameterInt16>(makeScalarCodec<ParameterInt16, int16_t>('i'));
  add<ParameterInt64>(makeScalarCodec<ParameterInt64, int64_t>('i'));
  add<ParameterUInt8>(makeScalarCodec<ParameterUInt8, uint8_t>('i'));
  add<ParameterUInt16>(makeScalarCodec<ParameterUInt16, uint16_t>('i'));
  add<ParameterUInt32>(makeScalarCodec<ParameterUInt32, uint32_t>('i'));
  add<ParameterUInt64>(makeScalarCodec<ParameterUInt64, uint64_t>('i'));
  add<ParameterDouble>(makeScalarCodec<ParameterDouble, double>('d'));

  add<ParameterString>(makeScalarCodec<ParameterString, std::string>('s'));
  add<ParameterMenu>(makeScalarCodec<ParameterMenu, int32_t>('i'));
  add<ParameterChoice>(makeScalarCodec<ParameterChoice, uint64_t>('i'));

  add<ParameterPose>(makePoseCodec());
  add<ParameterVec3>(makeVecCodec<ParameterVec3, Vec3f, 3>("fff"));
  add<ParameterVec4>(makeVecCodec<ParameterVec4, Vec4f, 4>("ffff"));
  add<ParameterVec5>(makeVecCodec<ParameterVec5, Vec5f, 5>("fffff"));
  add<ParameterColor>(makeColorCodec());
}
