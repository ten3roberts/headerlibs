#ifndef HASHTABLE_H
#define HASHTABLE_H
#include <stdint.h>

// Implement a simple dynamic hashtable in C
// To build the library do
// #define HASHTABLE_IMPLEMENTATION in ONE C file to create the function implementations before including the header
// To configure the library, add the define under <CONFIGURATION> above the header include in the C file declaring the implementation
// hashtable can be safely included several times, but only one C file can define HASHTABLE_IMPLEMENTATION

// CONFIGURATION
// HASHTABLE_SIZE_TOLERANCE (70) sets the tolerance in percent that will cause the table to resize
// -> If the table count is HASHTABLE_SIZE_TOLERANCE of the total size, it will resize, likewise down
// -> If set to 0, the hashtable will not resize
// -> Value is clamped to >= 50 if not 0
// HASHTABLE_DEFAULT_SIZE (default 16) decides the default size of the hashtable
// -> Table will resize up and down in powers of two automatically
// -> Note, must be a power of 2

// Basic usage
// hashtable_t storing arbitrary type with string keys
// hashtable_t* table = hashtable_create_string();

// Adding data to the hashtable
// hashtable_insert(table, "Testkey", &mystruct)
// If an entry of that name already exists, it is replaced and returned

// The table only stores a pointer to the key and data stored
// This means that when an entry is removed, it's value and key is not freed
// This means that the data and key can be stack allocated if kept in scope
// Keys can also be literals like "ABC"

// If you want to dynamically allocate a string as key, it is recommended to store it withing the struct
// hashtable_insert(table, mystruct.name, &mystruct)
// This also makes insertion of structs easy if they store a name or similar identifier withing them

// Custom key types
// To create a hashtable with a custom type of key you need to create a hashfunction and a comparefunction
// The hashfunction takes in a void* of the key and returns a uint32_t as a hash, the more spread the hashfunction is, the better
// The compare function takes a void* of the key and returns 0 if they match

// To then create the hashtable with your custom functions, use hashtable_create(hashfunc, compfunc)
// Usage is exactly like a hashtable storing strings or any other type

// See end of file for license

typedef struct hashtable_t Hashtable;
typedef struct hashtable_t hashtable_t;

// Creates a hashtable with the specified functionality
// hashfunc should be the function that generates a hash from the key
// compfunc compares two keys and returns 0 on match
// Note, the key pointer should be valid as long as it is in the map
hashtable_t* hashtable_create(uint32_t (*hashfunc)(void*), int32_t (*compfunc)(void*, void*));

// Creates a hashtable with the string hash function
// Shorthand for hashtable_create(hashtable_hashfunc_string, hashtable_comp_string);
hashtable_t* hashtable_create_string();

// Inserts an item associated with a key into the hashtable
// If key already exists, it is returned and replaced
void* hashtable_insert(hashtable_t* hashtable, void* key, void* data);

// Finds and returns an item from the hashtable
// Returns NULL if failure
void* hashtable_find(hashtable_t* hashtable, void* key);

// Removes and returns an item from a hashtable
void* hashtable_remove(hashtable_t* hashtable, void* key);

// Removes and returns the first element in the hashtable
// Returns NUL when table is empty
// Can be used to clear free the stored data before hashtable_destroy
void* hashtable_pop(hashtable_t* hashtable);

// Returns how many items are in the hashtable
uint32_t hashtable_get_count(hashtable_t* hashtable);

// Destroys and frees a hashtable
// Does not free the stored data
void hashtable_destroy(hashtable_t* hashtable);

// Prints the hash table to a file descriptor, use for debug purposes
void hashtable_print(hashtable_t* hashtable, FILE* fp);

uint32_t hashtable_hashfunc_string(void* pkey);
int32_t hashtable_comp_string(void* pkey1, void* pkey2);

#ifdef HASHTABLE_IMPLEMENTATION

#ifndef HASHTABLE_DEFAULT_SIZE
#define HASHTABLE_DEFAULT_SIZE 16
#endif
#if HASHTABLE != 0 && HASHTABLE_SIZE_TOLERANCE < 50
#define HASHTABLE_SIZE_TOLERANCE 50
#endif

#ifndef HASHTABLE_SIZE_TOLERANCE
#define HASHTABLE_SIZE_TOLERANCE 70
#endif

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

struct hashtable_item
{
	void* key;
	void* data;
	// For collision chaining
	struct hashtable_item* next;
};

struct hashtable_t
{
	uint32_t (*hashfunc)(void*);
	int32_t (*compfunc)(void*, void*);

	// The amount of buckets in the list
	uint32_t size;

	// How many items are in the table, including
	uint32_t count;
	struct hashtable_item** items;
};

hashtable_t* hashtable_create(uint32_t (*hashfunc)(void*), int32_t (*compfunc)(void*, void*))
{
	hashtable_t* hashtable = malloc(sizeof(hashtable_t));
	hashtable->hashfunc = hashfunc;
	hashtable->compfunc = compfunc;
	hashtable->count = 0;
	hashtable->size = HASHTABLE_DEFAULT_SIZE;
	hashtable->items = calloc(HASHTABLE_DEFAULT_SIZE, sizeof(struct hashtable_item*));
	return hashtable;
}

hashtable_t* hashtable_create_string()
{
	return hashtable_create(hashtable_hashfunc_string, hashtable_comp_string);
}

// Inserts an already allocated item struct into the hash table
// Does not resize the hash table
// Does no increase count
static void* hashtable_insert_internal(hashtable_t* hashtable, struct hashtable_item* item)
{
	// Discard the next since collision chain will be reevaluated
	item->next = NULL;
	// Calculate the hash with the provided hash function to determine index
	uint32_t index = hashtable->hashfunc(item->key);
	// Make sure hash fits inside table
	index = index & (hashtable->size - 1);
	// Slot is empty, no collision
	struct hashtable_item* cur = hashtable->items[index];
	if (cur == NULL)
	{
		hashtable->items[index] = item;
	}
	// Insert at tail if collision
	// Check for duplicate
	else
	{
		struct hashtable_item* prev = NULL;
		while (cur)
		{
			// Duplicate
			if (hashtable->compfunc(cur->key, item->key) == 0)
			{
				// Handle beginning
				if (prev == NULL)
					hashtable->items[index] = item;
				else
					prev->next = item;

				item->next = cur->next;

				void* retdata = cur->data;
				free(cur);
				return retdata;
			}
			prev = cur;
			cur = cur->next;
		}
		// Insert at tail
		prev->next = item;
	}
	return NULL;
}

// Resizes the list either up (1) or down (-1)
// Internal function
static void hashtable_resize(hashtable_t* hashtable, int32_t direction)
{
	// Save the old values
	uint32_t old_size = hashtable->size;
	struct hashtable_item** old_items = hashtable->items;

	if (direction == 1)
		hashtable->size = hashtable->size << 1;
	else if (direction == -1)
		hashtable->size = hashtable->size >> 1;
	else
		return;

	// Allocate the larger list
	hashtable->items = calloc(hashtable->size, sizeof(struct hashtable_item*));

	for (uint32_t i = 0; i < old_size; i++)
	{
		struct hashtable_item* cur = old_items[i];
		struct hashtable_item* next = NULL;
		while (cur)
		{
			// Save the next since it will be changed with insert
			next = cur->next;

			// Reinsert into the new list
			// This won't resize the table again
			hashtable_insert_internal(hashtable, cur);

			cur = next;
		}
	}
	free(old_items);
}

void* hashtable_insert(hashtable_t* hashtable, void* key, void* data)
{
	// Check if table needs to be resized before calculating the hash as it will change
	if (++hashtable->count * 100 >= hashtable->size * HASHTABLE_SIZE_TOLERANCE)
		hashtable_resize(hashtable, 1);

	struct hashtable_item* item = malloc(sizeof(struct hashtable_item));
	item->key = key;
	item->data = data;
	item->next = NULL;
	return hashtable_insert_internal(hashtable, item);
}

void* hashtable_find(hashtable_t* hashtable, void* key)
{
	uint32_t index = hashtable->hashfunc(key);
	// Make sure hash fits inside table
	index = index & (hashtable->size - 1);
	struct hashtable_item* cur = hashtable->items[index];

	while (cur)
	{
		// Match
		if (hashtable->compfunc(cur->key, key) == 0)
		{
			return cur->data;
		}
		cur = cur->next;
	}
	return NULL;
}

// Removes and returns an item from a hashtable
void* hashtable_remove(hashtable_t* hashtable, void* key)
{
	uint32_t index = hashtable->hashfunc(key);
	// Make sure hash fits inside table
	index = index & (hashtable->size - 1);

	struct hashtable_item* cur = hashtable->items[index];
	struct hashtable_item* prev = NULL;

	while (cur)
	{
		// Match
		if (hashtable->compfunc(cur->key, key) == 0)
		{
			// Handle beginning
			if (prev == NULL)
				hashtable->items[index] = cur->next;
			else
				prev->next = cur->next;

			void* cur_data = cur->data;
			free(cur);

			// Check if table needs to be resized down after removing item
			if (--hashtable->count * 100 < hashtable->size * (100 - HASHTABLE_SIZE_TOLERANCE))
				hashtable_resize(hashtable, -1);

			return cur_data;
		}
		prev = cur;
		cur = cur->next;
	}
	return NULL;
}

void* hashtable_pop(hashtable_t* hashtable)
{
	for (uint32_t i = 0; i < hashtable->size; i++)
	{
		struct hashtable_item* cur = hashtable->items[i];
		if (cur != NULL)
		{
			hashtable->items[i] = cur->next;
			void* cur_data = cur->data;
			free(cur);

			// Check if table needs to be resized down after removing item
			if (--hashtable->count * 100 <= hashtable->size * (100 - HASHTABLE_SIZE_TOLERANCE))
				hashtable_resize(hashtable, -1);

			return cur_data;
		}
	}

	return NULL;
}

uint32_t hashtable_get_count(hashtable_t* hashtable)
{
	return hashtable->count;
}

void hashtable_destroy(hashtable_t* hashtable)
{
	for (uint32_t i = 0; i < hashtable->size; i++)
	{
		struct hashtable_item* cur = hashtable->items[i];
		struct hashtable_item* next = NULL;
		while (cur)
		{
			next = cur->next;
			free(cur);
			cur = next;
		}
	}
	free(hashtable->items);
	free(hashtable);
}

// Debug function
void hashtable_print(hashtable_t* hashtable, FILE* fp)
{
	for (uint32_t i = 0; i < hashtable->size; i++)
	{
		struct hashtable_item* cur = hashtable->items[i];
		if (cur == NULL)
			fprintf(fp, "[%.4u]: ---------", i);
		else
			fprintf(fp, "[%.4u]: ", i);

		while (cur)
		{
			fprintf(fp, "%p: \"%.10s\"; ", cur->key, (char*)cur->key);
			cur = cur->next;
		}
		fprintf(fp, "\n");
	}
}

// Common Hash functions
uint32_t hashtable_hashfunc_string(void* pkey)
{
	char* key = (char*)pkey;
	uint64_t value = 0;
	uint32_t i = 0;
	uint32_t lkey = strlen(key);

	for (; i < lkey; i++)
	{
		value = value * 37 + key[i];
	}
	return value;
}

int32_t hashtable_comp_string(void* pkey1, void* pkey2)
{
	return strcmp(pkey1, pkey2);
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