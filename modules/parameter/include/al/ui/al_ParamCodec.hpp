#ifndef AL_PARAM_CODEC_HPP
#define AL_PARAM_CODEC_HPP

/**
 * @file al_ParamCodec.hpp
 * @brief Type registry for Parameter OSC notify / receive / change wiring.
 *
 * Builtin codecs cover Parameter, ParameterBool, numeric ParamNumeric types,
 * String, Vec, Pose, Color, Menu, Choice, Trigger.
 *
 * ParameterServer and ParameterBundle look up codecs by typeid instead of
 * maintaining dynamic_cast ladders.
 *
 * Custom types:
 * @code
 *   paramCodecs().add<MyParam>(ParamCodec{
 *     .attach = [](ParameterMeta &p, auto &notify) { ... },
 *     .fromOsc = [](ParameterMeta *p, const std::string &addr,
 *                   osc::Message &m, ValueSource *src) { ... },
 *     .notify = [](OSCNotifier &n, const std::string &addr,
 *                  ParameterMeta *p, ValueSource *src) { ... },
 *   });
 * @endcode
 * See examples/ui/param_codec_custom.cpp.
 */

#include <functional>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>

#include "al/protocol/al_OSC.hpp"
#include "al/types/al_ValueSource.hpp"

namespace al {

class ParameterMeta;
class OSCNotifier;

struct ParamCodec {
  /// Wire change callbacks so value updates broadcast via \p notify.
  using AttachFn = std::function<void(
      ParameterMeta &param,
      const std::function<void(ValueSource *src)> &notify)>;

  /// Apply an OSC message to \p param if address matches. Return true if handled.
  using FromOscFn = std::function<bool(ParameterMeta *param,
                                       const std::string &address,
                                       osc::Message &m, ValueSource *src)>;

  /// Push current value of \p param to listeners at \p address.
  using NotifyFn = std::function<void(OSCNotifier &notifier,
                                      const std::string &address,
                                      ParameterMeta *param, ValueSource *src)>;

  AttachFn attach;
  FromOscFn fromOsc;
  NotifyFn notify;
};

class ParamCodecRegistry {
public:
  static ParamCodecRegistry &instance();

  void add(std::type_index type, ParamCodec codec);
  const ParamCodec *find(const ParameterMeta &param) const;
  const ParamCodec *find(std::type_index type) const;

  /// Register all built-in Parameter* codecs (idempotent).
  void ensureBuiltins();

  template <typename T> void add(ParamCodec codec) {
    add(std::type_index(typeid(T)), std::move(codec));
  }

  template <typename T> const ParamCodec *find() const {
    return find(std::type_index(typeid(T)));
  }

private:
  ParamCodecRegistry() = default;
  std::unordered_map<std::type_index, ParamCodec> mCodecs;
  bool mBuiltinsRegistered{false};
};

inline ParamCodecRegistry &paramCodecs() {
  return ParamCodecRegistry::instance();
}

} // namespace al

#endif
