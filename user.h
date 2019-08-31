#ifndef USER_H
#define USER_H

#include "list.h"
#include "object.h"


typedef struct {
	char *name;
	list *objects;
} user;

user *user_create(char *name);

object *user_search_object(user *u, void *name);

list_result user_insert_object(user *u, char *name, size_t len);

object *user_delete_object(user *u, void *name);

int user_compare(void *usr, void *usr_name);

void user_destroy(void *usr);

void user_print(void *usr);

#endif
