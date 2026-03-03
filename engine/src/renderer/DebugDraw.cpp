#include "engine/renderer/DebugDraw.hpp"

#include "engine/renderer/Camera.hpp"

#include <glad/gl.h>

namespace engine {

DebugDraw::~DebugDraw() {
  destroyGl();
}

void DebugDraw::init() {
  if (m_initialized) {
    return;
  }

  const std::string vs = std::string(ENGINE_ASSETS_DIR) + "/shaders/debug_line.vert";
  const std::string fs = std::string(ENGINE_ASSETS_DIR) + "/shaders/debug_line.frag";
  m_shader = Shader::fromFiles(vs, fs);

  glGenVertexArrays(1, &m_vao);
  glGenBuffers(1, &m_vbo);

  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(LineVertex) * 2), nullptr, GL_DYNAMIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(LineVertex), reinterpret_cast<void*>(0));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(LineVertex), reinterpret_cast<void*>(sizeof(glm::vec3)));

  glBindVertexArray(0);
  m_initialized = true;
}

void DebugDraw::line(const glm::vec3& from, const glm::vec3& to, const glm::vec3& color) {
  m_lines.push_back(DebugLine{from, to, color});
}

void DebugDraw::aabb(const glm::vec3& min, const glm::vec3& max, const glm::vec3& color) {
  const glm::vec3 p000{min.x, min.y, min.z};
  const glm::vec3 p001{min.x, min.y, max.z};
  const glm::vec3 p010{min.x, max.y, min.z};
  const glm::vec3 p011{min.x, max.y, max.z};
  const glm::vec3 p100{max.x, min.y, min.z};
  const glm::vec3 p101{max.x, min.y, max.z};
  const glm::vec3 p110{max.x, max.y, min.z};
  const glm::vec3 p111{max.x, max.y, max.z};

  line(p000, p001, color);
  line(p001, p011, color);
  line(p011, p010, color);
  line(p010, p000, color);

  line(p100, p101, color);
  line(p101, p111, color);
  line(p111, p110, color);
  line(p110, p100, color);

  line(p000, p100, color);
  line(p001, p101, color);
  line(p010, p110, color);
  line(p011, p111, color);
}

void DebugDraw::render(const Camera& camera) {
  if (!m_initialized || m_lines.empty()) {
    return;
  }

  m_uploadVertices.clear();
  m_uploadVertices.reserve(m_lines.size() * 2);

  for (const DebugLine& lineDef : m_lines) {
    m_uploadVertices.push_back(LineVertex{lineDef.from, lineDef.color});
    m_uploadVertices.push_back(LineVertex{lineDef.to, lineDef.color});
  }

  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBufferData(GL_ARRAY_BUFFER,
               static_cast<GLsizeiptr>(m_uploadVertices.size() * sizeof(LineVertex)),
               m_uploadVertices.data(),
               GL_DYNAMIC_DRAW);

  m_shader.bind();
  m_shader.setMat4("uView", camera.viewMatrix());
  m_shader.setMat4("uProj", camera.projMatrix());

  const GLboolean wasDepthTest = glIsEnabled(GL_DEPTH_TEST);
  glEnable(GL_DEPTH_TEST);
  glBindVertexArray(m_vao);
  glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(m_uploadVertices.size()));
  glBindVertexArray(0);
  if (!wasDepthTest) {
    glDisable(GL_DEPTH_TEST);
  }

  clear();
}

void DebugDraw::clear() {
  m_lines.clear();
}

void DebugDraw::destroyGl() {
  if (m_vbo) {
    glDeleteBuffers(1, &m_vbo);
    m_vbo = 0;
  }
  if (m_vao) {
    glDeleteVertexArrays(1, &m_vao);
    m_vao = 0;
  }
  m_initialized = false;
}

} // namespace engine
