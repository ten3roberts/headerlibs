#define MP_IMPLEMENTATION
#define MP_CHECK_FULL
#include "magpie.h"
#define HASHTABLE_IMPLEMENTATION
#include "hashtable.h"

#define MEMPOOL_IMPLEMENTATION
#include "mempool.h"

#define LIBJSON_IMPLEMENTATION
#include "libjson.h"

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
	struct Person* aletha = hashtable_find(table, "Aletha");
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
	Mempool* pool = mempool_create(sizeof(struct Person), pool_size);
	struct Person* people[lenof(names)];
	for (uint32_t i = 0; i < lenof(names); i++)
	{
		people[i] = mempool_alloc(pool);
		assert(people[i] != NULL);
		people[i]->age = i;
		snprintf(people[i]->name, sizeof(people[i]->name), "%s", names[i]);
	}

	// Print the people
	for (uint32_t i = 0; i < lenof(names); i++)
	{
		printf("[%4u]: name: %s, age: %d\n", i, people[i]->name, people[i]->age);
	}
	mempool_destroy(pool);
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
}

int main()
{
	if (test_hashtable())
	{
		printf("Hash table test failed\n");
		return -1;
	}
	//test_mempool(2);
	//test_mempool(8);
	//test_mempool(32);

	test_json();
	mp_print_locations();
	if (mp_get_count() > 0)
	{
		printf("Memory leaked!\n");
		return -10;
	}
	mp_terminate();
}