#include <stdio.h>
#include <stdlib.h>
#include "object.h"

#define OBJS_N 2
void test_object() {

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

	test += (object_compare(NULL,    NULL   ) !=  0);
	test += (object_compare(NULL,    objs[0]) !=  1);
	test += (object_compare(objs[0], NULL   ) != -1);
	test += (object_compare(objs[0], objs[1]) != -1);
	test += (object_compare(objs[0], objs[0]) !=  0);
	test += (object_compare(objs[1], objs[0]) !=  1);

	for (int i = 0; i < OBJS_N; ++i) {
		object_destroy(objs[i]);
	}

	printf("OBJECT TEST %s", test ? "FAILED!\n" : "PASSED!\n");
}

int main(int argc, char * argv[]) {

	test_object();

	return 0;
}
