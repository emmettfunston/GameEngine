#include "engine/platform/Window.hpp"

#include "engine/core/Log.hpp"

#include <SDL.h>

#include <string>
#include <stdexcept>

namespace engine {

static void throwSdl(const char* what) {
  const char* err = SDL_GetError();
  std::string msg = std::string(what) + ": " + (err ? err : "unknown SDL error");
  throw std::runtime_error(msg);
}

Window::Window(const WindowConfig& config) {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
    throwSdl("SDL_Init failed");
  }

  // macOS OpenGL ceiling is 4.1; request core + forward-compatible.
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);

  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

  std::uint32_t flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
  if (config.highDpi) {
    flags |= SDL_WINDOW_ALLOW_HIGHDPI;
  }

  m_window = SDL_CreateWindow(
      config.title,
      SDL_WINDOWPOS_CENTERED,
      SDL_WINDOWPOS_CENTERED,
      config.width,
      config.height,
      flags);
  if (!m_window) {
    throwSdl("SDL_CreateWindow failed");
  }

  m_gl = SDL_GL_CreateContext(m_window);
  if (!m_gl) {
    throwSdl("SDL_GL_CreateContext failed");
  }

  // On macOS, the context is current on creation, but we make it explicit.
  if (SDL_GL_MakeCurrent(m_window, m_gl) != 0) {
    throwSdl("SDL_GL_MakeCurrent failed");
  }

  Log::info("SDL window + OpenGL context created");
}

Window::~Window() {
  if (m_gl) {
    SDL_GL_DeleteContext(m_gl);
    m_gl = nullptr;
  }
  if (m_window) {
    SDL_DestroyWindow(m_window);
    m_window = nullptr;
  }
  SDL_Quit();
}

void Window::swap() {
  SDL_GL_SwapWindow(m_window);
}

void Window::setVsync(bool enabled) {
  // 1 = enable, 0 = disable
  SDL_GL_SetSwapInterval(enabled ? 1 : 0);
}

void Window::setCaptureMouse(bool enabled) {
  SDL_SetRelativeMouseMode(enabled ? SDL_TRUE : SDL_FALSE);
}

void Window::drawableSize(int& outW, int& outH) const {
  SDL_GL_GetDrawableSize(m_window, &outW, &outH);
}

} // namespace engine


