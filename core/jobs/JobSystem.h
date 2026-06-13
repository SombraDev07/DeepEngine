#pragma once
#include <Platform.h>
#include <functional>

enum class EJobPriority : u8
{
	High    = 0,
	Regular = 1,
	Low     = 2,
	Stream  = 3,
	COUNT   = 4
};

using JobFunction = std::function<void()>;

namespace JobSystem
{
	void Init(u32 workerCount = 0);
	void Shutdown();

	void Run(JobFunction&& func, EJobPriority priority = EJobPriority::Regular);
	void WaitAll();

	u32  GetWorkerCount();
	bool IsWorkerThread();
}
