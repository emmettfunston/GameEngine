#pragma once

#include <cstdint>

struct SDL_Window;
using SDL_GLContext = void*;

namespace engine {

struct WindowConfig {
  const char* title = "Engine Prototype";
  int width = 1280;
  int height = 720;
  bool highDpi = true;
};

class Window {
public:
  explicit Window(const WindowConfig& config);
  ~Window();

  Window(const Window&) = delete;
  Window& operator=(const Window&) = delete;

  void swap();
  void setVsync(bool enabled);
  void setCaptureMouse(bool enabled);

  SDL_Window* sdlWindow() const { return m_window; }
  SDL_GLContext glContext() const { return m_gl; }

  // Drawable size (HiDPI-aware). This is what glViewport should use.
  void drawableSize(int& outW, int& outH) const;

private:
  SDL_Window* m_window = nullptr;
  SDL_GLContext m_gl = nullptr;
};

} // namespace engine


