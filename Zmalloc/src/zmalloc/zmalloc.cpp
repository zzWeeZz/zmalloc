#include "zmalloc.h"
#include <iostream>
#include <zmalloc/OsUtils.h>

enum class HeapGroup
{
	Tiny,
	Small,
	Large
};

struct HeapMetadata
{
	HeapMetadata* previous;
	HeapMetadata* next;
	size_t totalSize;
	size_t freeSize;
	size_t block_count;
};

struct BlockMetadata
{
	HeapMetadata* parentHeap;
	BlockMetadata* previous;
	BlockMetadata* next;
	size_t size;
	bool isUsed;
};

static HeapMetadata* s_tinyHeap = nullptr;
static HeapMetadata* s_smallHeap = nullptr;
static HeapMetadata* s_largeHeap = nullptr;

template<typename T>
static inline void* Shift(void* start)
{
	return static_cast<void*>(static_cast<char*>(start) + sizeof(T));
}
template<typename T>
static inline void* ShiftBack(void* start)
{
	return static_cast<void*>(static_cast<char*>(start) - sizeof(T));
}


constexpr size_t AmountPerPage = 128;

static inline size_t TinyHeapAllocationSize()
{
	std::expected<size_t, OSUtils::ErrorCode> result = OSUtils::GetPageSize();
	_ASSERT(result && "Error failed to fetch page size");

	return 4 * result.value();
}

static inline size_t TinyBlockSize()
{
	return TinyHeapAllocationSize() / AmountPerPage;
}

static inline size_t SmallHeapAllocationSize()
{
	std::expected<size_t, OSUtils::ErrorCode> result = OSUtils::GetPageSize();
	_ASSERT(result && "Error failed to fetch page size");

	return 16 * result.value();
}

static inline size_t SmallBlockSize()
{
	return SmallHeapAllocationSize() / AmountPerPage;
}

static HeapMetadata* AllocateNewPageAndInitalizeMetadata(size_t size, HeapMetadata* previous_meta_data)
{
	std::expected<void*, OSUtils::ErrorCode> allocResult = OSUtils::AllocateMemory(size);
	_ASSERT(allocResult && "Error failed to allocate memory for tiny heap");

	// first time allocation
	HeapMetadata* meta_data = (HeapMetadata*)allocResult.value();
	memset(meta_data, 0, size);

	meta_data->previous = previous_meta_data;
	meta_data->next = nullptr;
	meta_data->totalSize = size - sizeof(HeapMetadata);
	meta_data->freeSize = meta_data->totalSize;
	meta_data->block_count = 0;

	return meta_data;
}

static void* zmalloc_internal(size_t size, HeapMetadata*& heap_to_alloc_to, size_t page_allocation_size, size_t block_size)
{
	HeapMetadata* currentHeap = heap_to_alloc_to;
	HeapMetadata* lastHeap = nullptr;
	while (currentHeap)
	{
		if (currentHeap->freeSize >= size + sizeof(BlockMetadata))
		{
			break;
		}
		lastHeap = currentHeap;
		currentHeap = currentHeap->next;
	}
	if (currentHeap == nullptr)
	{
		currentHeap = AllocateNewPageAndInitalizeMetadata(page_allocation_size, lastHeap);
	}

	if(!heap_to_alloc_to)
	{
		heap_to_alloc_to = currentHeap;
	}

	BlockMetadata* block = (BlockMetadata*)Shift<HeapMetadata>(currentHeap);
	BlockMetadata* lastBlock = nullptr;
	while (block != nullptr)
	{
		if (!block->isUsed)
		{
			// found a free block
			block->isUsed = true;
			block->size = size;
			currentHeap->freeSize -= block->size;
			currentHeap->block_count += 1;
			block->parentHeap = currentHeap;
			return Shift<BlockMetadata>(block);
		}
		lastBlock = block;
		block->next = reinterpret_cast<BlockMetadata*>(reinterpret_cast<char*>(block) + sizeof(BlockMetadata) + block_size);
		block = block->next;
		block->previous = lastBlock;
	}

	_ASSERT(false && "Error no free blocks found PANIC");
	return nullptr;
}


void* zmalloc(size_t size) 
{
	if (size < TinyBlockSize() - sizeof(BlockMetadata))
	{
		return zmalloc_internal(size, s_tinyHeap, TinyHeapAllocationSize(), TinyBlockSize());
	}
	else if (size < SmallBlockSize())
	{
		return zmalloc_internal(size, s_smallHeap, SmallHeapAllocationSize(), SmallBlockSize());
	}

	_ASSERT(false && "Error large allocations not supported yet");
	return nullptr;
}

void zfree(void* ptr)
{
	_ASSERT(ptr != nullptr && "zfree: Error invalid pointer");

	BlockMetadata* block = (BlockMetadata*)ShiftBack<BlockMetadata>(ptr);
	HeapMetadata* parentHeap = block->parentHeap;
	_ASSERT(block->isUsed && "zfree: Error double free detected");

	block->isUsed = false;
	parentHeap->freeSize += block->size;
	parentHeap->block_count -= 1;

	if (parentHeap->block_count == 0)
	{
		// free the entire heap
		if (parentHeap->previous)
		{
			parentHeap->previous->next = parentHeap->next;
		}
		if (parentHeap->next)
		{
			parentHeap->next->previous = parentHeap->previous;
		}
		if (s_tinyHeap == parentHeap)
		{
			s_tinyHeap = parentHeap->next;
		}
		else if (s_smallHeap == parentHeap)
		{
			s_smallHeap = parentHeap->next;
		}

		const OSUtils::ErrorCode result = OSUtils::FreeMemory(parentHeap);
		_ASSERT(result == OSUtils::ErrorCode::Success && "zfree: Error failed to free memory");
	}
}

void* zrealloc(void* ptr, size_t size)
{
	if (ptr == nullptr)
	{
		return zmalloc(size);
	}

	BlockMetadata* block = (BlockMetadata*)ShiftBack<BlockMetadata>(ptr);

	if (block->size >= size)
	{
		// current block is big enough
		return ptr;
	}
	
	void* new_block = zmalloc(size);
	memcpy(new_block, ptr, block->size);
	zfree(ptr);
	return new_block;
}

void* zcalloc(size_t num, size_t size)
{
	if (num == 0 || size == 0)
	{
		return nullptr;
	}
	const size_t total_size = num * size;
	void* ptr = zmalloc(total_size);
	memset(ptr, 0, total_size);
	return ptr;
}

void PrintHeapState()
{
	std::cout << "Tiny Heaps:\n";
	HeapMetadata* currentHeap = s_tinyHeap;
	while (currentHeap)
	{
		std::cout << "  Heap at " << currentHeap << ": totalSize=" << currentHeap->totalSize
			<< ", freeSize=" << currentHeap->freeSize << ", block_count=" << currentHeap->block_count << "\n";
		BlockMetadata* block = (BlockMetadata*)Shift<HeapMetadata>(currentHeap);
		while (block)
		{
			std::cout << "    Block at " << block << ": size=" << block->size
				<< ", isUsed=" << (block->isUsed ? "true" : "false") << "\n";
			block = block->next;
		}
		currentHeap = currentHeap->next;
	}
	std::cout << "Small Heaps:\n";
	currentHeap = s_smallHeap;
	while (currentHeap)
	{
		std::cout << "  Heap at " << currentHeap << ": totalSize=" << currentHeap->totalSize
			<< ", freeSize=" << currentHeap->freeSize << ", block_count=" << currentHeap->block_count << "\n";
		BlockMetadata* block = (BlockMetadata*)Shift<HeapMetadata>(currentHeap);
		while (block)
		{
			std::cout << "    Block at " << block << ": size=" << block->size
				<< ", isUsed=" << (block->isUsed ? "true" : "false") << "\n";
			block = block->next;
		}
		currentHeap = currentHeap->next;
	}
}

bool ValidateHeapIntegrity()
{
	// For simplicity, this function just checks that all heaps and blocks are consistent.
	HeapMetadata* currentHeap = s_tinyHeap;
	while (currentHeap)
	{
		size_t calculated_free_size = currentHeap->totalSize;
		size_t block_count = 0;
		BlockMetadata* block = (BlockMetadata*)Shift<HeapMetadata>(currentHeap);
		while (block)
		{
			if (block->isUsed)
			{
				calculated_free_size -= block->size;
				block_count += 1;
			}
			block = block->next;
		}
		if (calculated_free_size != currentHeap->freeSize || block_count != currentHeap->block_count)
		{
			return false;
		}
		currentHeap = currentHeap->next;
	}
	return true;
}

void CheckForMemoryLeaks()
{
	if (s_tinyHeap != nullptr || s_smallHeap != nullptr)
	{
		std::cerr << "Memory leak detected!\n";
		PrintHeapState();
		_ASSERT(false && "Memory leak detected");
	}
}
