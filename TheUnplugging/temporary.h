#pragma once

struct TemporaryAllocator {
	size_t occupied;
	size_t max_size;

	char* memory;

	TemporaryAllocator(size_t);
	~TemporaryAllocator();

	void* alloc(size_t);
	void clear();
};

extern TemporaryAllocator temporary_allocator;

template<typename T>
T* alloc_temporary() {
	return (T*)temporary_allocator.alloc(sizeof(T));
}