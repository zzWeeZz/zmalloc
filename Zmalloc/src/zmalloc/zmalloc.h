#pragma once

// Allocates a block of memory of the given size.
// Returns a pointer to the allocated memory, or nullptr if the allocation fails.
void* zmalloc(size_t size);
// Frees a previously allocated block of memory.
// The pointer must have been returned by a previous call to zmalloc, zcalloc, or zrealloc.
void zfree(void* ptr);
// Reallocates a block of memory to a new size.
// If ptr is nullptr, this function behaves like zmalloc.
// If the new size is smaller than the current size, the block is truncated.
// If the new size is larger, a new block is allocated, the contents are copied, and the old block is freed.
void* zrealloc(void* ptr, size_t size);
// Allocates an array of num elements, each of size bytes, and initializes all bytes to zero.
// Returns a pointer to the allocated memory, or nullptr if the allocation fails or if num or size is zero.
void* zcalloc(size_t num, size_t size);
