/**
 * @file param_codec_custom.cpp
 * @brief Register a custom Parameter type with ParamCodec for OSC I/O.
 *
 * Builtin types (Parameter, ParameterInt, Pose, …) are registered automatically.
 * Custom ParameterWrapper subclasses need an explicit ParamCodec:
 *   attach  — wire change → OSC notify
 *   fromOsc — apply incoming OSC to the param
 *   notify  — push current value to listeners
 */

#include <iostream>

#include "al/ui/al_ParamCodec.hpp"
#include "al/ui/al_Parameter.hpp"
#include "al/ui/al_ParameterServer.hpp"

using namespace al;

/// Example: MIDI note number as its own parameter type.
class ParameterMidiNote : public ParameterWrapper<int32_t> {
public:
  ParameterMidiNote(std::string name, std::string group = "",
                    int32_t defaultValue = 60)
      : ParameterWrapper<int32_t>(std::move(name), std::move(group),
                                  defaultValue, 0, 127) {}
};

static ParamCodec makeMidiNoteCodec() {
  ParamCodec c;
  c.attach = [](ParameterMeta &param,
                const std::function<void(ValueSource *)> &notify) {
    auto *p = static_cast<ParameterMidiNote *>(&param);
    p->registerChangeCallback(
        [notify](int32_t /*v*/, ValueSource *src) { notify(src); });
  };
  c.fromOsc = [](ParameterMeta *param, const std::string &address,
                 osc::Message &m, ValueSource *src) -> bool {
    auto *p = static_cast<ParameterMidiNote *>(param);
    if (address != p->getFullAddress() || m.typeTags() != "i") {
      return false;
    }
    int val;
    m >> val;
    p->set(val, src);
    return true;
  };
  c.notify = [](OSCNotifier &notifier, const std::string &address,
                ParameterMeta *param, ValueSource *src) {
    notifier.notifyListeners(
        address, static_cast<ParameterMidiNote *>(param)->get(), src);
  };
  return c;
}

int main() {
  // Idempotent if called more than once; builtins already registered.
  paramCodecs().add<ParameterMidiNote>(makeMidiNoteCodec());

  ParameterMidiNote note{"note", "midi", 60};
  ParameterServer server("127.0.0.1", 9010, false);
  server << note;

  note.registerChangeCallback(
      [](int32_t v) { std::cout << "note -> " << v << std::endl; });

  note.set(72);
  std::cout << "Registered custom ParameterMidiNote with ParamCodec.\n"
            << "OSC address: " << note.getFullAddress() << " (typetag i)\n";
  return 0;
}
