#pragma once

#include <glm/vec3.hpp>

#include "engine/physics/PhysicsSystem.hpp"

namespace engine {

class Camera;
class Input;

class PlayerController {
public:
  PlayerController() = default;

  bool init(PhysicsSystem& physics, const glm::vec3& spawnPosition);

  void fixedUpdate(PhysicsSystem& physics,
                   const Input& input,
                   const Camera& camera,
                   float fixedDt);

  void updateCameraFromPlayer(PhysicsSystem& physics, Camera& camera) const;

  PhysicsBodyID bodyId() const { return m_bodyId; }

private:
  bool isGrounded(PhysicsSystem& physics) const;

  PhysicsBodyID m_bodyId{};
  float m_capsuleRadius = 0.35f;
  float m_capsuleHalfHeight = 0.55f;
  float m_moveSpeed = 6.0f;
  float m_sprintMultiplier = 1.6f;
  float m_crouchSpeedMultiplier = 0.5f;
  float m_groundAcceleration = 45.0f;
  float m_airAcceleration = 14.0f;
  float m_airControlSpeedMultiplier = 1.15f;
  float m_jumpSpeed = 6.25f;
  float m_gravity = -20.0f;
  float m_standingEyeOffset = 0.85f;
  float m_crouchingEyeOffset = 0.45f;
  float m_eyeOffset = 0.85f;
  float m_groundStickSpeed = 2.0f;
  bool m_jumpHeld = false;
};

} // namespace engine
