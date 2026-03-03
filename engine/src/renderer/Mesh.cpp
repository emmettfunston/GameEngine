#include "engine/renderer/Mesh.hpp"

#include <glad/gl.h>

namespace engine {

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<std::uint32_t>& indices) {
  m_indexCount = static_cast<std::int32_t>(indices.size());

  glGenVertexArrays(1, &m_vao);
  glGenBuffers(1, &m_vbo);
  glGenBuffers(1, &m_ebo);

  glBindVertexArray(m_vao);

  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(Vertex)), vertices.data(),
               GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(indices.size() * sizeof(std::uint32_t)),
               indices.data(), GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(0));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(sizeof(glm::vec3)));

  glBindVertexArray(0);
}

Mesh::~Mesh() {
  destroy();
}

Mesh::Mesh(Mesh&& other) noexcept {
  *this = std::move(other);
}

Mesh& Mesh::operator=(Mesh&& other) noexcept {
  if (this == &other) {
    return *this;
  }
  destroy();
  m_vao = other.m_vao;
  m_vbo = other.m_vbo;
  m_ebo = other.m_ebo;
  m_indexCount = other.m_indexCount;
  other.m_vao = 0;
  other.m_vbo = 0;
  other.m_ebo = 0;
  other.m_indexCount = 0;
  return *this;
}

void Mesh::destroy() {
  if (m_ebo) glDeleteBuffers(1, &m_ebo);
  if (m_vbo) glDeleteBuffers(1, &m_vbo);
  if (m_vao) glDeleteVertexArrays(1, &m_vao);
  m_vao = 0;
  m_vbo = 0;
  m_ebo = 0;
  m_indexCount = 0;
}

void Mesh::draw() const {
  glBindVertexArray(m_vao);
  glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, nullptr);
  glBindVertexArray(0);
}

Mesh Mesh::makeCube() {
  // 24 verts (unique normals per face), 36 indices.
  const glm::vec3 p000{-0.5f, -0.5f, -0.5f};
  const glm::vec3 p001{-0.5f, -0.5f,  0.5f};
  const glm::vec3 p010{-0.5f,  0.5f, -0.5f};
  const glm::vec3 p011{-0.5f,  0.5f,  0.5f};
  const glm::vec3 p100{ 0.5f, -0.5f, -0.5f};
  const glm::vec3 p101{ 0.5f, -0.5f,  0.5f};
  const glm::vec3 p110{ 0.5f,  0.5f, -0.5f};
  const glm::vec3 p111{ 0.5f,  0.5f,  0.5f};

  std::vector<Vertex> v;
  v.reserve(24);

  // -X
  v.push_back({p000, {-1, 0, 0}});
  v.push_back({p001, {-1, 0, 0}});
  v.push_back({p011, {-1, 0, 0}});
  v.push_back({p010, {-1, 0, 0}});
  // +X
  v.push_back({p100, {1, 0, 0}});
  v.push_back({p110, {1, 0, 0}});
  v.push_back({p111, {1, 0, 0}});
  v.push_back({p101, {1, 0, 0}});
  // -Y
  v.push_back({p000, {0, -1, 0}});
  v.push_back({p100, {0, -1, 0}});
  v.push_back({p101, {0, -1, 0}});
  v.push_back({p001, {0, -1, 0}});
  // +Y
  v.push_back({p010, {0, 1, 0}});
  v.push_back({p011, {0, 1, 0}});
  v.push_back({p111, {0, 1, 0}});
  v.push_back({p110, {0, 1, 0}});
  // -Z
  v.push_back({p000, {0, 0, -1}});
  v.push_back({p010, {0, 0, -1}});
  v.push_back({p110, {0, 0, -1}});
  v.push_back({p100, {0, 0, -1}});
  // +Z
  v.push_back({p001, {0, 0, 1}});
  v.push_back({p101, {0, 0, 1}});
  v.push_back({p111, {0, 0, 1}});
  v.push_back({p011, {0, 0, 1}});

  std::vector<std::uint32_t> i;
  i.reserve(36);
  for (std::uint32_t face = 0; face < 6; ++face) {
    const std::uint32_t base = face * 4;
    // 0-1-2, 0-2-3 (CCW)
    i.push_back(base + 0);
    i.push_back(base + 1);
    i.push_back(base + 2);
    i.push_back(base + 0);
    i.push_back(base + 2);
    i.push_back(base + 3);
  }

  return Mesh(v, i);
}

} // namespace engine


