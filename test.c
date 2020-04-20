#define MP_IMPLEMENTATION
#define MP_CHECK_FULL
#include "magpie.h"
#define HASHTABLE_IMPLEMENTATION
#include "hashtable.h"
#include <stdio.h>

struct Person
{
	char name[256];
	int age;
};

int main()
{
	struct Person people[] = {{.name = "Adam", .age = 17},
							  {.name = "Bert", .age = 14},
							  {.name = "Cecilia", .age = 16},
							  {.name = "David", .age = 18}};

	Hashtable* table = hashtable_create_string();
	for (int i = 0; i < sizeof people / sizeof *people; i++)
	{
		hashtable_insert(table, people[i].name, &people[i]);
	}

	hashtable_print(table, stdout);

	struct Person* p = hashtable_find(table, "Adam");
	if (p)
		printf("%s is %d years old\n", p->name, p->age);
	else
		printf("Could not locate person\n");
	hashtable_destroy(table);
	mp_terminate();
}