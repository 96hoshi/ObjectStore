#ifndef LIST_H
#define LIST_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


typedef enum {
	list_success = 0,
	list_null = -10,
	list_not_found
} list_result;

typedef int (*fun_info_compare)(void *, void *);
typedef void (*fun_info_destroy)(void *);
typedef void (*fun_info_dump)(void *, FILE *);

typedef struct _node{
	void *info;
	struct _node *next;
} node;

typedef struct {
	node *head;
	size_t count;
	pthread_mutex_t mux;
	fun_info_compare info_compare;
	fun_info_destroy info_destroy;
	fun_info_dump info_dump;
} list;


//Thread Unsafe functions

list *list_create(fun_info_compare info_compare,
				  fun_info_destroy info_destroy,
				  fun_info_dump info_dump);

list_result list_insert_unsafe(list *l, void *info);

void *list_search_unsafe(list *l, void *info);

void* list_delete_unsafe(list *l, void *info);

// Thread Safe functions

list_result list_insert(list *l, void *info);

void *list_search(list *l, void *info);

int list_dump(list *l, FILE *f);

void *list_delete(list *l, void *info);

list_result list_destroy(list *l);

#endif
