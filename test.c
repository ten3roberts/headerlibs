#define MP_IMPLEMENTATION
#define MP_CHECK_FULL
#include "magpie.h"
#define HASHTABLE_IMPLEMENTATION
#define hashtable_create(hashfunc, compfunc) mp_bind(hashtable_create_internal(hashfunc, compfunc));
#include "hashtable.h"

#define MEMPOOL_ALLOC(pool) mp_bind(mempool_alloc_internal(pool))

#define MEMPOOL_IMPLEMENTATION
#define MEMPOOL_MAGPIE
#include "mempool.h"

#define LIBJSON_IMPLEMENTATION
#include "libjson.h"

#define LIST_IMPLEMENTATION
#include "list.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

char* names[] = {"Aletha", "Bert",	  "Ceasar", "David",	 "Elize",	 "Felix",
				 "George", "Heather", "Ingrid", "Josephine", "Katherine"};

struct Person
{
	char name[256];
	int age;
};

#define lenof(arr) (sizeof(arr) / sizeof(*arr))

int test_hashtable()
{
	Hashtable* table = hashtable_create_uint32();
	assert(table != NULL);

	// Generate random people
	for (int i = 0; i < 5; i++)
	{
		struct Person* p = malloc(sizeof(struct Person));
		char* name = names[i];
		memcpy(p->name, name, sizeof(p->name));
		size_t lname = strlen(name);
		lname = lname > sizeof p->name ? sizeof p->name : lname;
		p->name[lname] = '\0';
		p->age = i;
		free(hashtable_insert(table, &p->age, p));
	}

	hashtable_print(table, stdout);
	int age = 2;
	struct Person* aletha = hashtable_find(table, &age);
	if (aletha)
	{
		printf("%s is %d years old\n", aletha->name, aletha->age);
	}
	else
	{
		printf("Could not locate person\n");
		return -1;
	}

	// Test iterator
	struct Person* p = NULL;
	hashtable_iterator* it = hashtable_iterator_begin(table);
	uint32_t find_count = 0;
	printf("Iterating hashtable\n");
	while ((p = hashtable_iterator_next(it)))
	{
		++find_count;
		printf("%s is %d years old\n", p->name, p->age);
	}
	assert(find_count == hashtable_get_count(table));
	hashtable_iterator_end(it);

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

int test_mempool(int pool_size)
{
	mempool_t pool = MEMPOOL_INIT(sizeof(struct Person), pool_size);
	struct Person* people[lenof(names)];
	for (uint32_t i = 0; i < lenof(names); i++)
	{
		people[i] = mempool_alloc(&pool);
		assert(people[i] != NULL);
		people[i]->age = i;
		snprintf(people[i]->name, sizeof(people[i]->name), "%s", names[i]);
	}

	// Print the people
	for (uint32_t i = 0; i < lenof(names); i++)
	{
		printf("[%4u]: name: %s, age: %d\n", i, people[i]->name, people[i]->age);
	}

	return 0;
}

int test_json()
{
	JSON* root = json_loadfile("./example.json");

	json_destroy(json_pop_member(root, "age"));
	json_add_member(root, "name", json_create_string("Jacob"));
	printf("Name = %s\n", json_get_string(json_get_member(root, "name")));
	json_destroy(json_pop_member(root, "name"));

	json_writefile(root, "out.json", JSON_FORMAT);

	json_destroy(root);
	mp_terminate();
	return 0;
}

int main()
{
	if (test_hashtable())
	{
		printf("Hash table test failed\n");
		return -1;
	}
	test_mempool(2);
	test_mempool(8);
	test_mempool(32);

	//test_json();
	mp_print_locations();
	mp_terminate();
	if (mp_get_count() > 0)
	{
		printf("Memory leaked!\n");
		return -10;
	}
}