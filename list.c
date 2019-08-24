#include "list.h"
#include "user.h"

// Thread Unsafe functions

list *list_create(fun_info_compare info_compare,
				  fun_info_compare info_compare_field,
				  fun_info_destroy info_destroy,
				  fun_info_print info_print)
{
	list *l = (list *)calloc(1, sizeof(list));

	pthread_mutex_init(&(l->mux), NULL);
	l->head = NULL;
	l->count = 0;
	l->info_compare = info_compare;
	l->info_compare_field = info_compare_field;
	l->info_destroy = info_destroy;
	l->info_print = info_print;

	return l;
}

list_result list_insert_unsafe(list *l, void *info)
{
	if (l == NULL || info == NULL) return list_null;

	node *n = (node *)calloc(1, sizeof(node));
	n->info = info;
	n->next = l->head;
	l->head = n;
	l->count++;

	return list_success;
}

void *list_search_info_unsafe(list *l, void *info)
{
	if (l == NULL) return NULL;
	return list_search_unsafe(l, info, l->info_compare);
}

void *list_search_field_unsafe(list *l, void *field)
{
	if (l == NULL) return NULL;
	return list_search_unsafe(l, field, l->info_compare_field);
}

void *list_search_unsafe(list *l, void *info, fun_info_compare info_compare)
{
	if (l == NULL) return NULL;

	node *curr = l->head;

	while ((curr != NULL) && (info_compare(curr->info, info) != 0)) {
		curr = curr->next;
	}

	if (curr != NULL) return curr->info;
	return NULL;
}

list_result list_delete_info_unsafe(list *l, void *info)
{
	if (l == NULL) return list_null;
	return list_delete_unsafe(l, info, l->info_compare);
}

list_result list_delete_field_unsafe(list *l, void *field)
{
	if (l == NULL) return list_null;
	return list_delete_unsafe(l, field, l->info_compare_field);
}

list_result list_delete_unsafe(list *l, void *info, fun_info_compare info_compare)
{
	if (l == NULL || info == NULL) return list_null;

	node *prev = NULL;
	node *curr = l->head;

	while ((curr != NULL) && (info_compare(curr->info, info) != 0)) {
		prev = curr;
		curr = curr->next;
	}

	if ((curr != NULL) && (info_compare(curr->info, info) == 0)) {
		if (prev == NULL) {
			l->head = curr->next;
		} else {
			prev->next = curr->next;
		}
		l->info_destroy(curr->info);
		free(curr);
		l->count--;
		return list_success;
	}

	// while (curr != NULL) {
	// 	if (l->info_compare(curr->info, info) == 0) {
	// 		if (prev == NULL) {
	// 			l->head = curr->next;
	// 		} else {
	// 			prev->next = curr->next;
	// 		}
	// 		l->info_destroy(curr);
	// 		l->count--;
	// 		return list_success;
	// 	}
	// 	prev = curr;
	// 	curr = curr->next;
	// }

	return list_not_found;
}

list_result list_destroy(list *l)
{
	if (l == NULL) return list_success;

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

list_result list_lock(list *l)
{
	if (l == NULL) return list_null;
	if (pthread_mutex_lock(&(l->mux)) != 0) return list_err_mux;
	return list_success;
}

list_result list_unlock(list *l)
{
	if (l == NULL) return list_null;
	if (pthread_mutex_unlock(&(l->mux)) != 0) return list_err_mux;
	return list_success;
}

list_result list_print(list *l)
{
	if (l == NULL) return list_null;

	node *curr = l->head;
	while(curr != NULL) {
		l->info_print(curr->info);
		curr = curr->next;
	}
	return list_success;
}


//Thread Safe functions

list_result list_insert(list *l, void *info)
{
	if (list_lock(l) != 0) return list_err_mux;
	list_result l_res = list_insert_unsafe(l, info);
	if (list_unlock(l) != 0) return list_err_mux;

	return l_res;
}

node *list_search(list *l, void *info, fun_info_compare info_compare)
{
	if (list_lock(l) != 0) return NULL;
	node *n = list_search_unsafe(l, info, info_compare);
	if (list_unlock(l) != 0) return NULL;

	return n;
}

list_result list_delete(list *l, void *info, fun_info_compare info_compare)
{
	if (list_lock(l) != 0) return list_err_mux;
	list_result l_res = list_delete_unsafe(l, info, info_compare);
	if (list_unlock(l) != 0) return list_err_mux;

	return l_res;
}
