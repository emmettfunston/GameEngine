#pragma once

#include <memory>
#include <unordered_map>

#include <glm/vec3.hpp>

#include "engine/physics/PhysicsTypes.hpp"
#include "engine/scene/Scene.hpp"

namespace JPH {
class TempAllocatorImpl;
class JobSystemThreadPool;
class PhysicsSystem;
class BodyInterface;
class BodyID;
}

namespace engine {

class PhysicsSystem {
public:
  PhysicsSystem();
  ~PhysicsSystem();

  PhysicsSystem(const PhysicsSystem&) = delete;
  PhysicsSystem& operator=(const PhysicsSystem&) = delete;

  bool init();
  void shutdown();

  void step(float fixedDt);

  PhysicsBodyID createStaticBox(const Transform& transform, const glm::vec3& halfExtents, PhysicsLayer layer);
  PhysicsBodyID createDynamicBox(const Transform& transform, const glm::vec3& halfExtents, float mass, PhysicsLayer layer);
  PhysicsBodyID createPlayerCapsule(const glm::vec3& position, float radius, float halfHeight);

  RaycastHit raycast(const glm::vec3& origin, const glm::vec3& dir, float maxDist, LayerMask mask) const;

  void bindEntityToBody(Entity entity, PhysicsBodyID bodyId);
  void syncTransformsToScene(Scene& scene);
  void syncSceneToPhysics(const Scene& scene);

  glm::vec3 bodyPosition(PhysicsBodyID bodyId) const;
  glm::vec3 bodyLinearVelocity(PhysicsBodyID bodyId) const;
  void setBodyLinearVelocity(PhysicsBodyID bodyId, const glm::vec3& linearVelocity);
  void setBodyRotationIdentity(PhysicsBodyID bodyId);

private:
  class BroadPhaseLayerInterfaceImpl;
  class ObjectVsBroadPhaseLayerFilterImpl;
  class ObjectLayerPairFilterImpl;

  PhysicsBodyID createBoxInternal(const Transform& transform,
                                  const glm::vec3& halfExtents,
                                  float mass,
                                  bool dynamic,
                                  PhysicsLayer layer);

  bool isInitialized() const;
  PhysicsLayer bodyLayer(PhysicsBodyID bodyId) const;

  static std::uint32_t toBodyKey(const JPH::BodyID& id);
  static PhysicsBodyID toPublicBodyId(const JPH::BodyID& id);

  JPH::BodyID toJoltBodyId(PhysicsBodyID bodyId) const;

  std::unique_ptr<BroadPhaseLayerInterfaceImpl> m_broadPhaseLayerInterface;
  std::unique_ptr<ObjectVsBroadPhaseLayerFilterImpl> m_objectVsBroadPhaseLayerFilter;
  std::unique_ptr<ObjectLayerPairFilterImpl> m_objectLayerPairFilter;

  std::unique_ptr<JPH::TempAllocatorImpl> m_tempAllocator;
  std::unique_ptr<JPH::JobSystemThreadPool> m_jobSystem;
  std::unique_ptr<JPH::PhysicsSystem> m_system;

  std::unordered_map<Entity, PhysicsBodyID> m_entityToBody;
  std::unordered_map<std::uint32_t, Entity> m_bodyToEntity;

  bool m_started = false;
};

} // namespace engine
