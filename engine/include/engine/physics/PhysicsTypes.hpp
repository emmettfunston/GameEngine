#pragma once

#include <cstdint>

#include <glm/vec3.hpp>

namespace engine {

enum class PhysicsLayer : std::uint16_t {
  WorldStatic = 0,
  WorldDynamic = 1,
  Player = 2,
  Trigger = 3,
};

using LayerMask = std::uint32_t;

constexpr LayerMask toLayerMask(PhysicsLayer layer) {
  return static_cast<LayerMask>(1u << static_cast<std::uint16_t>(layer));
}

constexpr LayerMask LayerMaskAll = 0xFFFFFFFFu;

struct PhysicsBodyID {
  std::uint32_t value = 0xFFFFFFFFu;

  bool isValid() const { return value != 0xFFFFFFFFu; }

  friend bool operator==(const PhysicsBodyID& a, const PhysicsBodyID& b) { return a.value == b.value; }
};

struct RaycastHit {
  bool hit = false;
  PhysicsBodyID bodyId{};
  std::uint32_t entityId = 0;
  glm::vec3 point{0.0f};
  glm::vec3 normal{0.0f, 1.0f, 0.0f};
  float distance = 0.0f;
};

} // namespace engine
