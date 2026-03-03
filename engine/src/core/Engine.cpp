#include "engine/core/Engine.hpp"

#include "engine/core/IApp.hpp"
#include "engine/core/Log.hpp"
#include "engine/core/Time.hpp"
#include "engine/renderer/GLDebug.hpp"

#include <SDL.h>
#include <glad/gl.h>

#include <algorithm>
#include <stdexcept>

namespace engine {
namespace {

static void throwIf(bool cond, const char* what) {
  if (cond) {
    throw std::runtime_error(what);
  }
}

} // namespace

Engine::Engine(const EngineConfig& config) : m_config(config) {
  WindowConfig wc;
  wc.title = config.title;
  wc.width = config.windowWidth;
  wc.height = config.windowHeight;
  wc.highDpi = config.highDpi;
  m_window = std::make_unique<Window>(wc);

  // Load OpenGL via glad using SDL's function loader.
  const int loaded = gladLoadGL(reinterpret_cast<GLADloadfunc>(SDL_GL_GetProcAddress));
  throwIf(loaded == 0, "gladLoadGL failed");

  const char* ver = reinterpret_cast<const char*>(glGetString(GL_VERSION));
  Log::info(ver ? ver : "GL_VERSION unavailable");

#ifndef NDEBUG
  enableGLDebugCallback();
#endif

  m_window->setVsync(config.vsync);
  m_window->setCaptureMouse(config.captureMouse);

  int dw = 0;
  int dh = 0;
  m_window->drawableSize(dw, dh);
  glViewport(0, 0, dw, dh);

  m_renderer.init();
}

Engine::~Engine() = default;

void Engine::requestQuit() {
  m_running = false;
}

void Engine::run(IApp& app) {
  m_running = true;

  app.onInit(*this);

  Time t;
  t.reset();

  constexpr float fixedDt = 1.0f / 60.0f;
  float accumulator = 0.0f;

  while (m_running) {
    const float rawDt = t.tickSeconds();
    const float dt = std::clamp(rawDt, 0.0f, 0.25f);
    accumulator += dt;

    if (dt > 0.0f) {
      const float fps = 1.0f / dt;
      const float alpha = 0.1f; // EMA smoothing
      m_smoothedFps = (m_smoothedFps == 0.0f) ? fps : (m_smoothedFps * (1.0f - alpha) + fps * alpha);
    }

    m_input.beginFrame();

    SDL_Event e;
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT) {
        requestQuit();
      }
      if (e.type == SDL_KEYDOWN && e.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
        // Toggle capture.
        m_config.captureMouse = !m_config.captureMouse;
        m_window->setCaptureMouse(m_config.captureMouse);
      }
      if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
        int dw = 0;
        int dh = 0;
        m_window->drawableSize(dw, dh);
        glViewport(0, 0, dw, dh);
      }

      m_input.handleEvent(e);
    }

    app.onUpdate(*this, dt);

    while (accumulator >= fixedDt) {
      app.onFixedUpdate(*this, fixedDt);
      accumulator -= fixedDt;
    }

    app.onRender(*this);
    m_window->swap();
  }

  app.onShutdown(*this);
}

} // namespace engine


