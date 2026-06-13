#include "Profiler.h"

std::vector<ProfileEntry> ScopedTimer::s_entries;
std::mutex ScopedTimer::s_mutex;

void ScopedTimer::AddEntry(const char* name, double ms)
{
	std::lock_guard lock(s_mutex);
	for (auto& e : s_entries)
	{
		if (strcmp(e.name, name) == 0)
		{
			e.timeMs += ms;
			e.callCount++;
			return;
		}
	}
	s_entries.push_back({name, ms, 1});
}
