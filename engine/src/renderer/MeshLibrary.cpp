#include "engine/renderer/MeshLibrary.hpp"

namespace engine {

std::uint32_t MeshLibrary::add(Mesh&& mesh) {
  const std::uint32_t idx = static_cast<std::uint32_t>(m_meshes.size());
  m_meshes.emplace_back(std::move(mesh));
  return idx;
}

const Mesh* MeshLibrary::tryGet(std::uint32_t index) const {
  if (index >= m_meshes.size()) {
    return nullptr;
  }
  return &m_meshes[index];
}

} // namespace engine


