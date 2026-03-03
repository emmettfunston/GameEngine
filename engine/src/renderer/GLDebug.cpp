#include "engine/renderer/GLDebug.hpp"

#include "engine/core/Log.hpp"

#include <glad/gl.h>

namespace engine {
namespace {

void onGlDebugMessage(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei /*length*/,
                      const GLchar* message, const void* /*userParam*/) {
  (void)source;
  (void)type;
  (void)id;

  // Keep it simple: map severity to log level.
  LogLevel lvl = LogLevel::Info;
  if (severity == GL_DEBUG_SEVERITY_HIGH) lvl = LogLevel::Error;
  else if (severity == GL_DEBUG_SEVERITY_MEDIUM) lvl = LogLevel::Warn;
  else if (severity == GL_DEBUG_SEVERITY_LOW) lvl = LogLevel::Info;
  else if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) lvl = LogLevel::Debug;

  Log::write(lvl, message ? message : "(null gl debug message)");
}

} // namespace

void enableGLDebugCallback() {
  if (!GLAD_GL_KHR_debug) {
    Log::debug("GL_KHR_debug not available; skipping GL debug callback");
    return;
  }

  glEnable(GL_DEBUG_OUTPUT);
  glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
  glDebugMessageCallback(onGlDebugMessage, nullptr);

  // Reduce spam (keep medium+ by default).
  glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);

  Log::info("Enabled OpenGL debug callback");
}

} // namespace engine


