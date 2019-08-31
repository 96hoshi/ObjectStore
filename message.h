#ifndef MESSAGE_H
#define MESSAGE_H

#include <stdlib.h>


typedef enum {
	message_register = 0,
	message_store,
	message_retrieve,
	message_data,
	message_delete,
	message_leave,
	message_ok,
	message_ko,
	message_err
} message_op;

typedef struct {
	char *buff;
	message_op op;
	char *name;
	size_t len;
	char *data;
} message;


message *message_create(message_op op,
						char *name,
						size_t len,
						void *data);

message *message_receive(long sock);

int message_send(long sock, message *m);

void *message_extract_data(message *m);

void message_destroy(message *m);

#endif
