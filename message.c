// Traduce i messaggi tra client (request) e server (response) in strutture "leggibili".

// Messaggio = "header" ,codificato in ASCII, è una stringa di testo terminata da "\n"
// a cui può succedere un blocco di dati.

// Cosa posso trovare nei messaggi:
// 	op 	= array di caratteri che indica l'operazione da eseguire
// 	name 	= array di caratteri che contiene il nome del client o del dato
// 	len 	= lunghezza in byte del dato, codificato in ASCII
// 	data 	= blocco binario i lunghezza len, posto dopo lo "\n"
// 	message = array di caratteri che contiene in messaggio di errore del server

// Struttura dati:
// -msg:
// 	-int 
// 	-char* name
// 	-size_t len
// 	-void* data (che conterrà anche message in caso)

#define _POSIX_C_SOURCE 200112L // per strtok_r in c99

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


// Creare un array di tringhe static (locale al modulo)
// per tenere traccia delle operazioni dei messaggi:
// A ogni indice dell'array corrisponde l'operazione dell'enum nell'header
// ops[msg->message_op] per lo switch case
static char* ops[] = {
	"REGISTER",
	"STORE",
	"RETRIEVE",
	"DATA",
	"DELETE",
	"LEAVE",
	"OK",
	"KO",
	"ERR"
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

static int checkDelimiter(const char * buffer, const size_t buffer_size, const char delimiter)
{
	size_t i;
	for (i = 0; (i < (buffer_size - 1)) && (buffer[i] != delimiter); i++);
	return (buffer[i] == delimiter);
}

static void my_write(int sock, char *buffer, size_t len) {
	ssize_t n = 0;
	ssize_t written = 0;
	while ((len - written) > 0) {
		n = write(sock, buffer + written, len - written);
		if (n < 0 ) {
			 if (errno == EINTR) {
				continue;
			}
			break; // TODO: handle write error
		}
		written += n;
	}
}

static int my_read(int sock,
				   char * buffer,
				   size_t * buffer_size,
				   ssize_t * nRead)
{
	ssize_t n = 0;

	if (*nRead == *buffer_size) {
		*buffer_size = *buffer_size * 2;
		char * buffer_new = realloc(buffer, *buffer_size);
		if (buffer_new == NULL) {
			free(buffer);
			exit(EXIT_FAILURE); //TODO: replace exit with something useful
		}
		buffer = buffer_new;
	}

	n = read(sock, buffer + *nRead, *buffer_size - *nRead);

	if (n == 0) {   // the client closed the connection
		return FALSE;   // TODO: replace exit with something useful
	}
	if (n < 0) {
		if (errno == EINTR) {   // if the call was canceled by an interrupt
			return TRUE;
		}
		return FALSE;
	}
	*nRead += n;
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
	size_t buffer_size = MAX_BUFF;
	char * lasts = NULL;
	char * buffer = (char *)calloc(buffer_size, sizeof(char));

	ssize_t n = 0;
	ssize_t nRead = 0;

	// read from socket until it found '\n' in the buffer.
	// Note: the buffer may contain more data after '\n'
	do {
		n += nRead;
		my_read(sock, buffer, &buffer_size, &nRead);
	} while (checkDelimiter(buffer + n, buffer_size, '\n') == 0);

	message_op op = message_err;
	char * name = NULL;
	size_t len = 0;
	char * data = NULL;

	char * len_str = NULL;
	char * op_str = NULL;

	size_t lastToRead = 0;
	// 	lastToRead
	// (lasts - buffer) = the number of bytes already read up to "\n" (not included)
	// 2 count for the two char "\n " before "data" 
	// len = number of bytes of "data"

	op_str = strtok_r(buffer, " ", &lasts); // get the op string
	op = getOp(op_str);

	switch(op) {

		case message_register:		// REGISTER name \n
		case message_retrieve:		// RETRIEVE name \n
		case message_delete:		// DELETE name \n
			name = strtok_r(NULL, " ", &lasts); // get the name
			break;

		case message_store:			// STORE name len \n data
			name = strtok_r(NULL, " ", &lasts); // get the name
			len_str = strtok_r(NULL, " ", &lasts);  // get the len
			len = strtol(len_str, NULL, 10);
			data = lasts + 2;
			lastToRead = (lasts - buffer) + 2 + len;

			while (nRead < lastToRead) {
				if (my_read(sock, buffer, &buffer_size, &nRead) == TRUE) {
					continue;
				} else {
					free(buffer);
					exit(EXIT_FAILURE);   // if there is another type of error
				}
			}
			break;

		case message_data:			// DATA len \n data
			len_str = strtok_r(NULL, " ", &lasts);  // get the len
			len = strtol(len_str, NULL, 10);
			data = lasts + 2;
			lastToRead = (lasts - buffer) + 2 + len;

			while (nRead < lastToRead) {
				if (my_read(sock, buffer, &buffer_size, &nRead) == TRUE) {
					continue;
				} else {
					exit(EXIT_FAILURE);   // TODO: handle if there is another type of error
				}
			}
			break;

		case message_leave:			// LEAVE \n
		case message_ok:			// OK \n
			break;

		case message_ko:			// KO message \n
			name = strtok_r(NULL, "\n", &lasts); // get the message
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

void message_send(int sock, message *m)
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

		case message_register:		// "REGISTER nome \n"
		case message_retrieve:		// "RETRIEVE nome \n"
		case message_delete:		// "DELETE nome \n"
			len_op = strlen(ops[op]);
			len_name = strlen(name);
			//		OP + space + name + space + '\n'
			size =  len_op + 1 + len_name + 1 + 1;

			//size + 1 for '\0' puts by sprintf
			buffer = (char *)calloc(size + 1, sizeof(char));
			check_calloc(buffer, NULL);
			sprintf(buffer, "%s %s \n", ops[op], name);
			break;

		case message_store:			// "STORE name len \n data"
			if (len == 0)	len_digits = 1;
			if (len > 0)	len_digits = (int)(log10(len) + 1);
			len_op = strlen(ops[op]);
			len_name = strlen(name); 
			//		OP + space + name + space + len + space + '\n' + space + dataSize
			size = len_op + 1 + len_name + 1 + len_digits + 1 + 1 + 1 + len;

			//size + 1 for '\0' puts by sprintf
			buffer = (char *)calloc(size + 1, sizeof(char));
			check_calloc(buffer, NULL);
			offset = sprintf(buffer, "%s %s %zu \n ", ops[op], name, len);
			memcpy(buffer + offset, data, len);
			break;

		case message_data:			// "DATA len \n data"
			if (len == 0)	len_digits = 1;
			if (len > 0)	len_digits = (int)(log10(len) + 1);
			len_op = strlen(ops[op]);
			//		OP + space + len + space + '\n' + space + dataSize
			size = len_op + 1 + len_digits + 1 + 1 + 1 + len;

			//size + 1 for '\0' puts by sprintf
			buffer = (char *)calloc(size + 1, sizeof(char));
			check_calloc(buffer, NULL);
			offset = sprintf(buffer, "%s %zu \n ", ops[op], len);
			memcpy(buffer + offset, data, len);
			break;

		case message_leave:			// "LEAVE \n"
		case message_ok:			// "OK \n"
			len_op = strlen(ops[op]);
			//		OP + space + '\n'
			size = len_op + 1 + 1;

			//size + 1 for '\0' puts by sprintf
			buffer = (char *)calloc(size + 1, sizeof(char));
			check_calloc(buffer, NULL);
			sprintf(buffer, "%s \n", ops[op]);
			break;

		case message_ko:			// "KO message \n"
			len_op = strlen(ops[op]);
			len_name = strlen(name);
			//		OP + space + message + space + '\n'
			size = len_op + 1 + len_name + 1 + 1;

			//size + 1 for '\0' puts by sprintf
			buffer = (char *)calloc(size + 1, sizeof(char));
			check_calloc(buffer, NULL);
			sprintf(buffer, "%s %s \n", ops[op], name);
			break;

		case message_err:
		default:
			invalid_operation(NULL);
			break;
	}
	m->buff = buffer;

	my_write(sock, buffer, size);
}

// TODO: destroy con doppio puntatore?
void message_destroy(message *m)
{
	free(m->buff);
	free(m);
}
