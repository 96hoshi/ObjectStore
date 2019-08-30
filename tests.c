#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "object.h"
#include "user.h"
#include "list.h"
#include "common.h"
#include "message.h"

#define OBJS_N 2
void test_object()
{
	int test = 0;
	object * objs[OBJS_N];

	for (int i = 0; i < OBJS_N; ++i) {
		char * str = (char *)calloc(2, sizeof(char));
		str[0] = 'a' + i;
		str[1] = '\0';
		objs[i] = object_create(str, i);
		printf("objs[%d] (%s, %ld)\n", i, objs[i]->name, objs[i]->len);
		free(str);
	}

	test += (object_compare(NULL,    NULL         ) !=  0);
	test += (object_compare(NULL,    objs[0]->name) !=  1);
	test += (object_compare(objs[0], NULL         ) != -1);
	test += (object_compare(objs[0], objs[1]->name) != -1);
	test += (object_compare(objs[0], objs[0]->name) !=  0);
	test += (object_compare(objs[1], objs[0]->name) !=  1);

	for (int i = 0; i < OBJS_N; ++i) {
		object_destroy(objs[i]);
	}

	printf("OBJECT TEST %s", test ? "FAILED!\n" : "PASSED!\n");
}

#define USERS_N 4
void test_list()
{
	list *user_list = NULL;
	list_result err;
	int test = 0;

	user_list = list_create(user_compare,
							user_destroy,
							user_print);


	// Insert tests
	for (int i = 0; i < USERS_N; ++i) {
		char * str = (char *)calloc(2, sizeof(char));
		str[0] = 'a' + i;
		str[1] = '\0';

		user *u = user_create(str);
		err = list_insert_unsafe(user_list, u);
		if (err != list_success) {
			test++;
		}
		free(str);
	}
	user *duplicate_u = user_create("a");
	user *wrong_u = user_create("w");

	// Search tests
	user *u = (user *)list_search_unsafe(user_list, duplicate_u->name);
	if (u == NULL) test++;
	u = (user *)list_search_unsafe(user_list, wrong_u->name);
	if (u != NULL) test++;

	// Delete tests
	err = list_delete_unsafe(user_list, duplicate_u->name);
	if (err != list_success) test++;
	err = list_delete_unsafe(user_list, wrong_u->name);
	if (err != list_not_found) test++;
	err = list_delete_info_unsafe(user_list, wrong_u);
	if (err != list_not_found) test++;

	// Print test
	err = list_print(user_list);
	if (err != 0) test++;

	// Destroy tests
	user_destroy(duplicate_u);
	user_destroy(wrong_u);
	err = list_destroy(user_list);
	if (err != list_success) test++;

	printf("LIST TEST %s", test ? "FAILED!\n" : "PASSED!\n");
}
void message_print(message *m) {
	printf("m->buff = %s\n", m->buff);
	printf("m->op = %d\n", m->op);
	printf("m->name = %s\n", m->name);
	printf("m->len = %zu\n", m->len);
	if (data != NULL)	printf("m->data = %p %c\n", m->data, m->data[0]);
}

int main(int argc, char * argv[])
{
	test_object();
	test_list();

	return 0;
}
