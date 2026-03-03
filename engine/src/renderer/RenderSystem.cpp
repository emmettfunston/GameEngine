#include "engine/renderer/RenderSystem.hpp"

#include "engine/core/Log.hpp"
#include "engine/renderer/Camera.hpp"
#include "engine/renderer/Mesh.hpp"
#include "engine/renderer/MeshLibrary.hpp"
#include "engine/scene/Scene.hpp"

#include <glad/gl.h>

#include <glm/gtc/matrix_transform.hpp>

namespace engine {

RenderSystem::RenderSystem() = default;

RenderSystem::~RenderSystem() {
  destroySkyResources();
}

void RenderSystem::init() {
  const std::string meshVs = std::string(ENGINE_ASSETS_DIR) + "/shaders/mesh.vert";
  const std::string meshFs = std::string(ENGINE_ASSETS_DIR) + "/shaders/mesh.frag";
  m_meshShader = Shader::fromFiles(meshVs, meshFs);

  const std::string skyVs = std::string(ENGINE_ASSETS_DIR) + "/shaders/sky.vert";
  const std::string skyFs = std::string(ENGINE_ASSETS_DIR) + "/shaders/sky.frag";
  m_skyShader = Shader::fromFiles(skyVs, skyFs);

  glGenVertexArrays(1, &m_skyVao);

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glFrontFace(GL_CCW);

  Log::info("RenderSystem initialized");
}

void RenderSystem::render(const Scene& scene, const Camera& camera, const DirectionalLight& light, const MeshLibrary& meshes) {
  renderSky(camera);

  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_TRUE);
  glClear(GL_DEPTH_BUFFER_BIT);

  m_meshShader.bind();
  m_meshShader.setMat4("uView", camera.viewMatrix());
  m_meshShader.setMat4("uProj", camera.projMatrix());
  m_meshShader.setVec3("uViewPos", camera.position);
  m_meshShader.setVec3("uLightDir", glm::normalize(light.direction));
  m_meshShader.setVec3("uLightColor", light.color);
  m_meshShader.setVec3("uFogColor", m_fogColor);
  m_meshShader.setFloat("uFogNear", 18.0f);
  m_meshShader.setFloat("uFogFar", 110.0f);

  const auto& entities = scene.meshRendererEntities();
  for (const Entity e : entities) {
    std::size_t trIdx = 0;
    std::size_t mrIdx = 0;
    if (!scene.tryGetTransformIndex(e, trIdx) || !scene.tryGetMeshRendererIndex(e, mrIdx)) {
      continue;
    }

    const Transform& tr = scene.transforms()[trIdx];
    const MeshRenderer& mr = scene.meshRenderers()[mrIdx];

    const Mesh* mesh = meshes.tryGet(mr.meshIndex);
    if (!mesh) {
      continue;
    }

    m_meshShader.setMat4("uModel", tr.matrix());
    m_meshShader.setVec3("uAlbedo", mr.albedo);
    m_meshShader.setInt("uEnableGroundGrid", mr.enableGroundGrid ? 1 : 0);

    mesh->draw();
  }
}

void RenderSystem::renderSky(const Camera& /*camera*/) {
  glDisable(GL_DEPTH_TEST);
  glDepthMask(GL_FALSE);
  glDisable(GL_CULL_FACE);

  m_skyShader.bind();
  m_skyShader.setVec3("uSkyBottom", m_skyBottom);
  m_skyShader.setVec3("uSkyTop", m_skyTop);

  glBindVertexArray(m_skyVao);
  glDrawArrays(GL_TRIANGLES, 0, 3);
  glBindVertexArray(0);

  glEnable(GL_CULL_FACE);
}

void RenderSystem::destroySkyResources() {
  if (m_skyVao) {
    glDeleteVertexArrays(1, &m_skyVao);
    m_skyVao = 0;
  }
}

} // namespace engine
