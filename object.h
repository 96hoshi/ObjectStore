#ifndef OBJECT_H
#define OBJECT_H

#include <stdlib.h>


typedef struct {
	char *name;
	size_t len;
} object;


object *object_create(char *name, size_t len);

int object_compare(void *obj, void *obj_name);

void object_destroy(void *obj);

void object_print(void *obj);

#endif
