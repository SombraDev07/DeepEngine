#include <Logger.h>
#include <ECS.h>

void RunECSTest()
{
	LOG_INFO("--- ECS Test ---");

	auto player = ECS::Create();
	ECS::Add<TransformComponent>(player, Vec3(10, 0, 5));
	ECS::Add<CameraComponent>(player);
	ECS::Get<TagComponent>(player).name = "Player";

	auto& t = ECS::Get<TransformComponent>(player);
	LOG_INFO("Player pos: (%.1f, %.1f, %.1f)", t.position.x, t.position.y, t.position.z);

	auto light = ECS::Create();
	ECS::Add<LightComponent>(light, Vec3(1, 0.8f, 0.6f), 2.0f);
	ECS::Get<TagComponent>(light).name = "Sun";

	int count = 0;
	ECS::ForEach<TransformComponent>([&](Entity e, const TransformComponent& tr) { (void)e; (void)tr; count++; });
	LOG_INFO("Entities with Transform: %d", count);

	ECS::Destroy(player);
	ECS::Destroy(light);
	LOG_INFO("ECS tests passed!");
}
