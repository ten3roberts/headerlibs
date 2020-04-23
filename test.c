#define MP_IMPLEMENTATION
#define MP_CHECK_FULL
#include "magpie.h"
#define HASHTABLE_IMPLEMENTATION
#include "hashtable.h"

#define MEMPOOL_IMPLEMENTATION
#include "mempool.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

char* names[] = {"Aletha", "Bert", "Ceasar", "David", "Elize", "Felix", "George", "Heather", "Ingrid", "Josephine", "Katherine"};

struct Person
{
	char name[256];
	int age;
};

#define lenof(arr) (sizeof(arr) / sizeof(*arr))

int test_hashtable()
{
	Hashtable* table = hashtable_create_string();
	assert(table != NULL);

	// Generate random people
	for (int i = 0; i < sizeof names / sizeof *names; i++)
	{
		struct Person* p = malloc(sizeof(struct Person));
		char* name = names[i];
		memcpy(p->name, name, sizeof(p->name));
		int lname = strlen(name);
		lname = lname > sizeof p->name ? sizeof p->name : lname;
		p->name[lname] = '\0';
		p->age = rand() % 20 + 10;
		free(hashtable_insert(table, p->name, p));
	}

	hashtable_print(table, stdout);
	struct Person* p = hashtable_find(table, "Aletha");
	if (p)
	{
		printf("%s is %d years old\n", p->name, p->age);
	}
	else
	{
		printf("Could not locate person\n");
		return -1;
	}

	struct Person* pfree = NULL;
	while ((pfree = hashtable_pop(table)))
	{
		free(pfree);
	}
	printf("After freeing\n");
	hashtable_print(table, stdout);

	hashtable_destroy(table);
	return 0;
}

int test_mempool()
{
	mempool_t* pool = mempool_create(sizeof(struct Person), 16);
	struct Person* people[lenof(names)];
	for (uint32_t i = 0; i < lenof(names); i++)
	{
		people[i] = mempool_alloc(pool);
		assert(people[i] != NULL);
		people[i]->age = i;
		snprintf(people[i]->name, sizeof(people[i]->name), "%s", names[i]);
	}

	// Print the people
	for(uint32_t i = 0; i < lenof(names); i++)
	{
		printf("[%4u]: name: %s, age:%d\n", i, people[i]->name, people[i]->age);
	}
	mempool_destroy(pool);
	return 0;
}

int main()
{
	//test_hashtable();
	test_mempool();
	mp_print_locations();
	mp_terminate();
}