#include "Logger.h"
#include <cstdio>
#include <cstdarg>

namespace Logger
{
	static const char* s_levelNames[] = {"DEBUG", "INFO", "WARN", "ERROR"};

	void Log(Level level, const char* fmt, ...)
	{
		char buffer[4096];
		va_list args;
		va_start(args, fmt);
		vsnprintf(buffer, sizeof(buffer), fmt, args);
		va_end(args);

		printf("[%s] %s\n", s_levelNames[(int)level], buffer);
	}
}
