#ifndef HASHTABLE_H
#define HASHTABLE_H
#include <stdint.h>

// Implement a simple dynamic hashtable in C
// To build the library do
// #define HASHTABLE_IMPLEMENTATION in ONE C file to create the function implementations before including the header
// To configure the library, add the define under <CONFIGURATION> above the header include in the C file declaring the implementation
// hashtable can be safely included several times, but only one C file can define HASHTABLE_IMPLEMENTATION

typedef struct Hashtable Hashtable;

// Creates a hashtable with the specified functionality
// hashfunc should be the function that generates a hash from the key
// compfunc compares two keys and returns 0 on match
// Note, the key pointer should be valid as long as it is in the map
Hashtable* hashtable_create(unsigned int (*hashfunc)(void*), int (*compfunc)(void*, void*));

// Creates a hashtable with the string hash function
// Shorthand for hashtable_create(hashtable_hashfunc_string, hashtable_comp_string);
Hashtable* hashtable_create_string();

// Inserts an item associated with a key into the hashtable
// If key already exists, it is returned and replaced
void* hashtable_insert(Hashtable* hashtable, void* key, void* data);

// Finds and returns an item from the hashtable
// Returns NULL if failure
void* hashtable_find(Hashtable* hashtable, void* key);

// Removes and returns an item from a hashtable
void* hashtable_remove(Hashtable* hashtable, void* key);

// Prints the hash table to a file descriptor, use for debug purposes
void hashtable_print(Hashtable* hashtable, FILE* fp);

// Destroys and frees a hashtable
// Does not free the stored data
void hashtable_destroy(Hashtable* hashtable);

unsigned int hashtable_hashfunc_string(void* pkey);
int hashtable_comp_string(void* pkey1, void* pkey2);

#ifdef HASHTABLE_IMPLEMENTATION

#ifndef HASHTABLE_DEFAULT_SIZE
#define HASHTABLE_DEFAULT_SIZE 16
#endif

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

struct Hashtable_item
{
	void* key;
	void* data;
	// For collision chaining
	struct Hashtable_item* next;
};

struct Hashtable
{
	unsigned int (*hashfunc)(void*);
	int (*compfunc)(void*, void*);
	unsigned int size;
	unsigned int count;
	struct Hashtable_item** items;
};

Hashtable* hashtable_create(unsigned int (*hashfunc)(void*), int (*compfunc)(void*, void*))
{
	Hashtable* hashtable = malloc(sizeof(Hashtable));
	hashtable->hashfunc = hashfunc;
	hashtable->compfunc = compfunc;
	hashtable->count = 0;
	hashtable->size = HASHTABLE_DEFAULT_SIZE;
	hashtable->items = calloc(HASHTABLE_DEFAULT_SIZE, sizeof(struct Hashtable_item*));
	return hashtable;
}

Hashtable* hashtable_create_string()
{
	return hashtable_create(hashtable_hashfunc_string, hashtable_comp_string);
}

void* hashtable_insert(Hashtable* hashtable, void* key, void* data)
{
	unsigned int index = hashtable->hashfunc(key);
	// Make sure hash fits inside table
	index = index & (hashtable->size - 1);
	struct Hashtable_item* item = malloc(sizeof(struct Hashtable_item));
	item->key = key;
	item->data = data;
	item->next = NULL;

	// Slot is empty, no collision
	struct Hashtable_item* cur = hashtable->items[index];
	if (cur == NULL)
	{
		hashtable->items[index] = item;
	}
	// Insert at tail if collision
	// Check for duplicate
	else
	{
		struct Hashtable_item* prev = NULL;
		while (cur)
		{
			// Duplicate
			if (hashtable->compfunc(cur->key, key) == 0)
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

void* hashtable_find(Hashtable* hashtable, void* key)
{
	unsigned int index = hashtable->hashfunc(key);
	// Make sure hash fits inside table
	index = index & (hashtable->size - 1);
	struct Hashtable_item* cur = hashtable->items[index];

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
void* hashtable_remove(Hashtable* hashtable, void* key)
{
	unsigned int index = hashtable->hashfunc(key);
	// Make sure hash fits inside table
	index = index & (hashtable->size - 1);
	struct Hashtable_item* cur = hashtable->items[index];
	struct Hashtable_item* prev = NULL;

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

			free(cur);
			return cur->data;
		}
		prev = cur;
		cur = cur->next;
	}
	return NULL;
}

void hashtable_print(Hashtable* hashtable, FILE* fp)
{
	struct Hashtable_item* cur = NULL;
	for (unsigned int i = 0; i < hashtable->size; i++)
	{
		cur = hashtable->items[i];
		if (cur == NULL)
			fprintf(fp, "[%.4u]: ---------", i);
		else
			fprintf(fp, "[%.4u]: ", i);

		while (cur)
		{
			fprintf(fp, "%p: \"%.10s\"; ", cur->key, cur->key);
			cur = cur->next;
		}
		fprintf(fp, "\n");
	}
}

void hashtable_destroy(Hashtable* hashtable)
{
	struct Hashtable_item* cur = NULL;
	struct Hashtable_item* next = NULL;
	for (unsigned int i = 0; i < hashtable->size; i++)
	{
		cur = hashtable->items[i];
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

// Common Hash functions
unsigned int hashtable_hashfunc_string(void* pkey)
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

int hashtable_comp_string(void* pkey1, void* pkey2)
{
	return strcmp(pkey1, pkey2);
}

#endif
#endif