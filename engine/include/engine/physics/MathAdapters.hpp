#pragma once

#include <glm/geometric.hpp>
#include <glm/vec3.hpp>

#include <Jolt/Jolt.h>
#include <Jolt/Math/Quat.h>
#include <Jolt/Math/Vec3.h>

namespace engine::math_adapters {

inline JPH::Vec3 toJolt(const glm::vec3& v) {
  return JPH::Vec3(v.x, v.y, v.z);
}

inline JPH::RVec3 toJoltR(const glm::vec3& v) {
  return JPH::RVec3(v.x, v.y, v.z);
}

inline glm::vec3 toGlm(const JPH::Vec3& v) {
  return glm::vec3(v.GetX(), v.GetY(), v.GetZ());
}

inline glm::vec3 normalizedOrZero(const glm::vec3& v) {
  const float len2 = glm::dot(v, v);
  if (len2 <= 1.0e-8f) {
    return glm::vec3(0.0f);
  }
  return glm::normalize(v);
}

} // namespace engine::math_adapters
