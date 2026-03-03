#pragma once

#include <array>
#include <cstdint>

union SDL_Event;

namespace engine {

class Input {
public:
  void beginFrame();
  void handleEvent(const SDL_Event& e);

  bool keyDown(int sdlScancode) const;
  bool mouseButtonDown(int sdlButton) const;

  float mouseDeltaX() const { return m_mouseDeltaX; }
  float mouseDeltaY() const { return m_mouseDeltaY; }
  float scrollY() const { return m_scrollY; }

private:
  // SDL scancodes are < 512.
  std::array<std::uint8_t, 512> m_keys{};
  std::array<std::uint8_t, 8> m_mouseButtons{};

  float m_mouseDeltaX = 0.0f;
  float m_mouseDeltaY = 0.0f;
  float m_scrollY = 0.0f;
};

} // namespace engine


