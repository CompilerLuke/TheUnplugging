#include "temporary.h"

#ifndef TEMPORARY_ALLOCATOR_SIZE
#define TEMPORARY_ALLOCATOR_SIZE 10000
#endif

TemporaryAllocator temporary_allocator(TEMPORARY_ALLOCATOR_SIZE);

void TemporaryAllocator::clear() {
	this->occupied = 0;
}

TemporaryAllocator::TemporaryAllocator(size_t max_size) {
	this->occupied = 0;
	this->max_size = max_size;
	this->memory = new char[max_size];
}

TemporaryAllocator::~TemporaryAllocator() {
	delete[] this->memory;
}



void* TemporaryAllocator::alloc(size_t size) {
	auto occupied = this->occupied;
	this->occupied += size;
	if (this->occupied > this->max_size) {
		throw "Temporary allocator out of memory";
	}
	return this->memory + occupied;
}

