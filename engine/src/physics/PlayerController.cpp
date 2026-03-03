#include "engine/physics/PlayerController.hpp"

#include "engine/input/Input.hpp"
#include "engine/renderer/Camera.hpp"

#include <SDL.h>

#include <glm/geometric.hpp>
#include <glm/common.hpp>
#include <glm/trigonometric.hpp>

namespace engine {

bool PlayerController::init(PhysicsSystem& physics, const glm::vec3& spawnPosition) {
  m_bodyId = physics.createPlayerCapsule(spawnPosition, m_capsuleRadius, m_capsuleHalfHeight);
  return m_bodyId.isValid();
}

void PlayerController::fixedUpdate(PhysicsSystem& physics,
                                   const Input& input,
                                   const Camera& camera,
                                   float fixedDt) {
  if (!m_bodyId.isValid()) {
    return;
  }

  const float yaw = glm::radians(camera.yawDegrees);
  glm::vec3 fwd = glm::normalize(glm::vec3(std::cos(yaw), 0.0f, std::sin(yaw)));
  glm::vec3 right = glm::normalize(glm::cross(fwd, glm::vec3(0.0f, 1.0f, 0.0f)));

  glm::vec3 wish(0.0f);
  if (input.keyDown(SDL_SCANCODE_W)) wish += fwd;
  if (input.keyDown(SDL_SCANCODE_S)) wish -= fwd;
  if (input.keyDown(SDL_SCANCODE_D)) wish += right;
  if (input.keyDown(SDL_SCANCODE_A)) wish -= right;

  if (glm::dot(wish, wish) > 1.0e-6f) {
    wish = glm::normalize(wish);
  }

  const bool crouching = input.keyDown(SDL_SCANCODE_LCTRL) || input.keyDown(SDL_SCANCODE_RCTRL) ||
                         input.keyDown(SDL_SCANCODE_C);
  const bool sprinting = !crouching &&
                         (input.keyDown(SDL_SCANCODE_LSHIFT) || input.keyDown(SDL_SCANCODE_RSHIFT));

  const float targetEyeOffset = crouching ? m_crouchingEyeOffset : m_standingEyeOffset;
  m_eyeOffset += (targetEyeOffset - m_eyeOffset) * 0.25f;

  float speed = m_moveSpeed;
  if (sprinting) speed *= m_sprintMultiplier;
  if (crouching) speed *= m_crouchSpeedMultiplier;

  glm::vec3 velocity = physics.bodyLinearVelocity(m_bodyId);

  const bool grounded = isGrounded(physics);

  glm::vec2 horizVelocity(velocity.x, velocity.z);
  glm::vec2 targetHorizVelocity(0.0f);
  if (glm::dot(wish, wish) > 1.0e-6f) {
    const float wishSpeed = grounded ? speed : (speed * m_airControlSpeedMultiplier);
    targetHorizVelocity = glm::vec2(wish.x, wish.z) * wishSpeed;
  } else if (!grounded) {
    // Keep airborne momentum when there is no input.
    targetHorizVelocity = horizVelocity;
  }

  const float accel = grounded ? m_groundAcceleration : m_airAcceleration;
  const float maxDelta = accel * fixedDt;
  const glm::vec2 delta = targetHorizVelocity - horizVelocity;
  const float deltaLen = glm::length(delta);
  if (deltaLen > maxDelta && deltaLen > 1.0e-6f) {
    horizVelocity += (delta / deltaLen) * maxDelta;
  } else {
    horizVelocity = targetHorizVelocity;
  }

  velocity.x = horizVelocity.x;
  velocity.z = horizVelocity.y;

  const bool jumpNow = input.keyDown(SDL_SCANCODE_SPACE);
  if (jumpNow && !m_jumpHeld && grounded) {
    velocity.y = m_jumpSpeed;
  } else if (!grounded) {
    velocity.y += m_gravity * fixedDt;
  } else {
    // Keep light downward velocity while grounded to avoid micro-bounces/float.
    velocity.y = -m_groundStickSpeed;
  }
  m_jumpHeld = jumpNow;

  physics.setBodyRotationIdentity(m_bodyId);
  physics.setBodyLinearVelocity(m_bodyId, velocity);
}

void PlayerController::updateCameraFromPlayer(PhysicsSystem& physics, Camera& camera) const {
  if (!m_bodyId.isValid()) {
    return;
  }
  const glm::vec3 p = physics.bodyPosition(m_bodyId);
  camera.position = glm::vec3(p.x, p.y + m_eyeOffset, p.z);
}

bool PlayerController::isGrounded(PhysicsSystem& physics) const {
  const glm::vec3 p = physics.bodyPosition(m_bodyId);
  const float footOffset = m_capsuleHalfHeight + m_capsuleRadius;
  const glm::vec3 origin = glm::vec3(p.x, p.y - footOffset + 0.05f, p.z);
  const RaycastHit hit = physics.raycast(origin,
                                         glm::vec3(0.0f, -1.0f, 0.0f),
                                         0.15f,
                                         toLayerMask(PhysicsLayer::WorldStatic) | toLayerMask(PhysicsLayer::WorldDynamic));
  return hit.hit;
}

} // namespace engine
