#include "engine/renderer/Camera.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>

namespace engine {
namespace {

glm::vec3 dirFromYawPitch(float yawDeg, float pitchDeg) {
  const float yaw = glm::radians(yawDeg);
  const float pitch = glm::radians(pitchDeg);

  glm::vec3 f;
  f.x = std::cos(yaw) * std::cos(pitch);
  f.y = std::sin(pitch);
  f.z = std::sin(yaw) * std::cos(pitch);
  return glm::normalize(f);
}

} // namespace

glm::vec3 Camera::forward() const {
  return dirFromYawPitch(yawDegrees, pitchDegrees);
}

glm::vec3 Camera::right() const {
  return glm::normalize(glm::cross(forward(), glm::vec3(0, 1, 0)));
}

glm::vec3 Camera::up() const {
  return glm::normalize(glm::cross(right(), forward()));
}

glm::mat4 Camera::viewMatrix() const {
  return glm::lookAt(position, position + forward(), glm::vec3(0, 1, 0));
}

glm::mat4 Camera::projMatrix() const {
  return glm::perspective(glm::radians(fovDegrees), aspect, nearPlane, farPlane);
}

} // namespace engine


