#include "user.h"
#include <stdlib.h>
#include <string.h>


user *user_create(char *name, int fd) 
{
	if (name == NULL) return NULL;

	size_t name_len = strlen(name) + 1;
	user *u = (user *)calloc(1, sizeof(user));
	u->name = (char *)calloc(name_len, sizeof(char));
	u->name = strcpy(u->name, name);
	u->objects = list_create(object_compare, object_destroy);
	u->fd = fd;

	return u;
}

object *user_search_object(user *u, char *name)
{
	if (name == NULL) return NULL;

	return (object *)list_search(u->objects, name);
}

int user_compare(void *u1, void *u2)
{
	user *user1 = (user *)u1;
	user *user2 = (user *)u2;

	if (user1 == NULL && user2 == NULL) return 0;
	if (user1 == NULL) return 1;
	if (user2 == NULL) return -1;

	return strcmp(user1->name, user2->name);
}

void user_destroy(user *u)
{
	if (u == NULL) return;
	if (u->name != NULL) free(u->name);
	list_destroy(u->objects);
	free(u);
}
