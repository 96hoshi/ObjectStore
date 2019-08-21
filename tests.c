#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "object.h"
#include "user.h"
#include "list.h"
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
void message_print(message *m) {
	printf("m->buff = %s\n", m->buff);
	printf("m->OP = %d\n", m->OP);
	printf("m->name = %s\n", m->name);
	printf("m->len = %d\n", m->len);
	printf("m->data = %p\n", m->data);
}

void test_message()
{
	int test = 0;

	char data[5] = "ciao";

	message *msg = message_create(message_store, "pluto", 5, (void *)data);
	message_print(msg);

	char *prova = message_to_string(msg);
	printf("MESSAGE_TO_STRING try: %s\n", prova);
	message_destroy(msg);
	//free(data);

	message *a = message_create(message_register, "pluto", -1, NULL);
	char *b = message_to_string(a);
	printf("MESSAGE_TO_STRING try2: %s\n", b);

	message_destroy(a);

	// char *string = (char *)calloc(strlen("STORE pippo 5 \n aooo") + 1, sizeof(char));
	// strcpy(string, "STORE pippo 5 \n ao ok");

	// message *m = string_to_message(string);
	// message_print(m);
	// message_destroy(m);

	printf("MESSAGE TEST %s", test ? "FAILED!\n" : "PASSED!\n");
}

int main(int argc, char * argv[])
{
	// test_object();
	// test_list();
	test_message();

	return 0;
}
