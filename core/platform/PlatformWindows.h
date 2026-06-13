#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <intrin.h>

inline long AtomicInc(volatile long* p) { return _InterlockedIncrement(p); }
inline long AtomicDec(volatile long* p) { return _InterlockedDecrement(p); }
inline long AtomicAdd(volatile long* p, long v) { return _InterlockedExchangeAdd(p, v); }
inline long long AtomicAdd64(volatile long long* p, long long v) { return _InterlockedExchangeAdd64(p, v); }
inline long long AtomicCmpXchg64(volatile long long* p, long long xchg, long long cmp) { return _InterlockedCompareExchange64(p, xchg, cmp); }

#define ENGINE_THREAD_LOCAL __declspec(thread)
#define ENGINE_ALIGN(x) __declspec(align(x))
#define ENGINE_FORCE_INLINE __forceinline
#define ENGINE_NO_INLINE __declspec(noinline)
#define ENGINE_BREAK_IF_DEBUGGER() __debugbreak()
