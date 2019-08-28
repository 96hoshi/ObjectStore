#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>

#define TRUE  1
#define FALSE 0

#define MAX_BUFF 1024

#define UNIX_PATH_MAX	108
#define SOCKNAME		"./objstore.sock"

inline static void check_calloc(void *buff, void (*err_callback)(void))
{
	if (buff == NULL) {
		if (err_callback == NULL) {
			fprintf(stderr,"ERR: calloc error\n");
		} else {
			err_callback();
		}
		exit(EXIT_FAILURE);
	}
}

inline static void invalid_operation(void (*err_callback)(void))
{
	if (err_callback == NULL) {
		fprintf(stderr,"ERR: invalid operation\n");
	} else {
		err_callback();
	}
	exit(EXIT_FAILURE);
}


#endif
