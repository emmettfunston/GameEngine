#pragma once

#include <cstdint>
#include <memory>

#include "engine/input/Input.hpp"
#include "engine/platform/Window.hpp"
#include "engine/renderer/MeshLibrary.hpp"
#include "engine/renderer/RenderSystem.hpp"

namespace engine {

class IApp;

struct EngineConfig {
  const char* title = "Engine Prototype";
  int windowWidth = 1280;
  int windowHeight = 720;
  bool highDpi = true;
  bool vsync = true;
  bool captureMouse = true;
};

class Engine {
public:
  explicit Engine(const EngineConfig& config = {});
  ~Engine();

  Engine(const Engine&) = delete;
  Engine& operator=(const Engine&) = delete;

  void run(IApp& app);
  void requestQuit();

  Window& window() { return *m_window; }
  const Window& window() const { return *m_window; }

  Input& input() { return m_input; }
  const Input& input() const { return m_input; }

  RenderSystem& renderer() { return m_renderer; }
  const RenderSystem& renderer() const { return m_renderer; }

  MeshLibrary& meshes() { return m_meshes; }
  const MeshLibrary& meshes() const { return m_meshes; }

  float smoothedFps() const { return m_smoothedFps; }

private:
  EngineConfig m_config{};
  bool m_running = false;

  std::unique_ptr<Window> m_window;
  Input m_input{};

  MeshLibrary m_meshes{};
  RenderSystem m_renderer{};

  float m_smoothedFps = 0.0f;
};

} // namespace engine


