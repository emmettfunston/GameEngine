#include "engine/scene/Scene.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace engine {

glm::mat4 Transform::matrix() const {
  glm::mat4 m(1.0f);
  m = glm::translate(m, position);
  m = glm::rotate(m, glm::radians(rotationEulerDegrees.y), glm::vec3(0, 1, 0));
  m = glm::rotate(m, glm::radians(rotationEulerDegrees.x), glm::vec3(1, 0, 0));
  m = glm::rotate(m, glm::radians(rotationEulerDegrees.z), glm::vec3(0, 0, 1));
  m = glm::scale(m, scale);
  return m;
}

Entity Scene::createEntity() {
  return m_next++;
}

Transform& Scene::addTransform(Entity e) {
  const auto it = m_transformIndex.find(e);
  if (it != m_transformIndex.end()) {
    return m_transforms[it->second];
  }
  const std::size_t idx = m_transforms.size();
  m_transformEntities.push_back(e);
  m_transforms.emplace_back();
  m_transformIndex.emplace(e, idx);
  return m_transforms[idx];
}

MeshRenderer& Scene::addMeshRenderer(Entity e) {
  const auto it = m_meshRendererIndex.find(e);
  if (it != m_meshRendererIndex.end()) {
    return m_meshRenderers[it->second];
  }
  const std::size_t idx = m_meshRenderers.size();
  m_meshRendererEntities.push_back(e);
  m_meshRenderers.emplace_back();
  m_meshRendererIndex.emplace(e, idx);
  return m_meshRenderers[idx];
}

Transform* Scene::tryGetTransform(Entity e) {
  const auto it = m_transformIndex.find(e);
  if (it == m_transformIndex.end()) return nullptr;
  return &m_transforms[it->second];
}

const Transform* Scene::tryGetTransform(Entity e) const {
  const auto it = m_transformIndex.find(e);
  if (it == m_transformIndex.end()) return nullptr;
  return &m_transforms[it->second];
}

MeshRenderer* Scene::tryGetMeshRenderer(Entity e) {
  const auto it = m_meshRendererIndex.find(e);
  if (it == m_meshRendererIndex.end()) return nullptr;
  return &m_meshRenderers[it->second];
}

const MeshRenderer* Scene::tryGetMeshRenderer(Entity e) const {
  const auto it = m_meshRendererIndex.find(e);
  if (it == m_meshRendererIndex.end()) return nullptr;
  return &m_meshRenderers[it->second];
}

bool Scene::tryGetTransformIndex(Entity e, std::size_t& outIndex) const {
  const auto it = m_transformIndex.find(e);
  if (it == m_transformIndex.end()) return false;
  outIndex = it->second;
  return true;
}

bool Scene::tryGetMeshRendererIndex(Entity e, std::size_t& outIndex) const {
  const auto it = m_meshRendererIndex.find(e);
  if (it == m_meshRendererIndex.end()) return false;
  outIndex = it->second;
  return true;
}

} // namespace engine
