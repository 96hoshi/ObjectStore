#include "object.h"
#include <stdio.h>
#include <string.h>

object *object_create(char *name, size_t len)
{
	if (name == NULL) return NULL;

	size_t name_len = strlen(name) + 1;
	object *o = (object *)calloc(1, sizeof(object));
	o->name = (char *)calloc(name_len, sizeof(char));
	o->name = strcpy(o->name, name);
	o->len = len;

	return o;
}

int object_compare(void *obj, void *obj_name)
{
	object *o = (object *)obj;
	char *objname = (char *)obj_name;

	if (o == NULL && objname == NULL) return 0;
	if (o == NULL) return 1;
	if (objname == NULL) return -1;

	return strcmp(o->name, objname);
}

void object_destroy(void *obj)
{
	if (obj == NULL) return;
	object *o = (object *)obj;
	if (o->name != NULL) free(o->name);
	free(o);
}

void object_print(void *obj)
{
	if (obj == NULL) return;
	object *o = (object *)obj;
	if (o->name != NULL) printf("Object name: %s\n", o->name);
}
