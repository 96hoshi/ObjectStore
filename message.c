#define _POSIX_C_SOURCE 200112L	// needed by strtok_r

#include "message.h"
#include <stdio.h>
// #include <stdlib.h>
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

static int myWrite(long sock, char *buffer, size_t len) {
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

static char *readHeader(long sock, char *buffer, size_t *buffer_size, size_t *pos_delimiter, size_t *nread)
{
	char *b = buffer;			// local buffer pointer
	size_t s = *buffer_size;	// local buffer size
	size_t r = 0;				// local number of bytes already read
	ssize_t p = -1;				// local position delimiter

	while (p < 0) {

		if (r == s) {
			s = s * 2;
			char *b_new = realloc(b, s);
			if (b_new == NULL) {
				free(b);
				return NULL;
			}
			b = b_new;
		}

		ssize_t n = read(sock, b + r, s - r);

		// closed connection
		if (n == 0) return NULL;

		if (n < 0) {
			// delivery of a signal
			if (errno == EINTR) {
				continue;
			}
			return NULL;
		}

		p = getPosDelimiter(buffer + r, s, '\n');
		r += n;
	}

	// update values
	*buffer_size = s;
	*pos_delimiter = p;
	*nread = r;

	return b;
}

static int readData(long sock, char *buffer, size_t buffer_size)
{
	size_t r = 0;
	while (r < buffer_size) {
		ssize_t n = read(sock, buffer + r, buffer_size - r);

		// closed connection
		if (n == 0) {
			return FALSE;
		}

		if (n < 0) {
			// delivery of a signal
			if (errno == EINTR) {
				continue;
			}
			return FALSE;
		}

		r += n;
	}

	return TRUE;
}


message *message_create(message_op op, char *name, size_t len, void *data)
{
	message *m = (message *)calloc(1, sizeof(message));
	if (m == NULL) return NULL;

	m->buff = NULL;
	m->op = op;
	m->name = name;
	m->len = len;
	m->data = (char *)data;

	return m;
}

message *message_receive(long sock)
{
	size_t header_size = MAX_BUFF;
	char *header = (char *)calloc(header_size, sizeof(char));
	if (header == NULL) return NULL;
	char *lasts = NULL;

	size_t pos_delimiter = 0;
	size_t nread = 0;

	header = readHeader(sock, header, &header_size, &pos_delimiter, &nread);
	if (header == NULL) return NULL;

	message_op op = message_err;
	char *name = NULL;
	size_t len = 0;
	char *data = NULL;

	char *len_str = NULL;
	char *op_str = NULL;

	ssize_t offset = 0;

	op_str = strtok_r(header, " ", &lasts);
	op = getOp(op_str);

	switch(op) {
		case message_register:						// REGISTER name \n
		case message_retrieve:						// RETRIEVE name \n
		case message_delete:						// DELETE name \n
			name = strtok_r(NULL, " ", &lasts);
			break;

		case message_store:							// STORE name len \n data
			name = strtok_r(NULL, " ", &lasts);
		case message_data:							// "DATA len \n data"
			len_str = strtok_r(NULL, " ", &lasts);
			len = strtol(len_str, NULL, 10);

			data = (char *)calloc(len, sizeof(char));
			if (data == NULL) {
				free(header);
				return NULL;
			}

			// get the number of data-bytes already read
			offset = nread - (pos_delimiter + 2);
			// copy the data-bytes already read into data
			memcpy(data, header + pos_delimiter + 2, offset);

			// if there is more to read
			if (len > offset) {
				if (readData(sock, data + offset, len - offset) == FALSE) {
					free(header);
					free(data);
					return NULL;
				}
			}
			break;

		case message_leave:							// LEAVE \n
		case message_ok:							// OK \n
			break;

		case message_ko:							// KO message \n
			name = strtok_r(NULL, "\n", &lasts);
			break;

		case message_err:
		default:
			return NULL;
	}

	message *m = message_create(op, name, len, data);
	m->buff = header;

	return m;
}

int message_send(long sock, message *m)
{
	char *header = NULL;
	message_op op = m->op;
	char *name = m->name;
	size_t len = m->len;
	char *data = m->data;

	size_t size = 0;
	size_t len_op = 0;
	size_t len_name = 0;
	size_t len_digits = 0;

	len_op = strlen(ops[op]);

	switch (op) {

		case message_register:						// "REGISTER nome \n"
		case message_retrieve:						// "RETRIEVE nome \n"
		case message_delete:						// "DELETE nome \n"
			len_name = strlen(name);
			// op + space + name + space + '\n'
			size = len_op + 1 + len_name + 1 + 1;

			header = (char *)calloc(size + 1, sizeof(char));
			if (header == NULL) {
				return FALSE;
			}
			snprintf(header, size + 1, "%s %s \n", ops[op], name);
			break;

		case message_store:							// "STORE name len \n data"
			len_name = strlen(name);
			len_digits = numberOfDigits(len);
			// op + space + name + space + len + space + '\n' + space
			size = len_op + 1 + len_name + 1 + len_digits + 1 + 1 + 1;

			header = (char *)calloc(size + 1, sizeof(char));
			if (header == NULL) {
				return FALSE;
			}
			snprintf(header, size + 1, "%s %s %zu \n ", ops[op], name, len);
			break;

		case message_data:							// "DATA len \n data"
			len_digits = numberOfDigits(len);
			// op + space + len + space + '\n' + space + dataSize
			size = len_op + 1 + len_digits + 1 + 1 + 1;

			header = (char *)calloc(size + 1, sizeof(char));
			if (header == NULL) {
				return FALSE;
			}
			snprintf(header, size + 1, "%s %zu \n ", ops[op], len);
			break;

		case message_leave:							// "LEAVE \n"
		case message_ok:							// "OK \n"
			// op + space + '\n'
			size = len_op + 1 + 1;

			header = (char *)calloc(size + 1, sizeof(char));
			if (header == NULL) {
				return FALSE;
			}
			snprintf(header, size + 1, "%s \n", ops[op]);
			break;

		case message_ko:							// "KO message \n"
			len_name = (name != NULL ? strlen(name) : 0);
			// op + space + message + space + '\n'
			size = len_op + 1 + len_name + 1 + 1;

			header = (char *)calloc(size + 1, sizeof(char));
			if (header == NULL) {
				return FALSE;
			}
			snprintf(header, size + 1, "%s %s \n", ops[op], name);
			break;

		case message_err:
		default:
			return FALSE;
	}

	m->buff = header;

	// send header
	if (myWrite(sock, header, size) == FALSE) {
		return FALSE;
	}

	// send data if needed
	if (op == message_store || op == message_data) {
		return myWrite(sock, data, len);
	}

	return TRUE;
}

// Now message m no more contains data pointer
void *message_extract_data(message *m)
{
	if (m == NULL) return NULL;
	void *data = m->data;
	m->data = NULL;

	return data;
}

void message_destroy(message *m)
{
	if (m == NULL) return;
	if (m->buff != NULL) free(m->buff);
	if (m->data != NULL) free(m->data);
	free(m);
}
