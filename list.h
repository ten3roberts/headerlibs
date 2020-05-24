/*#ifndef LIST_H
#define LIST_H
#include <stdint.h>

// List is a dynamic array
// Creating a list
// list_t mylist = list_create(sizeof (myobject));

// To build the library do
// #define LIST_IMPLEMENTATION in ONE C file to create the function implementations before including the header
// defined LIST_IMPLEMENTATION

// Creating and using a list
// list_t mylist = list_create(sizeof (myobject));
// list_push(&mylist, object);
// list_destroy(&mylist);

typedef struct list_t list_t;
typedef struct list_t List;

struct list_t
{
	void* data;
	uint32_t count;
	uint32_t size;
	uint32_t element_size;
};

#ifndef list_create
#define list_create(s) list_create_internal(s);
#endif


#ifdef LIST_IMPLEMENTATION



list_t* list_create()
{

}

void list_push(list_t* list, void* element)
{

}
#endif
#endif*/