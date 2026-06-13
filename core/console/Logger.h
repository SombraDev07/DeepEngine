#pragma once
#include <Platform.h>

namespace Logger
{
	enum class Level { Debug, Info, Warning, Error };

	void Log(Level level, const char* fmt, ...);
}

#define LOG_DEBUG(fmt, ...)  Logger::Log(Logger::Level::Debug,   "[%s:%d] " fmt, __FILE__, __LINE__, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)   Logger::Log(Logger::Level::Info,    fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)   Logger::Log(Logger::Level::Warning, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...)  Logger::Log(Logger::Level::Error,   fmt, ##__VA_ARGS__)

#define ENGINE_ASSERT(cond, msg) \
	do { if (!(cond)) { LOG_ERROR("ASSERT: %s - %s", #cond, msg); ENGINE_BREAK_IF_DEBUGGER(); } } while(0)
