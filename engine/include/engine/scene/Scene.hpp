#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

namespace engine {

using Entity = std::uint32_t;

struct Transform {
  glm::vec3 position{0.0f};
  glm::vec3 rotationEulerDegrees{0.0f}; // pitch, yaw, roll (degrees)
  glm::vec3 scale{1.0f};

  glm::mat4 matrix() const;
};

struct MeshRenderer {
  std::uint32_t meshIndex = 0;
  glm::vec3 albedo{0.8f, 0.4f, 0.2f};
  glm::vec3 localAabbMin{-0.5f, -0.5f, -0.5f};
  glm::vec3 localAabbMax{0.5f, 0.5f, 0.5f};
  bool enableGroundGrid = false;
};

class Scene {
public:
  Entity createEntity();

  Transform& addTransform(Entity e);
  MeshRenderer& addMeshRenderer(Entity e);

  Transform* tryGetTransform(Entity e);
  const Transform* tryGetTransform(Entity e) const;

  MeshRenderer* tryGetMeshRenderer(Entity e);
  const MeshRenderer* tryGetMeshRenderer(Entity e) const;

  const std::vector<Entity>& meshRendererEntities() const { return m_meshRendererEntities; }
  const std::vector<Transform>& transforms() const { return m_transforms; }
  std::vector<Transform>& transformsMutable() { return m_transforms; }
  const std::vector<MeshRenderer>& meshRenderers() const { return m_meshRenderers; }
  std::vector<MeshRenderer>& meshRenderersMutable() { return m_meshRenderers; }

  bool tryGetTransformIndex(Entity e, std::size_t& outIndex) const;
  bool tryGetMeshRendererIndex(Entity e, std::size_t& outIndex) const;

private:
  Entity m_next = 1;

  std::vector<Entity> m_transformEntities;
  std::vector<Transform> m_transforms;
  std::unordered_map<Entity, std::size_t> m_transformIndex;

  std::vector<Entity> m_meshRendererEntities;
  std::vector<MeshRenderer> m_meshRenderers;
  std::unordered_map<Entity, std::size_t> m_meshRendererIndex;
};

} // namespace engine
