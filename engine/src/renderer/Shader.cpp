#include "engine/renderer/Shader.hpp"

#include "engine/core/Log.hpp"

#include <glad/gl.h>

#include <fstream>
#include <sstream>
#include <stdexcept>

namespace engine {
namespace {

std::int32_t uniformLoc(std::uint32_t program, const char* name) {
  return glGetUniformLocation(program, name);
}

void throwWithInfoLog(std::uint32_t object, bool isProgram, const char* stageLabel) {
  std::int32_t len = 0;
  if (isProgram) {
    glGetProgramiv(object, GL_INFO_LOG_LENGTH, &len);
  } else {
    glGetShaderiv(object, GL_INFO_LOG_LENGTH, &len);
  }

  std::string log;
  log.resize(static_cast<std::size_t>(len > 0 ? len : 1));

  if (isProgram) {
    glGetProgramInfoLog(object, len, nullptr, log.data());
  } else {
    glGetShaderInfoLog(object, len, nullptr, log.data());
  }

  throw std::runtime_error(std::string(stageLabel) + " failed:\n" + log);
}

} // namespace

Shader::~Shader() {
  if (m_program) {
    glDeleteProgram(m_program);
    m_program = 0;
  }
}

Shader::Shader(Shader&& other) noexcept {
  m_program = other.m_program;
  other.m_program = 0;
}

Shader& Shader::operator=(Shader&& other) noexcept {
  if (this == &other) {
    return *this;
  }
  if (m_program) {
    glDeleteProgram(m_program);
  }
  m_program = other.m_program;
  other.m_program = 0;
  return *this;
}

std::string Shader::readTextFile(const std::string& path) {
  std::ifstream f(path);
  if (!f) {
    throw std::runtime_error("Failed to open file: " + path);
  }
  std::stringstream ss;
  ss << f.rdbuf();
  return ss.str();
}

std::uint32_t Shader::compileStage(std::uint32_t type, const std::string& src) {
  const char* cstr = src.c_str();
  std::uint32_t s = glCreateShader(type);
  glShaderSource(s, 1, &cstr, nullptr);
  glCompileShader(s);

  std::int32_t ok = 0;
  glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
  if (!ok) {
    throwWithInfoLog(s, false, (type == GL_VERTEX_SHADER) ? "Vertex shader compile" : "Fragment shader compile");
  }
  return s;
}

Shader Shader::fromFiles(const std::string& vertexPath, const std::string& fragmentPath) {
  Shader sh;

  const std::string vsSrc = readTextFile(vertexPath);
  const std::string fsSrc = readTextFile(fragmentPath);

  const std::uint32_t vs = compileStage(GL_VERTEX_SHADER, vsSrc);
  const std::uint32_t fs = compileStage(GL_FRAGMENT_SHADER, fsSrc);

  sh.m_program = glCreateProgram();
  glAttachShader(sh.m_program, vs);
  glAttachShader(sh.m_program, fs);
  glLinkProgram(sh.m_program);

  glDeleteShader(vs);
  glDeleteShader(fs);

  std::int32_t ok = 0;
  glGetProgramiv(sh.m_program, GL_LINK_STATUS, &ok);
  if (!ok) {
    throwWithInfoLog(sh.m_program, true, "Program link");
  }

  const std::string msg = "Linked shader program: " + vertexPath + " + " + fragmentPath;
  Log::info(msg);
  return sh;
}

void Shader::bind() const {
  glUseProgram(m_program);
}

void Shader::setMat4(const char* name, const glm::mat4& v) const {
  const std::int32_t loc = uniformLoc(m_program, name);
  glUniformMatrix4fv(loc, 1, GL_FALSE, &v[0][0]);
}

void Shader::setVec3(const char* name, const glm::vec3& v) const {
  const std::int32_t loc = uniformLoc(m_program, name);
  glUniform3f(loc, v.x, v.y, v.z);
}

void Shader::setFloat(const char* name, float v) const {
  const std::int32_t loc = uniformLoc(m_program, name);
  glUniform1f(loc, v);
}

void Shader::setInt(const char* name, int v) const {
  const std::int32_t loc = uniformLoc(m_program, name);
  glUniform1i(loc, v);
}

} // namespace engine
