#include "user.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


user *user_create(char *name)
{
	if (name == NULL) return NULL;

	size_t name_len = strlen(name) + 1;
	user *u = (user *)calloc(1, sizeof(user));
	u->name = (char *)calloc(name_len, sizeof(char));
	u->name = strncpy(u->name, name, name_len);
	u->objects = list_create(object_compare, object_compare_name, object_destroy, object_print);

	return u;
}

object *user_search_object(user *u, void *name)
{
	if (name == NULL) return NULL;

	return (object *)list_search_field_unsafe(u->objects, name);
}

list_result user_insert_object(user *u, char *name, size_t len)
{
	if (u == NULL) return list_null;

	object *obj = object_create(name, len);
	return list_insert_unsafe(u->objects, obj);
}

list_result user_delete_object(user *u, object *obj)
{
	if (u == NULL) return list_null;

	return list_delete_info_unsafe(u->objects, obj);
}

int user_compare(void *usr1, void *usr2)
{
	user *u1 = (user *)usr1;
	user *u2 = (user *)usr2;

	if (u1 == NULL && u2 == NULL) return 0;
	if (u1 == NULL) return 1;
	if (u2 == NULL) return -1;

	return strcmp(u1->name, u2->name);
}

int user_compare_name(void *usr, void *usr_name)
{
	user *u = (user *)usr;
	char *username = (char *)usr_name;

	if (u == NULL && username == NULL) return 0;
	if (u == NULL) return 1;
	if (username == NULL) return -1;

	return strcmp(u->name, username);
}

void user_destroy(void *usr)
{
	if (usr == NULL) return;
	user *u = (user *)usr;
	if (u->name != NULL) free(u->name);
	if (u->objects != NULL) list_destroy(u->objects);
	free(u);
}

void user_print(void *usr)
{
	if (usr == NULL) return;
	user *u = (user *)usr;

	if (u->name != NULL) printf("User name: %s\n", u->name);
	if (u->objects != NULL) list_print(u->objects);
}
