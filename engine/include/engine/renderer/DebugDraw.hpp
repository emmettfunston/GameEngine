#pragma once

#include <cstdint>
#include <vector>

#include <glm/vec3.hpp>

#include "engine/renderer/Shader.hpp"

namespace engine {

class Camera;

struct DebugLine {
  glm::vec3 from{0.0f};
  glm::vec3 to{0.0f};
  glm::vec3 color{1.0f};
};

class DebugDraw {
public:
  DebugDraw() = default;
  ~DebugDraw();

  DebugDraw(const DebugDraw&) = delete;
  DebugDraw& operator=(const DebugDraw&) = delete;

  DebugDraw(DebugDraw&&) = delete;
  DebugDraw& operator=(DebugDraw&&) = delete;

  void init();
  void line(const glm::vec3& from, const glm::vec3& to, const glm::vec3& color);
  void aabb(const glm::vec3& min, const glm::vec3& max, const glm::vec3& color);
  void render(const Camera& camera);
  void clear();

private:
  struct LineVertex {
    glm::vec3 pos{0.0f};
    glm::vec3 color{1.0f};
  };

  void destroyGl();

  std::vector<DebugLine> m_lines;
  std::vector<LineVertex> m_uploadVertices;

  Shader m_shader;
  std::uint32_t m_vao = 0;
  std::uint32_t m_vbo = 0;
  bool m_initialized = false;
};

} // namespace engine
