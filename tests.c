#include <stdio.h>
#include <stdlib.h>
#include "object.h"
#include "user.h"
#include "list.h"

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

	test += (object_compare_name(NULL,    NULL         ) !=  0);
	test += (object_compare_name(NULL,    objs[0]->name) !=  1);
	test += (object_compare_name(objs[0], NULL         ) != -1);
	test += (object_compare_name(objs[0], objs[1]->name) != -1);
	test += (object_compare_name(objs[0], objs[0]->name) !=  0);
	test += (object_compare_name(objs[1], objs[0]->name) !=  1);

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
							user_compare_name,
							user_destroy,
							user_print);


	// Insert tests
	for (int i = 0; i < USERS_N; ++i) {
		char * str = (char *)calloc(2, sizeof(char));
		str[0] = 'a' + i;
		str[1] = '\0';

		user *u = user_create(str, i);
		err = list_insert_unsafe(user_list, u);
		if (err != list_success) {
			test++;
		}
		free(str);
	}

	user *duplicate_u = user_create("a", -1);
	user *wrong_u = user_create("w", -1);

	err = list_insert_unsafe(user_list, duplicate_u);
	if (err != list_duplicate) test++;


	// Search_field tests
	user *u = (user *)list_search_field_unsafe(user_list, duplicate_u->name);
	if (u == NULL) test++;

	u = (user *)list_search_field_unsafe(user_list, wrong_u->name);
	if (u != NULL) test++;


	// Search_info tests
	u = (user *)list_search_info_unsafe(user_list, wrong_u);
	if (u != NULL) test++;

	u = (user *)list_search_info_unsafe(user_list, duplicate_u);
	if (u == NULL) test++;


	// Compare tests
	u = (user *)list_search_info_unsafe(user_list, duplicate_u);
	err = user_compare_name(u, duplicate_u->name);
	if (err != 0) test++;

	err = user_compare_name(u, wrong_u->name);
	if (err == 0) test++;


	// Delete tests
	err = list_delete_field_unsafe(user_list, duplicate_u->name);
	if (err != list_success) test++;

	err = list_delete_field_unsafe(user_list, wrong_u->name);
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

int main(int argc, char * argv[])
{
	test_object();
	test_list();

	return 0;
}
