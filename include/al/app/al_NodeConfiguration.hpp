#ifndef INCLUDE_AL_NODECONFIGURATION
#define INCLUDE_AL_NODECONFIGURATION

#include <cinttypes>
#include <iostream>

namespace al {

// TODO flow parameters
typedef enum {
  CAP_NONE = 0,
  CAP_SIMULATOR = 1 << 1,
  CAP_RENDERING = 1 << 2,
  CAP_OMNIRENDERING = 1 << 3,
  CAP_AUDIO_IO = 1 << 4,
  CAP_OSC = 1 << 5,
  CAP_CONSOLE_IO = 1 << 6,
  CAP_2DGUI = 1 << 7,
  CAP_STATE_SEND = 1 << 8,
  CAP_USER = 1 << 10
  // User defined capabilities can add from here through bitshifting
} Capability;

/**
 * @brief The NodeConfiguration struct describes a distributed node and its
 * capabilities
 */
struct NodeConfiguration {
  uint16_t rank{0};
  uint16_t group{0};

  std::string dataRoot;

  Capability mCapabilites{CAP_NONE};

  bool hasCapability(Capability cap) { return cap & mCapabilites; }
  bool isPrimary() { return rank == 0; }

  void setRole(std::string role) {
    if (role == "desktop") {
      mCapabilites =
          (Capability)(CAP_SIMULATOR | CAP_STATE_SEND | CAP_RENDERING |
                       CAP_AUDIO_IO | CAP_OSC | CAP_2DGUI);
    } else if (role == "renderer") {
      mCapabilites = (Capability)(CAP_SIMULATOR | CAP_OMNIRENDERING | CAP_OSC);
    } else if (role == "audio") {
      mCapabilites =
          (Capability)(CAP_SIMULATOR | CAP_AUDIO_IO | CAP_CONSOLE_IO | CAP_OSC);
    } else if (role == "simulator") {
      mCapabilites = (Capability)(CAP_SIMULATOR | CAP_STATE_SEND |
                                  CAP_CONSOLE_IO | CAP_OSC);
    } else if (role == "replica") {
      mCapabilites = (Capability)(CAP_SIMULATOR | CAP_OMNIRENDERING |
                                  CAP_AUDIO_IO | CAP_OSC);
    } else if (role == "control") {
      mCapabilites = (Capability)(CAP_RENDERING | CAP_OSC | CAP_2DGUI);
    } else {
      std::cerr << "WARNING: Setting no capabilities for this app from "
                   "config file"
                << std::endl;
    }
  }
};

} // namespace al

#endif // INCLUDE_AL_NODECONFIGURATION
