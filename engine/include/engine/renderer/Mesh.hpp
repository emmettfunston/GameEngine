#pragma once

#include <cstdint>
#include <vector>

#include <glm/vec3.hpp>

namespace engine {

struct Vertex {
  glm::vec3 pos;
  glm::vec3 normal;
};

class Mesh {
public:
  Mesh() = default;
  Mesh(const std::vector<Vertex>& vertices, const std::vector<std::uint32_t>& indices);
  ~Mesh();

  Mesh(const Mesh&) = delete;
  Mesh& operator=(const Mesh&) = delete;

  Mesh(Mesh&& other) noexcept;
  Mesh& operator=(Mesh&& other) noexcept;

  void draw() const;

  static Mesh makeCube();

private:
  void destroy();

  std::uint32_t m_vao = 0;
  std::uint32_t m_vbo = 0;
  std::uint32_t m_ebo = 0;
  std::int32_t m_indexCount = 0;
};

} // namespace engine


