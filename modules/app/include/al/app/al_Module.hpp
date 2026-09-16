#ifndef INCLUDE_AL_MODULE_HPP
#define INCLUDE_AL_MODULE_HPP

/**
 * @file al_Module.hpp
 * @brief Runtime Module vocabulary and schedule kinds.
 *
 * Runtime owns every loop. Root units are Modules; ordered work inside a
 * Module tick is a Stage; windows/FBOs are Surfaces owned by graphics.
 *
 * Legacy type names (*Domain, AsynchronousDomain, SynchronousDomain) remain
 * during migration — see TODO.md.
 */

#include <cstdint>

namespace al {

/// How Runtime drives a root Module.
enum class ModuleSchedule : uint8_t {
  /// Runtime main pump calls poll() then tickFrame() on the main thread.
  Main,
  /// Runtime-owned worker thread pump (future).
  Worker,
  /// start()/stop() only; device or network callbacks enter the module.
  Callback,
  /// Not a Runtime root — ticked only as a stage of a parent Module.
  Nested
};

} // namespace al

#endif // INCLUDE_AL_MODULE_HPP
