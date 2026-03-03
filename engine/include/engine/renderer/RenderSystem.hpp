#pragma once

#include <cstdint>

#include <glm/vec3.hpp>

#include "engine/renderer/Shader.hpp"

namespace engine {

class Camera;
class MeshLibrary;
class Scene;

struct DirectionalLight {
  glm::vec3 direction = glm::normalize(glm::vec3(-1.0f, -1.0f, -0.3f));
  glm::vec3 color = glm::vec3(1.0f);
};

class RenderSystem {
public:
  RenderSystem();
  ~RenderSystem();

  RenderSystem(const RenderSystem&) = delete;
  RenderSystem& operator=(const RenderSystem&) = delete;

  void init();
  void render(const Scene& scene, const Camera& camera, const DirectionalLight& light, const MeshLibrary& meshes);

private:
  void renderSky(const Camera& camera);
  void destroySkyResources();

  Shader m_meshShader;
  Shader m_skyShader;
  std::uint32_t m_skyVao = 0;

  glm::vec3 m_skyBottom{0.72f, 0.83f, 0.95f};
  glm::vec3 m_skyTop{0.20f, 0.53f, 0.90f};
  glm::vec3 m_fogColor{0.68f, 0.79f, 0.90f};
};

} // namespace engine
