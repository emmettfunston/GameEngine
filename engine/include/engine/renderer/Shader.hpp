#pragma once

#include <cstdint>
#include <string>

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

namespace engine {

class Shader {
public:
  Shader() = default;
  ~Shader();

  Shader(const Shader&) = delete;
  Shader& operator=(const Shader&) = delete;

  Shader(Shader&& other) noexcept;
  Shader& operator=(Shader&& other) noexcept;

  static Shader fromFiles(const std::string& vertexPath, const std::string& fragmentPath);

  void bind() const;
  std::uint32_t program() const { return m_program; }

  void setMat4(const char* name, const glm::mat4& v) const;
  void setVec3(const char* name, const glm::vec3& v) const;
  void setFloat(const char* name, float v) const;
  void setInt(const char* name, int v) const;

private:
  static std::string readTextFile(const std::string& path);
  static std::uint32_t compileStage(std::uint32_t type, const std::string& src);

  std::uint32_t m_program = 0;
};

} // namespace engine
