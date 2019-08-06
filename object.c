// -objects: lista di
// 		-char* name
// 		-size_t len
// 		-objects* next

#include "object.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

int addObj(object **list, char *nameObj)
{
	if (nameObj == NULL || searchObj(*list, nameObj) != NULL) {
		return -1;
	}

	object *new = (object *)malloc(sizeof(object));
	new->name = (char *)malloc((strlen(nameObj) + 1) * sizeof(char));
	new->name = strcpy(new->name, nameObj);
	new->next = NULL;

	if (*list != NULL) {
		new->next = *list;
	}
	*list = new;

	return 0;
}

object *searchObj(object *list, char *nameObj)
{
	if(list == NULL || nameObj == NULL) {
		return NULL;
	}

	while(list != NULL) {
		if (!strcmp(nameObj, list->name)) {
			return list;
		}
		list = list->next;
	}

	return NULL;
}

int deleteObj(object **list, char *nameObj)
{
	if(*list == NULL || nameObj == NULL) {
		return -1;
	}

	object *prev = NULL;
	object *curr = *list;

	while(curr != NULL) {
		if(!strcmp(nameObj, curr->name)) {
			if(prev == NULL) {
				*list = curr->next;
			} else {
				prev->next = curr->next;
			}
			free(curr->name);
			free(curr);

			return 0;
		} else {
			prev = curr;
			curr = curr->next;
		}
	}

	return -1;
}

void printObjects(object *list)
{
	if(list == NULL) {
		return;
	}

	while(list != NULL) {
		printf("%s\n", list->name);
		list = list->next;
	}
}

void freeObjects(object *list)
{
	if(list == NULL) {
		return;
	}

	while(list != NULL) {
		object *curr = list;
		list = list->next;
		free(curr->name);
		free(curr);
	}
}
