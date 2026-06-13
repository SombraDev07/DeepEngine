#pragma once
#include <entt/entt.hpp>
#include <Platform.h>
#include <Vec3.h>
#include <mutex>

using Entity = entt::entity;
using Registry = entt::registry;
constexpr Entity NullEntity = entt::null;

struct TransformComponent { Vec3 position; Vec3 rotation; Vec3 scale = Vec3(1); };
struct MeshComponent     { u32 meshId = 0; };
struct CameraComponent   { f32 fov = 60.0f; f32 nearPlane = 0.1f; f32 farPlane = 1000.0f; };
struct LightComponent    { Vec3 color = Vec3(1); f32 intensity = 1.0f; };
struct TagComponent      { const char* name = "Unnamed"; };

namespace ECS
{
	inline Registry g_Registry;
	inline std::mutex g_Mutex;

	inline Entity Create()
	{
		std::lock_guard lock(g_Mutex);
		auto e = g_Registry.create();
		g_Registry.emplace<TagComponent>(e, "Entity");
		return e;
	}

	template<typename T, typename... Args>
	inline T& Add(Entity e, Args&&... args)
	{
		std::lock_guard lock(g_Mutex);
		return g_Registry.emplace<T>(e, std::forward<Args>(args)...);
	}

	template<typename T>
	inline T& Get(Entity e)
	{
		std::lock_guard lock(g_Mutex);
		return g_Registry.get<T>(e);
	}

	template<typename T>
	inline bool Has(Entity e)
	{
		std::lock_guard lock(g_Mutex);
		return g_Registry.all_of<T>(e);
	}

	template<typename T>
	inline void Remove(Entity e)
	{
		std::lock_guard lock(g_Mutex);
		g_Registry.remove<T>(e);
	}

	inline void Destroy(Entity e)
	{
		std::lock_guard lock(g_Mutex);
		g_Registry.destroy(e);
	}

	template<typename... Components, typename Func>
	inline void ForEach(Func&& func)
	{
		std::lock_guard lock(g_Mutex);
		g_Registry.view<Components...>().each(func);
	}


}
