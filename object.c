#include "object.h"
#include <string.h>


object *object_create(char *name, size_t len)
{
	if (name == NULL) return NULL;

	size_t name_len = strlen(name) + 1;
	object *o = (object *)calloc(1, sizeof(object));
	if (o == NULL) {
		return NULL;
	}
	o->name = (char *)calloc(name_len, sizeof(char));
	if (o->name == NULL) {
		free(o);
		return NULL;
	}
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

void object_dump(void *obj, FILE *f)
{
	if (obj == NULL) return;
	object *o = (object *)obj;

	if (o->name == NULL) {
		fprintf(f, "0\n");
		fprintf(f, "0\n");
	} else {
		fprintf(f, "%zu\n", strlen(o->name));
		fprintf(f, "%s\n", o->name);
		fprintf(f, "%zu\n", o->len);
	}
}

void object_destroy(void *obj)
{
	if (obj == NULL) return;
	object *o = (object *)obj;
	if (o->name != NULL) free(o->name);
	free(o);
}
