#pragma once
#include <Platform.h>
#include <Vec3.h>
#include <ECS.h>
#include <memory>
#include <vector>
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyID.h>

class PhysicsSystem
{
public:
	PhysicsSystem();
	~PhysicsSystem();
	bool Init();
	void Shutdown();
	void Update(float dt);

	void AddGroundPlane();
	JPH::BodyID AddBox(const Vec3& pos, const Vec3& halfExtent, bool isDynamic = true);
	void RemoveBody(JPH::BodyID id);
	Vec3 GetPosition(JPH::BodyID id) const;

	void RegisterEntity(Entity e, JPH::BodyID id);
	void UnregisterEntity(Entity e);
	void SyncTransforms();

	static PhysicsSystem& Get() { return *s_instance; }

private:
	static PhysicsSystem* s_instance;

	struct Impl;
	std::unique_ptr<Impl> m_impl;
	std::vector<std::pair<Entity, JPH::BodyID>> m_entityBodies;
};
