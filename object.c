#include "object.h"
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

int object_compare(void *o1, void *o2)
{
	object *obj1 = (object *)o1;
	object *obj2 = (object *)o2;

	if (obj1 == NULL && obj2 == NULL) return 0;
	if (obj1 == NULL) return 1;
	if (obj2 == NULL) return -1;

	return strcmp(obj1->name, obj2->name);
}

void object_destroy(object *o)
{
	if (o == NULL) return;
	if (o->name != NULL) free(o->name);
	free(o);
}
