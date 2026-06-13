#include <Logger.h>
#include <JobSystem.h>
#include <Vec3.h>
#include <Allocator.h>
#include <thread>
#include <chrono>

#ifndef SKIP_ECS_TEST
void RunECSTest();
#endif

int main()
{
	LOG_INFO("=== Engine Test Suite ===");

	// Test math
	Vec3 a(1, 2, 3);
	Vec3 b(4, 5, 6);
	Vec3 c = a + b;
	LOG_INFO("Vec3 add: (%.1f, %.1f, %.1f)", c.x, c.y, c.z);
	ENGINE_ASSERT(c.x == 5 && c.y == 7 && c.z == 9, "Vec3 add failed");

	f32 dot = a.Dot(b);
	LOG_INFO("Vec3 dot: %.1f", dot);
	ENGINE_ASSERT(dot == 32.0f, "Vec3 dot failed");

	Vec3 cross = a.Cross(b);
	LOG_INFO("Vec3 cross: (%.1f, %.1f, %.1f)", cross.x, cross.y, cross.z);

	// Test allocator
	PoolAllocator pool(64, 10);
	void* ptr = pool.Alloc(32, 16);
	LOG_INFO("Pool alloc: %p", ptr);
	ENGINE_ASSERT(ptr != nullptr, "Pool alloc failed");
	pool.Free(ptr);

	// Test job system
	JobSystem::Init(2);
	LOG_INFO("Job system initialized with %u workers", JobSystem::GetWorkerCount());

	std::atomic<int> counter{0};
	for (int i = 0; i < 10; ++i)
	{
		JobSystem::Run([&counter] { counter++; });
	}
	JobSystem::WaitAll();
	LOG_INFO("Job count: %d", counter.load());
	ENGINE_ASSERT(counter == 10, "Job count mismatch");

	JobSystem::Shutdown();

#ifndef SKIP_ECS_TEST
	RunECSTest();
#endif

	LOG_INFO("=== All tests passed! ===");
	return 0;
}
