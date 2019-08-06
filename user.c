// typedef struct us {
// 	char *name;
// 	object *obj;
// 	struct us *next;
// }user;


//NOPE! Fare progettazione prima!

#include "user.h"
#include "object.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int addUs(user **list, char *nameUs)
{
	if (nameUs == NULL || searchUs(list, nameUs) != NULL) {
		return -1;
	}

	user *new = (user *)malloc(sizeof(user));
	new->obj = NULL;
	new->next = NULL;
	char *string = new->name;
	string = (char *)malloc(strlen(nameUs) * sizeof(char));
	string = strcpy(string, nameUs);

	if (*list != NULL) {
		new->next = *list;
	}
	*list = new;

	return 0;
}

user searchUs(user *list, char *nameUs)
{
	if(list == NULL || nameUs == NULL) {
		return NULL;
	}

	user *curr = list;

	while(curr != NULL) {
		if (!strcmp(nameUs, curr->name)) {
			return curr;
		}
		curr = curr->next;
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

	while(curr->next != NULL) {
		if(!strcmp(nameUs, curr->name)) {
			if(prev == NULL) {
				*lista = curr->next;
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

void printUsers(user **list)
{
	if(*list == NULL) {
		return;
	}

	user *curr = *list;

	while(curr != NULL) {
		printf("%s\n", curr->name);
		curr = curr->next;
	}
}

void freeNode(user *u) {
	free(u->name);
	if(u->obj != NULL) {
		freeObjects(u->obj);
	}
	free(u);
}

void cleanUsers(user **list)
{
	if(*list == NULL) {
		return;
	}

	while(*list != NULL) {
		user *curr = *list;
		*list = *list->next;
		freeNode(curr);
	}
}
