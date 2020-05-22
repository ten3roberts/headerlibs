// See end of file for license

// A memory pool for allocating fixed sized elements
// Allocated elements can be freed separately or the whole pool can be freed at once
// The pool dynamically allocates blocks of a specific size that can hold n of specified elements
// When the pool is created, no block is allocated until alloc is called

// Creating a pool
// mempool_t mypool = MEMPOOL_INIT(sizeof(MyType), 128);
// MyType* p = mempool_alloc(&mypool);
// mempool_free(&mypool, p);

// When the last element allocated from a pool is freed, the internal blocks are also freed
// This means that there is no need to pool cleanup at program termination to not leak memory

// CONFIGURATION
// MEMPOOL_MALLOC to define your own allocator used to allocate the pool
// MEMPOOL_REALLOC to define your own allocator used to allocate the pool blocks
// MEMPOOL_FREE to define your own dealloctor when freeing the pool
// Note: custom allocators should have similar behavious as malloc, realloc, and free
// MEMPOOL_MESSAGE (default fputs(s, stderr) to define your own error message callback
// MEMPOOL_MAGPIE to allow magpie to track where the mempool_alloc originated from

#ifndef MEMPOOL_H
#define MEMPOOL_H

struct mempool_free
{
	// The adress for the freed element is the address of this struct
	struct mempool_free* next;
};

typedef struct mempool_t
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
} mempool_t, Mempool;

// Statically initializes a memory pool
// element_size describes how large every allocation will be in bytes
// element_count describes how many elements each block can store before the pool allocates a new one
// -> A higher value may uses more memory that is unused if allocations aren't filled but results in less fragmented memory and fewer allocations
#define MEMPOOL_INIT(elem_size, elem_count)                                                                \
	(mempool_t)                                                                                            \
	{                                                                                                      \
		.element_size = elem_size < sizeof(struct mempool_free) ? sizeof(struct mempool_free) : elem_size, \
		.block_size = elem_size * elem_count                                                               \
	}

// Returnes an element_size chunk of memory from the pool
// Either tries to fill a freed spot, take at the end of a block, or malloc a new block
#define mempool_alloc(pool) mempool_alloc_internal(pool, __FILE__, __LINE__)
void* mempool_alloc_internal(mempool_t* pool, const char* file, uint32_t line);

// Frees a chunk from the pool
// Note: element must be previosly allocated from the pool
// If the allocated count reaches zero the internal blocks are freed
void mempool_free(mempool_t* pool, void* element);

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

// Returnes an element_size chunk of memory from the pool
// Either tries to fill a freed spot, take at the end of a block, or malloc a new block
void* mempool_alloc_internal(mempool_t* pool, const char* file, uint32_t line)
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
		// Make magpie detect the called of this function
#ifdef MEMPOOL_MAGPIE
		mp_bind_internal(pool->blocks[pool->block_count - 1], file, line);
#endif
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
	// Chain any existing freed blocks
	freed->next = pool->free_elements;
	pool->free_elements = freed;

	// Free everything if pool has no allocations left
	if (pool->alloc_count == 0)
	{
		for (uint32_t i = 0; i < pool->block_count; i++)
		{
			MEMPOOL_FREE(pool->blocks[i]);
		}
		MEMPOOL_FREE(pool->blocks);
		pool->blocks = NULL;
		pool->block_count = 0;
		pool->block_end = 0;
		pool->free_elements = NULL;
	}
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