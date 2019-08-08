#include "list.h"

// Thread Unsafe functions

list *list_create(fun_info_compare info_compare,
				  fun_info_destroy info_destroy)
{
	list *new = (list *)calloc(1, sizeof(list));

	pthread_mutex_init(&(new->mux), NULL);
	*(new->head) = NULL;
	new->count = 0;
	new->info_compare = info_compare;
	new->info_destroy = info_destroy;

	return new;
}

void list_insert(list *l, void *info)
{
	if (l == NULL || info == NULL) return;
	if (list_search(l, info) != NULL) return;

	node *n = (node *)calloc(1, sizeof(node));

	n->info = info;
	n->next = *(l->head);
	*(l->head) = n;
	l->count++;
}

node *list_search(list *l, void *info)
{
	if (l == NULL) return NULL;

	node *curr = *(l->head);

	while ((curr) != NULL) {
		if (!l->info_compare(curr->info, info)) {
			return curr;
		}
		curr = curr->next;
	}

	return NULL;
}

void list_delete(list *l, void *info)
{
	if (l == NULL || info == NULL) return;

	node *prev = NULL;
	node *curr = *(l->head);

	while (curr != NULL) {
		if (!l->info_compare(curr->info, info)) {
			if (prev == NULL) {
				*(l->head) = curr->next;
			} else {
				prev->next = curr->next;
			}
			l->info_destroy(curr);
			l->count--;
			return;
		}
		prev = curr;
		curr = curr->next;
	}
}

void list_destroy(list *l)
{
	if (l == NULL) return;

	while (*(l->head) != NULL) {
		node *to_delete = *(l->head);

		*(l->head) = (*(l->head))->next;
		l->info_destroy(to_delete);
		l->count--;
	}
	pthread_mutex_destroy(&(l->mux));
	free(l);
}

void list_lock(list *l)
{
	pthread_mutex_lock(&(l->mux));
}

void list_unlock(list *l)
{
	pthread_mutex_unlock(&(l->mux));
}


//Thread Safe functions

list *list_create_s(fun_info_compare info_compare,
					fun_info_destroy info_destroy)
{
	return list_create(info_compare, info_destroy);
}

void list_insert_s(list *l, void *info)
{
	if (l == NULL || info == NULL) return;

	list_lock(l);
	list_insert(l, info);
	list_unlock(l);
}

node *list_search_s(list *l, void *info)
{
	if (l == NULL || info == NULL) return NULL;

	list_lock(l);
	node *n = list_search(l, info);
	list_unlock(l);

	return n;
}

void list_delete_s(list *l, void *info)
{
	if (l == NULL || info == NULL) return;

	list_lock(l);
	list_delete(l, info);
	list_unlock(l);
}

void list_destroy_s(list *l)
{
	if (l == NULL) return;

	list_lock(l);
	list_destroy_s(l);
	list_unlock(l);
}
