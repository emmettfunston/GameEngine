#include <engine/core/Engine.hpp>
#include <engine/core/IApp.hpp>
#include <engine/core/Log.hpp>

#include <engine/input/Input.hpp>
#include <engine/physics/PhysicsSystem.hpp>
#include <engine/physics/PlayerController.hpp>
#include <engine/renderer/Camera.hpp>
#include <engine/renderer/DebugDraw.hpp>
#include <engine/renderer/Mesh.hpp>
#include <engine/renderer/RenderSystem.hpp>
#include <engine/scene/Scene.hpp>

#include <SDL.h>

#include <glm/common.hpp>
#include <glm/geometric.hpp>

#include <algorithm>
#include <array>
#include <limits>
#include <optional>
#include <unordered_map>
#include <stdexcept>

#if ENGINE_ENABLE_IMGUI
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_sdl2.h>
#include <imgui.h>
#endif

namespace {

struct Ray {
  glm::vec3 origin{0.0f};
  glm::vec3 direction{0.0f, 0.0f, -1.0f};
};

void computeWorldAabb(const engine::Transform& tr,
                      const engine::MeshRenderer& mr,
                      glm::vec3& outMin,
                      glm::vec3& outMax) {
  const glm::mat4 m = tr.matrix();

  const glm::vec3 mn = mr.localAabbMin;
  const glm::vec3 mx = mr.localAabbMax;
  const std::array<glm::vec3, 8> corners = {
      glm::vec3(mn.x, mn.y, mn.z),
      glm::vec3(mx.x, mn.y, mn.z),
      glm::vec3(mn.x, mx.y, mn.z),
      glm::vec3(mx.x, mx.y, mn.z),
      glm::vec3(mn.x, mn.y, mx.z),
      glm::vec3(mx.x, mn.y, mx.z),
      glm::vec3(mn.x, mx.y, mx.z),
      glm::vec3(mx.x, mx.y, mx.z),
  };

  outMin = glm::vec3(std::numeric_limits<float>::max());
  outMax = glm::vec3(std::numeric_limits<float>::lowest());

  for (const glm::vec3& c : corners) {
    const glm::vec4 w = m * glm::vec4(c, 1.0f);
    outMin = glm::min(outMin, glm::vec3(w));
    outMax = glm::max(outMax, glm::vec3(w));
  }
}

bool rayIntersectsAabb(const Ray& ray,
                       const glm::vec3& aabbMin,
                       const glm::vec3& aabbMax,
                       float& outT) {
  float tMin = 0.0f;
  float tMax = std::numeric_limits<float>::max();

  for (int axis = 0; axis < 3; ++axis) {
    const float origin = ray.origin[axis];
    const float dir = ray.direction[axis];
    const float mn = aabbMin[axis];
    const float mx = aabbMax[axis];

    if (std::abs(dir) < 1.0e-6f) {
      if (origin < mn || origin > mx) {
        return false;
      }
      continue;
    }

    float t1 = (mn - origin) / dir;
    float t2 = (mx - origin) / dir;
    if (t1 > t2) {
      std::swap(t1, t2);
    }

    tMin = std::max(tMin, t1);
    tMax = std::min(tMax, t2);
    if (tMax < tMin) {
      return false;
    }
  }

  outT = tMin;
  return true;
}

class DemoApp final : public engine::IApp {
public:
  void onInit(engine::Engine& engine) override {
    m_debugDraw.init();

    if (!m_physics.init()) {
      throw std::runtime_error("Physics init failed");
    }

    engine::Mesh cube = engine::Mesh::makeCube();
    m_cubeMeshIndex = engine.meshes().add(std::move(cube));

    setupWorld(engine);

    m_camera.position = {0.0f, 2.2f, 8.0f};
    m_camera.yawDegrees = -90.0f;
    m_camera.pitchDegrees = -8.0f;
    m_camera.nearPlane = 0.05f;
    m_camera.farPlane = 250.0f;

    int w = 0;
    int h = 0;
    engine.window().drawableSize(w, h);
    m_camera.aspect = (h > 0) ? (static_cast<float>(w) / static_cast<float>(h)) : (16.0f / 9.0f);

    m_light.direction = glm::normalize(glm::vec3(-0.7f, -1.0f, -0.45f));
    m_light.color = glm::vec3(1.0f, 0.98f, 0.93f);

#if ENGINE_ENABLE_IMGUI
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForOpenGL(engine.window().sdlWindow(), engine.window().glContext());
    ImGui_ImplOpenGL3_Init("#version 410 core");
    m_imguiEnabled = true;
#endif
  }

  void onShutdown(engine::Engine& /*engine*/) override {
    m_physics.shutdown();

#if ENGINE_ENABLE_IMGUI
    if (m_imguiEnabled) {
      ImGui_ImplOpenGL3_Shutdown();
      ImGui_ImplSDL2_Shutdown();
      ImGui::DestroyContext();
    }
#endif
  }

  void onUpdate(engine::Engine& engine, float /*dtSeconds*/) override {
    int w = 0;
    int h = 0;
    engine.window().drawableSize(w, h);
    m_camera.aspect = (h > 0) ? (static_cast<float>(w) / static_cast<float>(h)) : m_camera.aspect;

    const float mouseSensitivity = 0.08f;
    m_camera.yawDegrees += engine.input().mouseDeltaX() * mouseSensitivity;
    m_camera.pitchDegrees -= engine.input().mouseDeltaY() * mouseSensitivity;
    m_camera.pitchDegrees = std::clamp(m_camera.pitchDegrees, -89.0f, 89.0f);

    m_playerController.updateCameraFromPlayer(m_physics, m_camera);

    const Ray centerRay{m_camera.position, m_camera.forward()};
    m_selectedEntity = pickEntity(centerRay);

    updateHighlightColors();

    // Simple center-screen crosshair in world space (camera-facing tiny plus).
    const glm::vec3 crosshairCenter = m_camera.position + centerRay.direction * 0.9f;
    const glm::vec3 crosshairRight = m_camera.right() * 0.010f;
    const glm::vec3 crosshairUp = m_camera.up() * 0.010f;
    m_debugDraw.line(crosshairCenter - crosshairRight, crosshairCenter + crosshairRight, glm::vec3(1.0f));
    m_debugDraw.line(crosshairCenter - crosshairUp, crosshairCenter + crosshairUp, glm::vec3(1.0f));

    const glm::vec3 rayEnd = centerRay.origin + centerRay.direction * 40.0f;
    m_debugDraw.line(centerRay.origin, rayEnd, glm::vec3(1.0f, 0.95f, 0.2f));

    const bool leftMouse = engine.input().mouseButtonDown(SDL_BUTTON_LEFT);
    if (leftMouse && !m_prevLeftMouse) {
      const engine::RaycastHit hit = m_physics.raycast(
          centerRay.origin,
          centerRay.direction,
          150.0f,
          engine::toLayerMask(engine::PhysicsLayer::WorldStatic) |
              engine::toLayerMask(engine::PhysicsLayer::WorldDynamic));

      if (hit.hit) {
        const std::string msg = "Physics ray hit body=" + std::to_string(hit.bodyId.value) +
                                " entity=" + std::to_string(hit.entityId) +
                                " distance=" + std::to_string(hit.distance);
        engine::Log::info(msg);
      } else {
        engine::Log::info("Physics ray missed");
      }
    }
    m_prevLeftMouse = leftMouse;
  }

  void onFixedUpdate(engine::Engine& engine, float fixedDtSeconds) override {
    m_playerController.fixedUpdate(m_physics, engine.input(), m_camera, fixedDtSeconds);
    m_physics.step(fixedDtSeconds);
    m_physics.syncTransformsToScene(m_scene);
  }

  void onRender(engine::Engine& engine) override {
    engine.renderer().render(m_scene, m_camera, m_light, engine.meshes());
    m_debugDraw.render(m_camera);

#if ENGINE_ENABLE_IMGUI
    if (m_imguiEnabled) {
      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplSDL2_NewFrame();
      ImGui::NewFrame();

      ImGui::SetNextWindowBgAlpha(0.35f);
      if (ImGui::Begin("World Demo", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("FPS: %.1f", engine.smoothedFps());
        ImGui::Text("Cam: %.2f %.2f %.2f", m_camera.position.x, m_camera.position.y, m_camera.position.z);
        ImGui::Text("Selected Entity: %u", m_selectedEntity.value_or(0));
        ImGui::TextUnformatted("Mouse: look, WASD: move, Shift: sprint");
        ImGui::TextUnformatted("Ctrl/C: crouch, Space: jump");
        ImGui::TextUnformatted("LMB: physics raycast log");
      }
      ImGui::End();

      ImGui::Render();
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
#endif
  }

private:
  void setupWorld(engine::Engine& /*engine*/) {
    const engine::Entity ground = m_scene.createEntity();
    auto& groundTr = m_scene.addTransform(ground);
    groundTr.position = {0.0f, -0.05f, 0.0f};
    groundTr.scale = {80.0f, 0.1f, 80.0f};

    auto& groundMr = m_scene.addMeshRenderer(ground);
    groundMr.meshIndex = m_cubeMeshIndex;
    groundMr.albedo = {0.24f, 0.34f, 0.24f};
    groundMr.enableGroundGrid = true;

    m_baseAlbedo[ground] = groundMr.albedo;

    engine::PhysicsBodyID groundBody =
        m_physics.createStaticBox(groundTr,
                                  glm::vec3(40.0f, 0.05f, 40.0f),
                                  engine::PhysicsLayer::WorldStatic);
    m_physics.bindEntityToBody(ground, groundBody);

    m_cubeEntity = m_scene.createEntity();
    auto& cubeTr = m_scene.addTransform(m_cubeEntity);
    cubeTr.position = {0.0f, 0.5f, 0.0f};
    cubeTr.scale = {1.0f, 1.0f, 1.0f};

    auto& cubeMr = m_scene.addMeshRenderer(m_cubeEntity);
    cubeMr.meshIndex = m_cubeMeshIndex;
    cubeMr.albedo = {0.95f, 0.54f, 0.21f};
    m_baseAlbedo[m_cubeEntity] = cubeMr.albedo;

    engine::PhysicsBodyID cubeBody =
        m_physics.createStaticBox(cubeTr,
                                  glm::vec3(0.5f, 0.5f, 0.5f),
                                  engine::PhysicsLayer::WorldStatic);
    m_physics.bindEntityToBody(m_cubeEntity, cubeBody);

    m_secondEntity = m_scene.createEntity();
    auto& secondTr = m_scene.addTransform(m_secondEntity);
    secondTr.position = {2.4f, 0.75f, -2.0f};
    secondTr.scale = {1.5f, 1.5f, 1.5f};

    auto& secondMr = m_scene.addMeshRenderer(m_secondEntity);
    secondMr.meshIndex = m_cubeMeshIndex;
    secondMr.albedo = {0.22f, 0.68f, 0.92f};
    m_baseAlbedo[m_secondEntity] = secondMr.albedo;

    engine::PhysicsBodyID secondBody =
        m_physics.createStaticBox(secondTr,
                                  glm::vec3(0.75f, 0.75f, 0.75f),
                                  engine::PhysicsLayer::WorldStatic);
    m_physics.bindEntityToBody(m_secondEntity, secondBody);

    m_dynamicEntity = m_scene.createEntity();
    auto& dynTr = m_scene.addTransform(m_dynamicEntity);
    dynTr.position = {-2.0f, 4.0f, 0.0f};

    auto& dynMr = m_scene.addMeshRenderer(m_dynamicEntity);
    dynMr.meshIndex = m_cubeMeshIndex;
    dynMr.albedo = {0.92f, 0.80f, 0.30f};
    m_baseAlbedo[m_dynamicEntity] = dynMr.albedo;

    engine::PhysicsBodyID dynBody =
        m_physics.createDynamicBox(dynTr,
                                   glm::vec3(0.5f, 0.5f, 0.5f),
                                   5.0f,
                                   engine::PhysicsLayer::WorldDynamic);
    m_physics.bindEntityToBody(m_dynamicEntity, dynBody);

    const glm::vec3 playerSpawn(0.0f, 1.7f, 6.5f);
    if (!m_playerController.init(m_physics, playerSpawn)) {
      throw std::runtime_error("Player controller init failed");
    }
  }

  std::optional<engine::Entity> pickEntity(const Ray& ray) const {
    float bestT = std::numeric_limits<float>::max();
    std::optional<engine::Entity> best{};

    for (engine::Entity e : m_scene.meshRendererEntities()) {
      const engine::Transform* tr = m_scene.tryGetTransform(e);
      const engine::MeshRenderer* mr = m_scene.tryGetMeshRenderer(e);
      if (tr == nullptr || mr == nullptr) {
        continue;
      }

      if (mr->enableGroundGrid) {
        continue;
      }

      glm::vec3 aabbMin;
      glm::vec3 aabbMax;
      computeWorldAabb(*tr, *mr, aabbMin, aabbMax);

      float t = 0.0f;
      if (rayIntersectsAabb(ray, aabbMin, aabbMax, t) && t < bestT) {
        bestT = t;
        best = e;
      }
    }

    return best;
  }

  void updateHighlightColors() {
    for (engine::Entity e : m_scene.meshRendererEntities()) {
      engine::MeshRenderer* mr = m_scene.tryGetMeshRenderer(e);
      if (mr == nullptr) {
        continue;
      }

      const auto baseIt = m_baseAlbedo.find(e);
      if (baseIt == m_baseAlbedo.end()) {
        continue;
      }

      if (m_selectedEntity.has_value() && m_selectedEntity.value() == e) {
        mr->albedo = glm::min(baseIt->second * 1.55f, glm::vec3(1.0f));
      } else {
        mr->albedo = baseIt->second;
      }
    }
  }

  engine::Scene m_scene;
  std::uint32_t m_cubeMeshIndex = 0;

  engine::Entity m_cubeEntity = 0;
  engine::Entity m_secondEntity = 0;
  engine::Entity m_dynamicEntity = 0;

  std::unordered_map<engine::Entity, glm::vec3> m_baseAlbedo;
  std::optional<engine::Entity> m_selectedEntity;

  engine::Camera m_camera;
  engine::DirectionalLight m_light{};
  engine::DebugDraw m_debugDraw;

  engine::PhysicsSystem m_physics;
  engine::PlayerController m_playerController;

  bool m_prevLeftMouse = false;

#if ENGINE_ENABLE_IMGUI
  bool m_imguiEnabled = false;
#endif
};

} // namespace

int main() {
  engine::EngineConfig cfg;
  cfg.title = "World Demo - SDL2 + OpenGL 4.1 + Jolt";
  cfg.captureMouse = true;
  cfg.vsync = true;

  engine::Engine engine(cfg);
  DemoApp app;
  engine.run(app);
  return 0;
}
