// typedef struct us {
// 	char *name;
// 	object *obj;
// 	struct us *next;
// }user;

#include "user.h"
#include "object.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int addUs(user **list, char *nameUs)
{
	if (nameUs == NULL || searchUs(*list, nameUs) != NULL) {
		return -1;
	}

	user *new = (user *)malloc(sizeof(user));
	new->obj = NULL;
	new->next = NULL;
	new->name = (char *)malloc((strlen(nameUs) + 1) * sizeof(char));
	new->name = strcpy(new->name, nameUs);

	if (*list != NULL) {
		new->next = *list;
	}
	*list = new;

	return 0;
}

user *searchUs(user *list, char *nameUs)
{
	if(nameUs == NULL) {
		return NULL;
	}

	while(list != NULL) {
		if (!strcmp(nameUs, list->name)) {
			return list;
		}
		list = list->next;
	}

	return NULL;
}

int deleteUs(user **list, char *nameUs)
{
	if(*list == NULL || nameUs == NULL) {
		return -1;
	}

	user *prev = NULL;
	user *curr = *list;

	while(curr != NULL) {
		if(!strcmp(nameUs, curr->name)) {
			if(prev == NULL) {
				*list = curr->next;
			} else {
				prev->next = curr->next;
			}
			freeNode(curr);

			return 0;
		} else {
			prev = curr;
			curr = curr->next;
		}
	}

	return -1;
}

void printUsers(user *list)
{
	if(list == NULL) {
		return;
	}

	while(list != NULL) {
		printf("%s\n", list->name);
		list = list->next;
	}
}

void freeNode(user *u) {
	free(u->name);
	if(u->obj != NULL) {
		//freeObjects(u->obj);
	}
	free(u);
}

void cleanUsers(user *list)
{
	if(list == NULL) {
		return;
	}

	while(list != NULL) {
		user *curr = list;
		list = list->next;
		freeNode(curr);
	}
}
