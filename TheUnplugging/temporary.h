#pragma once

#include <cstdlib>
#include <memory_resource>

struct TemporaryAllocator {
	size_t occupied;
	size_t max_size;

	char* memory;

	TemporaryAllocator(const TemporaryAllocator&) = delete;

	TemporaryAllocator(size_t);
	~TemporaryAllocator();

	void* alloc(size_t);
	void clear();
};

extern TemporaryAllocator temporary_allocator;

#define TEMPORARY_ALLOC(name, ...) new (temporary_allocator.alloc(sizeof(name))) name(__VA_ARGS__)
#define TEMPORARY_ARRAY(name, num) new (temporary_allocator.alloc(sizeof(name) * num)) name[num]

template<class T>
struct STDTemporaryAllocator : public std::allocator<T> {
	T* allocate(std::size_t size) {
		return temporary_allocator.alloc(size);
	}

	void deallocate(T* ptr, std::size_t) {};
};