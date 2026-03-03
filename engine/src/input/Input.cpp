#include "engine/input/Input.hpp"

#include <SDL.h>

namespace engine {

void Input::beginFrame() {
  m_mouseDeltaX = 0.0f;
  m_mouseDeltaY = 0.0f;
  m_scrollY = 0.0f;
}

void Input::handleEvent(const SDL_Event& e) {
  switch (e.type) {
  case SDL_KEYDOWN:
  case SDL_KEYUP: {
    const auto sc = static_cast<std::size_t>(e.key.keysym.scancode);
    if (sc < m_keys.size()) {
      m_keys[sc] = (e.type == SDL_KEYDOWN) ? 1 : 0;
    }
  } break;
  case SDL_MOUSEBUTTONDOWN:
  case SDL_MOUSEBUTTONUP: {
    const auto b = static_cast<std::size_t>(e.button.button);
    if (b < m_mouseButtons.size()) {
      m_mouseButtons[b] = (e.type == SDL_MOUSEBUTTONDOWN) ? 1 : 0;
    }
  } break;
  case SDL_MOUSEMOTION:
    m_mouseDeltaX += static_cast<float>(e.motion.xrel);
    m_mouseDeltaY += static_cast<float>(e.motion.yrel);
    break;
  case SDL_MOUSEWHEEL:
    m_scrollY += static_cast<float>(e.wheel.y);
    break;
  default:
    break;
  }
}

bool Input::keyDown(int sdlScancode) const {
  const auto sc = static_cast<std::size_t>(sdlScancode);
  if (sc >= m_keys.size()) {
    return false;
  }
  return m_keys[sc] != 0;
}

bool Input::mouseButtonDown(int sdlButton) const {
  const auto b = static_cast<std::size_t>(sdlButton);
  if (b >= m_mouseButtons.size()) {
    return false;
  }
  return m_mouseButtons[b] != 0;
}

} // namespace engine


