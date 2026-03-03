#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

namespace engine {

class Camera {
public:
  glm::vec3 position{0.0f, 0.0f, 3.0f};
  float yawDegrees = -90.0f;
  float pitchDegrees = 0.0f;
  float fovDegrees = 60.0f;
  float nearPlane = 0.1f;
  float farPlane = 200.0f;
  float aspect = 16.0f / 9.0f;

  glm::mat4 viewMatrix() const;
  glm::mat4 projMatrix() const;

  glm::vec3 forward() const;
  glm::vec3 right() const;
  glm::vec3 up() const;
};

} // namespace engine


