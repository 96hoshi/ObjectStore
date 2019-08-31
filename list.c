#include "list.h"


static void listLock(list *l)
{
	pthread_mutex_lock(&(l->mux));
}

static void listUnlock(list *l)
{
	pthread_mutex_unlock(&(l->mux));
}


list *list_create(fun_info_compare info_compare,
				  fun_info_destroy info_destroy,
				  fun_info_dump info_dump)
{
	list *l = (list *)calloc(1, sizeof(list));
	if (l == NULL) {
		return NULL;
	}

	pthread_mutex_init(&(l->mux), NULL);
	l->head = NULL;
	l->count = 0;
	l->info_compare = info_compare;
	l->info_destroy = info_destroy;
	l->info_dump = info_dump;

	return l;
}

list_result list_insert_unsafe(list *l, void *info)
{
	if (l == NULL) {
		return list_null;
	}

	node *n = (node *)calloc(1, sizeof(node));
	if (n == NULL) {
		return list_null;
	}

	n->info = info;
	n->next = l->head;
	l->head = n;
	l->count++;

	return list_success;
}

void *list_search_unsafe(list *l, void *info)
{
	if (l == NULL) {
		return NULL;
	}

	node *curr = l->head;
	while ((curr != NULL) && (l->info_compare(curr->info, info) != 0)) {
		curr = curr->next;
	}

	if (curr != NULL) {
		return curr->info;
	}
	return NULL;
}

void *list_delete_unsafe(list *l, void *info)
{
	if (l == NULL || info == NULL) {
		return NULL;
	}

	node *prev = NULL;
	node *curr = l->head;
	while ((curr != NULL) && (l->info_compare(curr->info, info) != 0)) {
		prev = curr;
		curr = curr->next;
	}

	if ((curr != NULL) && (l->info_compare(curr->info, info) == 0)) {
		if (prev == NULL) {
			l->head = curr->next;
		} else {
			prev->next = curr->next;
		}
		void *curr_info = curr->info;
		free(curr);
		l->count--;
		return curr_info;
	}

	return NULL;
}

list_result list_insert(list *l, void *info)
{
	listLock(l);
	list_result l_res = list_insert_unsafe(l, info);
	listUnlock(l);

	return l_res;
}

void *list_search(list *l, void *info)
{
	listLock(l);
	void *i = list_search_unsafe(l, info);
	listUnlock(l);

	return i;
}

int list_dump(list *l, FILE *f)
{
	if (l == NULL) {
		return list_null;
	}
	listLock(l);

	fprintf(f, "%zu\n", l->count);

	node *curr = l->head;
	while(curr != NULL) {
		l->info_dump(curr->info, f);
		curr = curr->next;
	}
	listUnlock(l);

	return list_success;
}

void *list_delete(list *l, void *info)
{
	listLock(l);
	void *i = list_delete_unsafe(l, info);
	listUnlock(l);

	return i;
}

list_result list_destroy(list *l)
{
	if (l == NULL) {
		return list_success;
	}

	while (l->head != NULL) {
		node *to_delete = l->head;

		l->head = l->head->next;
		l->info_destroy(to_delete->info);
		free(to_delete);
		l->count--;
	}
	pthread_mutex_destroy(&(l->mux));
	free(l);

	return list_success;
}
