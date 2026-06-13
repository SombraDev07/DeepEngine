#pragma once
#include <Platform.h>
#include <chrono>
#include <string>
#include <vector>
#include <mutex>

struct ProfileEntry
{
	const char* name;
	double timeMs;
	int callCount;
};

class ScopedTimer
{
public:
	ScopedTimer(const char* name) : m_name(name), m_start(std::chrono::high_resolution_clock::now()) {}
	~ScopedTimer()
	{
		auto end = std::chrono::high_resolution_clock::now();
		double ms = std::chrono::duration<double, std::milli>(end - m_start).count();
		AddEntry(m_name, ms);
	}

	static const std::vector<ProfileEntry>& GetEntries() { return s_entries; }
	static void Clear() { s_entries.clear(); }

private:
	static void AddEntry(const char* name, double ms);
	static std::vector<ProfileEntry> s_entries;
	static std::mutex s_mutex;
	const char* m_name;
	std::chrono::high_resolution_clock::time_point m_start;
};
