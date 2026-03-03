#pragma once

namespace engine {

class Engine;

class IApp {
public:
  virtual ~IApp() = default;

  virtual void onInit(Engine& engine) = 0;
  virtual void onShutdown(Engine& engine) = 0;

  // Variable timestep (once per rendered frame).
  virtual void onUpdate(Engine& engine, float dtSeconds) = 0;

  // Fixed timestep (e.g. 60 Hz), can run 0..N times per frame.
  virtual void onFixedUpdate(Engine& engine, float fixedDtSeconds) = 0;

  virtual void onRender(Engine& engine) = 0;
};

} // namespace engine


