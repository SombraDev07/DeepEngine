#pragma once

#if defined(_WIN32) || defined(_WIN64)
	#define PLATFORM_WINDOWS 1
	#define PLATFORM_NAME "Windows"
	#include "PlatformWindows.h"
#elif defined(__APPLE__)
	#define PLATFORM_MACOS 1
	#define PLATFORM_NAME "macOS"
#elif defined(__linux__)
	#define PLATFORM_LINUX 1
	#define PLATFORM_NAME "Linux"
#else
	#error "Unsupported platform"
#endif

#if defined(__x86_64__) || defined(_M_X64)
	#define ARCH_X64 1
#elif defined(__aarch64__) || defined(_M_ARM64)
	#define ARCH_ARM64 1
#endif

#if defined(__SSE2__) || defined(_M_X64)
	#define HAS_SSE2 1
#endif
#if defined(__SSE4_1__)
	#define HAS_SSE4 1
#endif
#if defined(__AVX2__)
	#define HAS_AVX2 1
#endif
#if defined(__ARM_NEON)
	#define HAS_NEON 1
#endif

#include <cstdint>

#ifndef ENGINE_TYPES_DEFINED
#define ENGINE_TYPES_DEFINED

using i8  = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;
using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using f32 = float;
using f64 = double;

#endif

#define BIT(x) (1u << (x))
#define ALIGN_UP(x, a) (((x) + ((a)-1)) & ~((a)-1))
#define ALIGN_DOWN(x, a) ((x) & ~((a)-1))
#define ARRAY_COUNT(x) (sizeof(x) / sizeof((x)[0]))

#define ENGINE_API
