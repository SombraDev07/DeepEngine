#include "PhysicsSystem.h"
#include "PhysicsLayers.h"
#include <ECS.h>
#include <Logger.h>

#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#undef min
#undef max

static constexpr u32 MAX_BODIES = 1024;
static constexpr u32 MAX_MUTEXES = 0;
static constexpr u32 MAX_BODY_PAIRS = 1024;
static constexpr u32 MAX_CONTACTS = 2048;

PhysicsSystem* PhysicsSystem::s_instance = nullptr;

struct PhysicsSystem::Impl
{
	JPH::TempAllocatorImpl tempAlloc{10 * 1024 * 1024};
	JPH::JobSystemThreadPool jobSystem{1024, 128, -1};
	JPH::PhysicsSystem physicsSystem;
	JPH::BodyInterface* bodyInterface = nullptr;
};

PhysicsSystem::PhysicsSystem() {}
PhysicsSystem::~PhysicsSystem() {}

bool PhysicsSystem::Init()
{
	s_instance = this;
	JPH::RegisterDefaultAllocator();
	JPH::Factory::sInstance = new JPH::Factory();
	JPH::RegisterTypes();

	m_impl = std::make_unique<Impl>();

	static BPLayerInterfaceImpl bpLayerInterface;
	static ObjectVsBroadPhaseLayerFilterImpl objVsBpFilter;
	static ObjectLayerPairFilterImpl objPairFilter;

	m_impl->physicsSystem.Init(MAX_BODIES, MAX_MUTEXES, MAX_BODY_PAIRS, MAX_CONTACTS,
		bpLayerInterface, objVsBpFilter, objPairFilter);

	m_impl->bodyInterface = &m_impl->physicsSystem.GetBodyInterface();
	AddGroundPlane();
	LOG_INFO("PhysicsSystem initialized");
	return true;
}

void PhysicsSystem::Shutdown()
{
	m_impl.reset();
	delete JPH::Factory::sInstance;
	JPH::Factory::sInstance = nullptr;
	LOG_INFO("PhysicsSystem shut down");
}

void PhysicsSystem::Update(float dt)
{
	m_impl->physicsSystem.Update(dt, 1, &m_impl->tempAlloc, &m_impl->jobSystem);

	// Manual gravity fallback: force bodies down
	for (auto& [e, id] : m_entityBodies)
	{
		JPH::Vec3 vel = m_impl->bodyInterface->GetLinearVelocity(id);
		vel += JPH::Vec3(0, -9.81f * dt, 0);
		m_impl->bodyInterface->SetLinearVelocity(id, vel);
	}

	SyncTransforms();
}

void PhysicsSystem::AddGroundPlane()
{
	JPH::BodyCreationSettings settings(new JPH::BoxShape(JPH::Vec3(50.0f, 0.1f, 50.0f)), JPH::RVec3(0, -0.1f, 0), JPH::Quat::sIdentity(), JPH::EMotionType::Static, Layers::NON_MOVING);
	m_impl->bodyInterface->CreateAndAddBody(settings, JPH::EActivation::DontActivate);
}

JPH::BodyID PhysicsSystem::AddBox(const Vec3& pos, const Vec3& halfExtent, bool isDynamic)
{
	JPH::BodyCreationSettings settings(new JPH::BoxShape(JPH::Vec3(halfExtent.x, halfExtent.y, halfExtent.z)),
		JPH::RVec3(pos.x, pos.y, pos.z), JPH::Quat::sIdentity(),
		isDynamic ? JPH::EMotionType::Dynamic : JPH::EMotionType::Static,
		isDynamic ? Layers::MOVING : Layers::NON_MOVING);

	return m_impl->bodyInterface->CreateAndAddBody(settings,
		isDynamic ? JPH::EActivation::Activate : JPH::EActivation::DontActivate);
}

void PhysicsSystem::RemoveBody(JPH::BodyID id)
{
	m_impl->bodyInterface->RemoveBody(id);
}

Vec3 PhysicsSystem::GetPosition(JPH::BodyID id) const
{
	auto p = m_impl->bodyInterface->GetPosition(id);
	return Vec3((f32)p.GetX(), (f32)p.GetY(), (f32)p.GetZ());
}

void PhysicsSystem::RegisterEntity(Entity e, JPH::BodyID id)
{
	m_entityBodies.push_back({e, id});
}

void PhysicsSystem::UnregisterEntity(Entity e)
{
	m_entityBodies.erase(std::remove_if(m_entityBodies.begin(), m_entityBodies.end(),
		[e](auto& p) { return p.first == e; }), m_entityBodies.end());
}

void PhysicsSystem::SyncTransforms()
{
	for (auto& [e, id] : m_entityBodies)
	{
		if (!ECS::Has<TransformComponent>(e)) continue;
		auto& t = ECS::Get<TransformComponent>(e);

		JPH::BodyLockRead lock(m_impl->physicsSystem.GetBodyLockInterface(), id);
		if (lock.Succeeded())
		{
			const JPH::Body& body = lock.GetBody();
			JPH::RVec3 pos = body.GetPosition();
			t.position = Vec3((f32)pos.GetX(), (f32)pos.GetY(), (f32)pos.GetZ());
		}
	}
}
