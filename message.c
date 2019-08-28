#define _POSIX_C_SOURCE 200112L	// needed by strtok_r

#include "message.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include "stats.h"
#include "common.h"


static char *ops[] = {
	"REGISTER",
	"STORE",
	"RETRIEVE",
	"DATA",
	"DELETE",
	"LEAVE",
	"OK",
	"KO",
	"ERROR"
};


static message_op getOp(char *op_str)
{
	for(message_op i = message_register; i < message_err; ++i) {
		if (strcmp(ops[i], op_str) == 0) {
			return i;
		}
	}
	return message_err;
}

static ssize_t getPosDelimiter(char *buffer, size_t buffer_size, char delimiter)
{
	ssize_t i;
	for (i = 0; (i < (buffer_size - 1)) && (buffer[i] != delimiter); i++);

	if (buffer[i] == delimiter) {
		return i;
	}
	return -1;
}

static size_t numberOfDigits(size_t val)
{
	if (val == 0) return 1;
	return (size_t)(log10(val) + 1);
}


// static void message_print(message *m) {
// 	printf("m->op = %s\n", ops[m->op]);
// 	printf("m->name = %s\n", (m->name != NULL ? m->name : "NULL"));
// 	printf("m->len = %zu\n", m->len);
// 	printf("m->data = [");
// 	for (size_t i = 0; i < m->len; ++i) {
// 		printf("%c", ((char *)m->data)[i]);
// 	}
// 	printf("]\n");
// }

static int myWrite(int sock, char *buffer, size_t len) {
	ssize_t n = 0;
	ssize_t written = 0;

	while ((len - written) > 0) {
		n = write(sock, buffer + written, len - written);
		if (n < 0) {
			 if (errno == EINTR) {
				continue;
			}
			return FALSE;
		}
		written += n;
	}

	return TRUE;
}

static int read_header(int sock, char *buffer, size_t buffer_size, size_t *posDelimiter, size_t *nRead)
{
	ssize_t p = -1;	// local position delimiter
	size_t r = 0;	// local number of char already read

	while (p < 0) {
		ssize_t n = read(sock, buffer + r, buffer_size - r);

		// handle connection closed
		if (n == 0) {
			return FALSE;
		}

		if (n < 0) {
			// handle delivery of a signal
			if (errno == EINTR) {
				continue;
			}
			return FALSE;
		}

		p = getPosDelimiter(buffer + r, buffer_size, '\n');
		r += n;
	}

	*posDelimiter = p;
	*nRead = r;

	return TRUE;
}

static int read_data(int sock, char *buffer, size_t buffer_size, size_t nRead)
{
	while (nRead < buffer_size) {
		ssize_t n = read(sock, buffer + nRead, buffer_size - nRead);

		// handle connection closed
		if (n == 0) {
			return FALSE;
		}

		if (n < 0) {
			// handle delivery of a signal
			if (errno == EINTR) {
				continue;
			}
			return FALSE;
		}

		nRead += n;
	}

	return TRUE;
}

message *message_create(message_op op,
						char *name,
						size_t len,
						void *data)
{
	message *m = (message *)calloc(1, sizeof(message));
	check_calloc(m, NULL);
	m->buff = NULL;
	m->op = op;
	m->name = name;
	m->len = len;
	m->data = (char *)data;

	return m;
}

message *message_receive(int sock)
{
	size_t buffer_size = MAX_BUFF; // MAX_BUFF must be at least 1024
	char *lasts = NULL;
	char *buffer = (char *)calloc(buffer_size, sizeof(char));

	size_t posDelimiter = 0;
	size_t nRead = 0;

	// read from socket until it found '\n' in the buffer.
	// Note: the buffer may contain more data after '\n'
	if (read_header(sock, buffer, buffer_size, &posDelimiter, &nRead) == FALSE) {
		free(buffer);
		return NULL;
	}

	message_op op = message_err;
	char *name = NULL;
	size_t len = 0;
	char *data = NULL;

	int offset_name = 0;
	int offset_data = 0;

	char *len_str = NULL;
	char *op_str = NULL;

	op_str = strtok_r(buffer, " ", &lasts);
	op = getOp(op_str);

	switch(op) {
		case message_register:						// REGISTER name \n
		case message_retrieve:						// RETRIEVE name \n
		case message_delete:						// DELETE name \n
			name = strtok_r(NULL, " ", &lasts);
			break;

		case message_store:							// STORE name len \n data
			offset_name = strtok_r(NULL, " ", &lasts) - buffer;
		case message_data:
			len_str = strtok_r(NULL, " ", &lasts);
			len = strtol(len_str, NULL, 10);
			offset_data = posDelimiter + 2;

			buffer_size = offset_data + len;
			if (buffer_size > MAX_BUFF) {
				buffer = realloc(buffer, buffer_size);
				if (buffer == NULL) {
					free(buffer);
					return NULL;
				}
			}
			if (read_data(sock, buffer, buffer_size, nRead) == FALSE) {
				free(buffer);
				return NULL;
			}

			name = buffer + offset_name;
			data = buffer + offset_data;

			break;

		case message_leave:							// LEAVE \n
		case message_ok:							// OK \n
			break;

		case message_ko:							// KO message \n
			name = strtok_r(NULL, "\n", &lasts);
			break;

		case message_err:
		default:
			invalid_operation(NULL);
			break;
	}

	message *m = message_create(op, name, len, data);
	m->buff = buffer;

	return m;
}

int message_send(int sock, message *m)
{
	char *buffer = NULL;
	message_op op = m->op;
	char *name = m->name;
	size_t len = m->len;
	char *data = m->data;

	size_t size = 0;
	size_t len_op = 0;
	size_t len_name = 0;
	size_t len_digits = 0;
	size_t offset = 0;

	switch (op) {

		case message_register:						// "REGISTER nome \n"
		case message_retrieve:						// "RETRIEVE nome \n"
		case message_delete:						// "DELETE nome \n"
			len_op = strlen(ops[op]);
			len_name = strlen(name);
			// op + space + name + space + '\n'
			size = len_op + 1 + len_name + 1 + 1;

			// size + '\0' needed by snprintf
			buffer = (char *)calloc(size + 1, sizeof(char));
			check_calloc(buffer, NULL);
			snprintf(buffer, size + 1, "%s %s \n", ops[op], name);
			break;

		case message_store:							// "STORE name len \n data"
			len_op = strlen(ops[op]);
			len_name = strlen(name);
			len_digits = numberOfDigits(len);

			// op + space + name + space + len + space + '\n' + space + dataSize
			size = len_op + 1 + len_name + 1 + len_digits + 1 + 1 + 1 + len;

			// size + '\0' needed by snprintf
			buffer = (char *)calloc(size + 1, sizeof(char));
			check_calloc(buffer, NULL);
			offset = snprintf(buffer, size + 1, "%s %s %zu \n ", ops[op], name, len);
			buffer[offset] = ' ';
			memcpy(buffer + offset, data, len);
			break;

		case message_data:							// "DATA len \n data"
			len_op = strlen(ops[op]);
			len_digits = numberOfDigits(len);

			// op + space + len + space + '\n' + space + dataSize
			size = len_op + 1 + len_digits + 1 + 1 + 1 + len;

			// size + '\0' needed by snprintf
			buffer = (char *)calloc(size + 1, sizeof(char));
			check_calloc(buffer, NULL);
			offset = snprintf(buffer, size + 1, "%s %zu \n ", ops[op], len);
			buffer[offset] = ' ';
			memcpy(buffer + offset, data, len);
			free(data);
			break;

		case message_leave:							// "LEAVE \n"
		case message_ok:							// "OK \n"
			len_op = strlen(ops[op]);

			// op + space + '\n'
			size = len_op + 1 + 1;

			// size + '\0' needed by snprintf
			buffer = (char *)calloc(size + 1, sizeof(char));
			check_calloc(buffer, NULL);
			snprintf(buffer, size + 1, "%s \n", ops[op]);
			break;

		case message_ko:							// "KO message \n"
			len_op = strlen(ops[op]);
			len_name = (name != NULL ? strlen(name) : 0);

			// op + space + message + space + '\n'
			size = len_op + 1 + len_name + 1 + 1;

			// size + '\0' needed by snprintf
			buffer = (char *)calloc(size + 1, sizeof(char));
			check_calloc(buffer, NULL);
			snprintf(buffer, size + 1, "%s %s \n", ops[op], name);
			break;

		case message_err:
		default:
			invalid_operation(NULL);
			break;
	}

	m->buff = buffer;

	return myWrite(sock, buffer, size);
}

void message_destroy(message *m)
{
	free(m->buff);
	free(m);
}
