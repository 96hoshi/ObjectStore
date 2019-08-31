#ifndef OBJECT_H
#define OBJECT_H

#include <stdio.h>
#include <stdlib.h>


typedef struct {
	char *name;
	size_t len;
} object;


object *object_create(char *name, size_t len);

int object_compare(void *obj, void *obj_name);

void object_dump(void *obj, FILE *f);

void object_destroy(void *obj);

#endif
