#pragma once
#include <Platform.h>
#include <cstdlib>
#include <new>

struct IAllocator
{
	virtual ~IAllocator() {}
	virtual void* Alloc(size_t size, size_t align = 16) = 0;
	virtual void Free(void* ptr) = 0;
	virtual void* Realloc(void* ptr, size_t size) { return nullptr; }
};

struct DefaultAllocator : IAllocator
{
	void* Alloc(size_t size, size_t align) override
	{
		return _aligned_malloc(size > 0 ? size : 1, align);
	}
	void Free(void* ptr) override { _aligned_free(ptr); }
};

inline DefaultAllocator g_DefaultAllocator;

struct PoolAllocator : IAllocator
{
	PoolAllocator(size_t blockSize, size_t blockCount, IAllocator* parent = &g_DefaultAllocator)
		: m_blockSize(blockSize), m_blockCount(blockCount), m_parent(parent)
	{
		size_t total = blockSize * blockCount + sizeof(void*) * blockCount;
		m_memory = (u8*)parent->Alloc(total, 16);
		m_freeList = (void**)(m_memory + blockSize * blockCount);
		for (size_t i = 0; i < blockCount; ++i)
		{
			m_freeList[i] = m_memory + i * blockSize;
		}
		m_freeIndex = blockCount - 1;
	}

	~PoolAllocator() { m_parent->Free(m_memory); }

	void* Alloc(size_t size, size_t align) override
	{
		if (m_freeIndex > 0)
			return m_freeList[m_freeIndex--];
		return m_parent->Alloc(size, align);
	}

	void Free(void* ptr) override
	{
		if (ptr >= m_memory && ptr < m_memory + m_blockSize * m_blockCount)
			m_freeList[++m_freeIndex] = ptr;
		else
			m_parent->Free(ptr);
	}

private:
	size_t m_blockSize;
	size_t m_blockCount;
	IAllocator* m_parent;
	u8* m_memory;
	void** m_freeList;
	size_t m_freeIndex = 0;
};
