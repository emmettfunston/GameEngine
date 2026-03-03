#include "engine/physics/PhysicsSystem.hpp"

#include "engine/core/Log.hpp"
#include "engine/physics/MathAdapters.hpp"

#include <Jolt/Jolt.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/RegisterTypes.h>

#include <glm/gtc/quaternion.hpp>

#include <array>
#include <cstdarg>
#include <thread>

namespace engine {
namespace {

using JoltObjectLayer = JPH::ObjectLayer;

namespace Layers {
static constexpr JoltObjectLayer WorldStatic = 0;
static constexpr JoltObjectLayer WorldDynamic = 1;
static constexpr JoltObjectLayer Player = 2;
static constexpr JoltObjectLayer Trigger = 3;
static constexpr std::uint32_t Count = 4;
} // namespace Layers

namespace BroadPhaseLayers {
static constexpr JPH::BroadPhaseLayer WorldStatic(0);
static constexpr JPH::BroadPhaseLayer WorldDynamic(1);
static constexpr JPH::BroadPhaseLayer Player(2);
static constexpr JPH::BroadPhaseLayer Trigger(3);
static constexpr std::uint32_t Count = 4;
} // namespace BroadPhaseLayers

JoltObjectLayer toJoltLayer(PhysicsLayer layer) {
  switch (layer) {
  case PhysicsLayer::WorldStatic: return Layers::WorldStatic;
  case PhysicsLayer::WorldDynamic: return Layers::WorldDynamic;
  case PhysicsLayer::Player: return Layers::Player;
  case PhysicsLayer::Trigger: return Layers::Trigger;
  }
  return Layers::WorldStatic;
}

PhysicsLayer fromJoltLayer(JoltObjectLayer layer) {
  switch (layer) {
  case Layers::WorldStatic: return PhysicsLayer::WorldStatic;
  case Layers::WorldDynamic: return PhysicsLayer::WorldDynamic;
  case Layers::Player: return PhysicsLayer::Player;
  case Layers::Trigger: return PhysicsLayer::Trigger;
  default: return PhysicsLayer::WorldStatic;
  }
}

bool layerInMask(PhysicsLayer layer, LayerMask mask) {
  return (mask & toLayerMask(layer)) != 0;
}

class LayerMaskObjectFilter final : public JPH::ObjectLayerFilter {
public:
  explicit LayerMaskObjectFilter(LayerMask mask) : m_mask(mask) {}

  bool ShouldCollide(JPH::ObjectLayer inLayer) const override {
    return layerInMask(fromJoltLayer(inLayer), m_mask);
  }

private:
  LayerMask m_mask = LayerMaskAll;
};

void traceImpl(const char* inFmt, ...) {
  char buffer[1024];
  va_list args;
  va_start(args, inFmt);
  std::vsnprintf(buffer, sizeof(buffer), inFmt, args);
  va_end(args);
  engine::Log::debug(buffer);
}

bool assertFailedImpl(const char* inExpression, const char* inMessage, const char* inFile, std::uint32_t inLine) {
  std::string msg = "Jolt assert: ";
  msg += inExpression;
  msg += " at ";
  msg += inFile;
  msg += ":";
  msg += std::to_string(inLine);
  if (inMessage != nullptr) {
    msg += " - ";
    msg += inMessage;
  }
  engine::Log::error(msg);
  return true;
}

} // namespace

class PhysicsSystem::BroadPhaseLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface {
public:
  BroadPhaseLayerInterfaceImpl() {
    m_objectToBroadPhase[Layers::WorldStatic] = BroadPhaseLayers::WorldStatic;
    m_objectToBroadPhase[Layers::WorldDynamic] = BroadPhaseLayers::WorldDynamic;
    m_objectToBroadPhase[Layers::Player] = BroadPhaseLayers::Player;
    m_objectToBroadPhase[Layers::Trigger] = BroadPhaseLayers::Trigger;
  }

  std::uint32_t GetNumBroadPhaseLayers() const override {
    return BroadPhaseLayers::Count;
  }

  JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override {
    return m_objectToBroadPhase[inLayer];
  }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
  const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override {
    switch (static_cast<std::uint32_t>(inLayer.GetValue())) {
    case 0: return "WORLD_STATIC";
    case 1: return "WORLD_DYNAMIC";
    case 2: return "PLAYER";
    case 3: return "TRIGGER";
    default: return "UNKNOWN";
    }
  }
#endif

private:
  std::array<JPH::BroadPhaseLayer, Layers::Count> m_objectToBroadPhase{};
};

class PhysicsSystem::ObjectLayerPairFilterImpl final : public JPH::ObjectLayerPairFilter {
public:
  bool ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const override {
    if (inObject1 == Layers::Trigger || inObject2 == Layers::Trigger) {
      return true;
    }

    if (inObject1 == Layers::WorldStatic) {
      return inObject2 == Layers::WorldDynamic || inObject2 == Layers::Player;
    }
    if (inObject1 == Layers::WorldDynamic) {
      return inObject2 != Layers::Trigger;
    }
    if (inObject1 == Layers::Player) {
      return inObject2 == Layers::WorldStatic || inObject2 == Layers::WorldDynamic;
    }

    return false;
  }
};

class PhysicsSystem::ObjectVsBroadPhaseLayerFilterImpl final : public JPH::ObjectVsBroadPhaseLayerFilter {
public:
  bool ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const override {
    const auto broad = static_cast<std::uint32_t>(inLayer2.GetValue());

    if (inLayer1 == Layers::WorldStatic) {
      return broad == 1 || broad == 2;
    }
    if (inLayer1 == Layers::WorldDynamic) {
      return broad == 0 || broad == 1 || broad == 2;
    }
    if (inLayer1 == Layers::Player) {
      return broad == 0 || broad == 1;
    }
    if (inLayer1 == Layers::Trigger) {
      return true;
    }
    return false;
  }
};

PhysicsSystem::PhysicsSystem() = default;

PhysicsSystem::~PhysicsSystem() {
  shutdown();
}

bool PhysicsSystem::init() {
  if (m_started) {
    return true;
  }

  JPH::RegisterDefaultAllocator();
  JPH::Trace = traceImpl;
#if defined(JPH_ENABLE_ASSERTS)
  JPH::AssertFailed = assertFailedImpl;
#endif

  JPH::Factory::sInstance = new JPH::Factory();
  JPH::RegisterTypes();

  m_broadPhaseLayerInterface = std::make_unique<BroadPhaseLayerInterfaceImpl>();
  m_objectLayerPairFilter = std::make_unique<ObjectLayerPairFilterImpl>();
  m_objectVsBroadPhaseLayerFilter = std::make_unique<ObjectVsBroadPhaseLayerFilterImpl>();

  m_tempAllocator = std::make_unique<JPH::TempAllocatorImpl>(10 * 1024 * 1024);

  constexpr std::uint32_t cMaxJobs = 1024;
  constexpr std::uint32_t cMaxBarriers = 1024;
  const auto hw = std::max(1u, std::thread::hardware_concurrency());
  const int workerThreads = static_cast<int>(hw > 1 ? hw - 1 : 1);
  m_jobSystem = std::make_unique<JPH::JobSystemThreadPool>(cMaxJobs, cMaxBarriers, workerThreads);

  m_system = std::make_unique<JPH::PhysicsSystem>();

  constexpr std::uint32_t cMaxBodies = 4096;
  constexpr std::uint32_t cNumBodyMutexes = 0;
  constexpr std::uint32_t cMaxBodyPairs = 4096;
  constexpr std::uint32_t cMaxContactConstraints = 4096;

  m_system->Init(cMaxBodies,
                 cNumBodyMutexes,
                 cMaxBodyPairs,
                 cMaxContactConstraints,
                 *m_broadPhaseLayerInterface,
                 *m_objectVsBroadPhaseLayerFilter,
                 *m_objectLayerPairFilter);

  m_system->SetGravity(JPH::Vec3(0.0f, -9.81f, 0.0f));

  m_started = true;
  Log::info("PhysicsSystem initialized (Jolt)");
  return true;
}

void PhysicsSystem::shutdown() {
  if (!m_started) {
    return;
  }

  m_bodyToEntity.clear();
  m_entityToBody.clear();

  m_system.reset();
  m_jobSystem.reset();
  m_tempAllocator.reset();

  m_objectVsBroadPhaseLayerFilter.reset();
  m_objectLayerPairFilter.reset();
  m_broadPhaseLayerInterface.reset();

  JPH::UnregisterTypes();
  delete JPH::Factory::sInstance;
  JPH::Factory::sInstance = nullptr;

  m_started = false;
  Log::info("PhysicsSystem shutdown");
}

void PhysicsSystem::step(float fixedDt) {
  if (!isInitialized()) {
    return;
  }
  constexpr int collisionSteps = 1;
  m_system->Update(fixedDt, collisionSteps, m_tempAllocator.get(), m_jobSystem.get());
}

PhysicsBodyID PhysicsSystem::createStaticBox(const Transform& transform,
                                             const glm::vec3& halfExtents,
                                             PhysicsLayer layer) {
  return createBoxInternal(transform, halfExtents, 0.0f, false, layer);
}

PhysicsBodyID PhysicsSystem::createDynamicBox(const Transform& transform,
                                              const glm::vec3& halfExtents,
                                              float mass,
                                              PhysicsLayer layer) {
  return createBoxInternal(transform, halfExtents, mass, true, layer);
}

PhysicsBodyID PhysicsSystem::createPlayerCapsule(const glm::vec3& position, float radius, float halfHeight) {
  if (!isInitialized()) {
    return {};
  }

  JPH::CapsuleShapeSettings capsuleSettings(halfHeight, radius);
  auto shapeResult = capsuleSettings.Create();
  if (shapeResult.HasError()) {
    Log::error("Failed to create player capsule shape");
    return {};
  }

  JPH::RefConst<JPH::Shape> shape = shapeResult.Get();
  JPH::BodyCreationSettings settings(shape,
                                     math_adapters::toJoltR(position),
                                     JPH::Quat::sIdentity(),
                                     JPH::EMotionType::Dynamic,
                                     toJoltLayer(PhysicsLayer::Player));

  settings.mFriction = 0.0f;
  settings.mRestitution = 0.0f;
  settings.mLinearDamping = 0.05f;
  settings.mAngularDamping = 0.95f;
  settings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;
  settings.mMassPropertiesOverride.mMass = 85.0f;

  JPH::BodyInterface& bi = m_system->GetBodyInterface();
  JPH::Body* body = bi.CreateBody(settings);
  if (body == nullptr) {
    Log::error("Failed to create player capsule body");
    return {};
  }

  const JPH::BodyID bodyId = body->GetID();
  bi.AddBody(bodyId, JPH::EActivation::Activate);
  bi.SetMotionQuality(bodyId, JPH::EMotionQuality::LinearCast);

  const PhysicsBodyID publicId = toPublicBodyId(bodyId);
  Log::info("Created player capsule body");
  return publicId;
}

RaycastHit PhysicsSystem::raycast(const glm::vec3& origin,
                                  const glm::vec3& dir,
                                  float maxDist,
                                  LayerMask mask) const {
  RaycastHit out{};
  if (!isInitialized()) {
    return out;
  }

  const glm::vec3 rayDir = math_adapters::normalizedOrZero(dir);
  if (glm::dot(rayDir, rayDir) <= 1.0e-8f) {
    return out;
  }

  const glm::vec3 end = origin + rayDir * maxDist;
  const JPH::RRayCast ray(math_adapters::toJoltR(origin), math_adapters::toJolt(end - origin));
  const LayerMaskObjectFilter objectFilter(mask);

  JPH::RayCastResult result;
  if (!m_system->GetNarrowPhaseQuery().CastRay(ray, result, {}, objectFilter, {})) {
    return out;
  }

  const PhysicsBodyID bodyId = toPublicBodyId(result.mBodyID);

  out.hit = true;
  out.bodyId = bodyId;
  out.distance = maxDist * result.mFraction;
  out.point = origin + rayDir * out.distance;

  auto bodyIt = m_bodyToEntity.find(bodyId.value);
  if (bodyIt != m_bodyToEntity.end()) {
    out.entityId = bodyIt->second;
  }

  return out;
}

void PhysicsSystem::bindEntityToBody(Entity entity, PhysicsBodyID bodyId) {
  if (!bodyId.isValid()) {
    return;
  }
  m_entityToBody[entity] = bodyId;
  m_bodyToEntity[bodyId.value] = entity;
}

void PhysicsSystem::syncTransformsToScene(Scene& scene) {
  if (!isInitialized()) {
    return;
  }

  const JPH::BodyInterface& bi = m_system->GetBodyInterface();

  for (const auto& [entity, bodyId] : m_entityToBody) {
    Transform* tr = scene.tryGetTransform(entity);
    if (tr == nullptr || !bodyId.isValid()) {
      continue;
    }

    const JPH::BodyID joltBody = toJoltBodyId(bodyId);
    if (!bi.IsAdded(joltBody)) {
      continue;
    }

    const JPH::RVec3 pos = bi.GetCenterOfMassPosition(joltBody);
    tr->position = math_adapters::toGlm(pos);

    const JPH::Quat q = bi.GetRotation(joltBody);
    const glm::quat gq(q.GetW(), q.GetX(), q.GetY(), q.GetZ());
    tr->rotationEulerDegrees = glm::degrees(glm::eulerAngles(gq));
  }
}

void PhysicsSystem::syncSceneToPhysics(const Scene& scene) {
  if (!isInitialized()) {
    return;
  }

  JPH::BodyInterface& bi = m_system->GetBodyInterface();

  for (const auto& [entity, bodyId] : m_entityToBody) {
    const Transform* tr = scene.tryGetTransform(entity);
    if (tr == nullptr || !bodyId.isValid()) {
      continue;
    }

    const JPH::BodyID joltBody = toJoltBodyId(bodyId);
    if (!bi.IsAdded(joltBody)) {
      continue;
    }

    const PhysicsLayer layer = bodyLayer(bodyId);
    if (layer != PhysicsLayer::WorldDynamic) {
      continue;
    }

    bi.SetPosition(joltBody, math_adapters::toJoltR(tr->position), JPH::EActivation::Activate);
  }
}

glm::vec3 PhysicsSystem::bodyPosition(PhysicsBodyID bodyId) const {
  if (!isInitialized() || !bodyId.isValid()) {
    return glm::vec3(0.0f);
  }

  const JPH::BodyInterface& bi = m_system->GetBodyInterface();
  const JPH::BodyID joltBody = toJoltBodyId(bodyId);
  if (!bi.IsAdded(joltBody)) {
    return glm::vec3(0.0f);
  }
  return math_adapters::toGlm(bi.GetCenterOfMassPosition(joltBody));
}

glm::vec3 PhysicsSystem::bodyLinearVelocity(PhysicsBodyID bodyId) const {
  if (!isInitialized() || !bodyId.isValid()) {
    return glm::vec3(0.0f);
  }

  const JPH::BodyInterface& bi = m_system->GetBodyInterface();
  const JPH::BodyID joltBody = toJoltBodyId(bodyId);
  if (!bi.IsAdded(joltBody)) {
    return glm::vec3(0.0f);
  }
  return math_adapters::toGlm(bi.GetLinearVelocity(joltBody));
}

void PhysicsSystem::setBodyLinearVelocity(PhysicsBodyID bodyId, const glm::vec3& linearVelocity) {
  if (!isInitialized() || !bodyId.isValid()) {
    return;
  }

  JPH::BodyInterface& bi = m_system->GetBodyInterface();
  const JPH::BodyID joltBody = toJoltBodyId(bodyId);
  if (!bi.IsAdded(joltBody)) {
    return;
  }

  bi.SetLinearVelocity(joltBody, math_adapters::toJolt(linearVelocity));
}

void PhysicsSystem::setBodyRotationIdentity(PhysicsBodyID bodyId) {
  if (!isInitialized() || !bodyId.isValid()) {
    return;
  }

  JPH::BodyInterface& bi = m_system->GetBodyInterface();
  const JPH::BodyID joltBody = toJoltBodyId(bodyId);
  if (!bi.IsAdded(joltBody)) {
    return;
  }

  bi.SetAngularVelocity(joltBody, JPH::Vec3::sZero());
  bi.SetRotation(joltBody, JPH::Quat::sIdentity(), JPH::EActivation::DontActivate);
}

PhysicsBodyID PhysicsSystem::createBoxInternal(const Transform& transform,
                                               const glm::vec3& halfExtents,
                                               float mass,
                                               bool dynamic,
                                               PhysicsLayer layer) {
  if (!isInitialized()) {
    return {};
  }

  JPH::BoxShapeSettings shapeSettings(math_adapters::toJolt(halfExtents));
  auto shapeResult = shapeSettings.Create();
  if (shapeResult.HasError()) {
    Log::error("Failed to create Jolt box shape");
    return {};
  }

  JPH::RefConst<JPH::Shape> shape = shapeResult.Get();

  const glm::vec3 rotRad = glm::radians(transform.rotationEulerDegrees);
  const glm::quat q = glm::quat(rotRad);
  const JPH::Quat jq(q.x, q.y, q.z, q.w);

  JPH::BodyCreationSettings settings(shape,
                                     math_adapters::toJoltR(transform.position),
                                     jq,
                                     dynamic ? JPH::EMotionType::Dynamic : JPH::EMotionType::Static,
                                     toJoltLayer(layer));

  settings.mFriction = 0.8f;
  settings.mRestitution = 0.0f;

  if (dynamic) {
    // Provide a target mass and let Jolt compute physically consistent inertia.
    // If mass is invalid, fall back to automatic mass/inertia from shape density.
    if (mass > 0.0f) {
      settings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;
      settings.mMassPropertiesOverride.mMass = mass;
    } else {
      settings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateMassAndInertia;
    }
  }

  JPH::BodyInterface& bi = m_system->GetBodyInterface();
  JPH::Body* body = bi.CreateBody(settings);
  if (body == nullptr) {
    Log::error("Failed to create Jolt body");
    return {};
  }

  const JPH::BodyID bodyId = body->GetID();
  bi.AddBody(bodyId, dynamic ? JPH::EActivation::Activate : JPH::EActivation::DontActivate);

  const PhysicsBodyID publicId = toPublicBodyId(bodyId);
  Log::info(dynamic ? "Created dynamic box body" : "Created static box body");
  return publicId;
}

bool PhysicsSystem::isInitialized() const {
  return m_started && m_system != nullptr;
}

PhysicsLayer PhysicsSystem::bodyLayer(PhysicsBodyID bodyId) const {
  if (!isInitialized() || !bodyId.isValid()) {
    return PhysicsLayer::WorldStatic;
  }

  const JPH::BodyInterface& bi = m_system->GetBodyInterface();
  const JPH::BodyID joltBody = toJoltBodyId(bodyId);
  if (!bi.IsAdded(joltBody)) {
    return PhysicsLayer::WorldStatic;
  }

  const JoltObjectLayer layer = bi.GetObjectLayer(joltBody);
  return fromJoltLayer(layer);
}

std::uint32_t PhysicsSystem::toBodyKey(const JPH::BodyID& id) {
  return id.GetIndexAndSequenceNumber();
}

PhysicsBodyID PhysicsSystem::toPublicBodyId(const JPH::BodyID& id) {
  return PhysicsBodyID{toBodyKey(id)};
}

JPH::BodyID PhysicsSystem::toJoltBodyId(PhysicsBodyID bodyId) const {
  return JPH::BodyID(bodyId.value);
}

} // namespace engine
