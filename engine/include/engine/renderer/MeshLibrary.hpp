#pragma once

#include <cstdint>
#include <vector>

#include "engine/renderer/Mesh.hpp"

namespace engine {

class MeshLibrary {
public:
  std::uint32_t add(Mesh&& mesh);
  const Mesh* tryGet(std::uint32_t index) const;

private:
  std::vector<Mesh> m_meshes;
};

} // namespace engine


