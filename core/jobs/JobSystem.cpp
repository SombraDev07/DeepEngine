#include "JobSystem.h"
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <algorithm>

namespace JobSystem
{
	static std::vector<std::thread> s_workers;
	static std::mutex s_queueMutex;
	static std::condition_variable s_condition;
	static std::queue<JobFunction> s_queue;
	static std::atomic<u32> s_activeJobs{0};
	static bool s_shutdown = false;

	static void WorkerThread()
	{
		while (true)
		{
			JobFunction job;
			{
				std::unique_lock lock(s_queueMutex);
				s_condition.wait(lock, [] { return !s_queue.empty() || s_shutdown; });
				if (s_shutdown && s_queue.empty()) return;
				job = std::move(s_queue.front());
				s_queue.pop();
			}
			job();
			s_activeJobs--;
		}
	}

	void Init(u32 workerCount)
	{
		if (workerCount == 0)
			workerCount = std::max(1u, std::thread::hardware_concurrency() - 1);
		for (u32 i = 0; i < workerCount; ++i)
			s_workers.emplace_back(WorkerThread);
	}

	void Shutdown()
	{
		s_shutdown = true;
		s_condition.notify_all();
		for (auto& w : s_workers)
			if (w.joinable()) w.join();
		s_workers.clear();
	}

	void Run(JobFunction&& func, EJobPriority priority)
	{
		s_activeJobs++;
		{
			std::lock_guard lock(s_queueMutex);
			s_queue.push(std::move(func));
		}
		s_condition.notify_one();
	}

	void WaitAll()
	{
		while (s_activeJobs.load() > 0)
			std::this_thread::yield();
	}

	u32  GetWorkerCount() { return (u32)s_workers.size(); }
	bool IsWorkerThread()
	{
		auto id = std::this_thread::get_id();
		for (auto& w : s_workers)
			if (w.get_id() == id) return true;
		return false;
	}
}
