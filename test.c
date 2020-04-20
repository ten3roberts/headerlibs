#define MP_IMPLEMENTATION
#define MP_CHECK_FULL
#include "magpie.h"
#define HASHTABLE_IMPLEMENTATION
#include "hashtable.h"
#include <stdio.h>
#include <stdlib.h>

char* names[] = {"Aletha", "Karma",	  "Cliff", "Adelle",  "Andra",	 "Kenny", "Masako", "Beatrice", "Irina",  "Tressa",
				 "Cuc",	   "Carleen", "Tamie", "Stanley", "Ladonna", "Suzan", "Lauryn", "Rogelio",	"Elaine", "Shayna"};

struct Person
{
	char name[256];
	int age;
};

int main()
{
	Hashtable* table = hashtable_create_string();

	// Generate people
	for (int i = 0; i < 20; i++)
	{
		struct Person* p = malloc(sizeof(struct Person));
		char* name = names[rand() % (sizeof names / sizeof *names)];
		memcpy(p->name, name, sizeof(p->name));
		int lname = strlen(name);
		lname = lname > sizeof p->name ? sizeof p->name : lname;
		p->name[lname] = '\0';
		free(hashtable_insert(table, p->name, p));
		
		
	}

	hashtable_print(table, stdout);
	free(hashtable_remove(table, "Aletha"));
	struct Person* p = hashtable_find(table, "Adam");
	if (p)
		printf("%s is %d years old\n", p->name, p->age);
	else
		printf("Could not locate person\n");

	struct Person* pfree = NULL;
	while ((pfree = hashtable_pop(table)))
		free(pfree);

	hashtable_print(table, stdout);
	hashtable_destroy(table);
	mp_terminate();
}