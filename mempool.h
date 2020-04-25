// See end of file for license

// A memory pool for allocating fixed sized elements
// Allocated elements can be freed separately or the whole pool can be freed at once
// The pool dynamically allocates blocks of a specific size that can hold n of specified elements
// When the pool is created, no block is allocated until alloc is called

// CONFIGURATION
// MEMPOOL_MALLOC to define your own allocator used to allocate the pool
// MEMPOOL_REALLOC to define your own allocator used to allocate the pool blocks
// MEMPOOL_FREE to define your own dealloctor when freeing the pool
// Note: custom allocators should have similar behavious as malloc, realloc, and free
// MEMPOOL_MESSAGE (default fputs(s, stderr) to define your own error message callback

#ifndef MEMPOOL_H
#define MEMPOOL_H

typedef struct mempool_t Mempool;
typedef struct mempool_t mempool_t;

// Creates a memory pool that can allocate block_size amounts of memory
// element_size describes how large every allocation will be in bytes
// element_count describes how many elements each block can store before the pool allocates a new one
// -> A higher value may uses more memory that is unused if allocations aren't filled but results in less fragmented memory and fewer allocations
mempool_t* mempool_create(uint32_t element_size, uint32_t element_count);

// Returnes an element_size chunk of memory from the pool
// Either tries to fill a freed spot, take at the end of a block, or malloc a new block
void* mempool_alloc(mempool_t* pool);

void mempool_free(mempool_t* pool, void* element);

// Returns how many element are currently allocated
// Pool can safely be destroyed if returned value is 0
uint32_t mempool_get_count(mempool_t* pool);

// Destroys and frees everything in the pool
void mempool_destroy(mempool_t* pool);

#ifdef MEMPOOL_IMPLEMENTATION
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#ifndef MEMPOOL_MALLOC
#define MEMPOOL_MALLOC(s) malloc(s)
#endif
#ifndef MEMPOOL_REALLOC
#define MEMPOOL_REALLOC(p, s) realloc(p, s)
#endif
#ifndef MEMPOOL_FREE
#define MEMPOOL_FREE(p) free(p)
#endif
#ifndef MEMPOOL_MESSAGE
#define MEMPOOL_MESSAGE(s) fputs(s, stderr)
#endif

struct mempool_free
{
	// The adress for the freed element is the address of this struct
	struct mempool_free* next;
};

struct mempool_t
{
	// The size of each individual elemen
	uint32_t element_size;
	// The size of each block in bytes (element_size * element_count)
	uint32_t block_size;
	uint32_t block_count;
	uint32_t alloc_count;
	// How much of the last pool that is used, e.g an index to the available bytes in the last block
	// This excludes freed blocks
	// In terms of bytes
	uint32_t block_end;
	// A list of fixed size buffers
	uint8_t** blocks;
	// A pointer to all free elements
	struct mempool_free* free_elements;
};

mempool_t* mempool_create(uint32_t element_size, uint32_t element_count)
{
	mempool_t* pool = MEMPOOL_MALLOC(sizeof(mempool_t));
	// Clamp the size so that a free element descriptor can fit
	pool->element_size = element_size < sizeof(struct mempool_free) ? sizeof(struct mempool_free) : element_size;
	pool->block_size = pool->element_size * element_count;
	pool->block_count = 0;
	pool->blocks = NULL;
	pool->free_elements = NULL;
	pool->alloc_count = 0;
	return pool;
}

// Returnes an element_size chunk of memory from the pool
// Either tries to fill a freed spot, take at the end of a block, or malloc a new block
void* mempool_alloc(mempool_t* pool)
{
	// First check for freed blocks
	if (pool->free_elements)
	{
		void* p = pool->free_elements;
		pool->free_elements = pool->free_elements->next;
		++pool->alloc_count;
		return p;
	}

	// Last pool is full, malloc a new one
	// No pools have yet been allocated
	if (pool->block_end == pool->block_size || pool->block_count == 0)
	{
		// Allocate space for the list that holds the blocks
		void* tmp = MEMPOOL_REALLOC(pool->blocks, ++pool->block_count * sizeof(*pool->blocks));
		// Allocation error
		if (tmp == NULL)
		{
			free(pool->blocks);
			pool->blocks = NULL;
			MEMPOOL_MESSAGE("Failed to allocate memory for block list");
			return NULL;
		}
		pool->blocks = tmp;
		// Allocate the block
		pool->blocks[pool->block_count - 1] = malloc(pool->block_size);
		// Start at the beginning of new pool
		pool->block_end = 0;
		if (pool->blocks[pool->block_count - 1] == NULL)
		{
			MEMPOOL_MESSAGE("Failed to allocate memory for new pool block");
			return NULL;
		}
	}

	// Get the pointer to the beginning of the free space in the last block
	void* p = pool->blocks[pool->block_count - 1] + pool->block_end;

	// Update end 'pointers'
	pool->block_end += pool->element_size;
	++pool->alloc_count;

	return p;
}

void mempool_free(mempool_t* pool, void* element)
{
	--pool->alloc_count;
	// Make the free struct fill the freed element
	struct mempool_free* freed = element;
	freed->next = pool->free_elements;
	pool->free_elements = freed;
}

uint32_t mempool_get_count(mempool_t* pool)
{
	return pool->alloc_count;
}

// Destroys and frees everything in the pool
void mempool_destroy(mempool_t* pool)
{
	for (uint32_t i = 0; i < pool->block_count; i++)
	{
		MEMPOOL_FREE(pool->blocks[i]);
	}
	MEMPOOL_FREE(pool->blocks);
	MEMPOOL_FREE(pool);
}

#endif
#endif

// ========LICENSE========
// MIT License
//
// Copyright (c) 2020 Tim Roberts
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.