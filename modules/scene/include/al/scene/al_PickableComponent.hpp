#ifndef AL_PICKABLE_COMPONENT_HPP
#define AL_PICKABLE_COMPONENT_HPP

/**
 * @file al_PickableComponent.hpp
 * @brief ECS affordance: entity participates in Scene picking.
 *
 * State for the Component path. Interaction boxes / hit tests live in
 * al/ui/al_ScenePick.hpp (uses existing PickableBB as an implementation
 * detail). Prefer this over registering Pickables by hand for new Scene code.
 *
 * Legacy OO API: al/ui/al_Pickable.hpp — kept for DynamicScene / older apps.
 */

#include "al/scene/al_Component.hpp"

namespace al {

struct PickableComponent : Component {
  /// Local width/height ratio before Entity::size() (1 = square).
  float aspect{1.f};
  /// Depth as a fraction of min(width, height) for the hit box.
  float depthFrac{0.04f};
  bool enabled{true};
  bool selected{false};
  bool hover{false};

  /// Optional: world extents override (if > 0, used instead of size*aspect).
  float worldWidth{0.f};
  float worldHeight{0.f};

  void setAspect(float a) { aspect = a > 1e-4f ? a : 1.f; }

  void setWorldExtents(float widthM, float heightM) {
    worldWidth = widthM;
    worldHeight = heightM;
  }

  void clearWorldExtents() {
    worldWidth = 0.f;
    worldHeight = 0.f;
  }
};

} // namespace al

#endif
