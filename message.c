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

static ssize_t getPosDelimiter(char *buffer, size_t buffer_size, char delimiter)
{
	ssize_t i;
	for (i = 0; (i < (buffer_size - 1)) && (buffer[i] != delimiter); i++);

	if (buffer[i] == delimiter) {
		return i;
	}
	return -1;
}

static void myWrite(int sock, char *buffer, size_t len) {
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

static void message_print(message *m) {
	printf("m->op = %s\n", ops[m->op]);
	printf("m->name = %s\n", (m->name != NULL ? m->name : "NULL"));
	printf("m->len = %zu\n", m->len);
	printf("m->data = [");
	for (size_t i = 0; i < (m->len > 2 ? 2 : 0); ++i) {
		printf("%c", ((char *)m->data)[i]);
	}
	printf("]\n");
}

message *message_receive(int sock)
{
	size_t buffer_size = MAX_BUFF; // MAX_BUFF must be at least 1024
	char * lasts = NULL;
	char * buffer = (char *)calloc(buffer_size, sizeof(char));

	size_t posDelimiter = 0;
	size_t nRead = 0;

	// read from socket until it found '\n' in the buffer.
	// Note: the buffer may contain more data after '\n'
	
	if (read_header(sock, buffer, buffer_size, &posDelimiter, &nRead) == FALSE) {
		free(buffer);
		exit(EXIT_FAILURE); //TODO: replace exit with something useful
	}

	message_op op = message_err;
	char * name = NULL;
	size_t len = 0;
	char * data = NULL;

	char * len_str = NULL;
	char * op_str = NULL;

	op_str = strtok_r(buffer, " ", &lasts); // get the operation string
	op = getOp(op_str);

	switch(op) {

		case message_register:		// REGISTER name \n
		case message_retrieve:		// RETRIEVE name \n
		case message_delete:		// DELETE name \n
			name = strtok_r(NULL, " ", &lasts); // get the name
			break;

		case message_store:			// STORE name len \n data
			name = strtok_r(NULL, " ", &lasts);		// get the name
		case message_data:			// DATA len \n data
			len_str = strtok_r(NULL, " ", &lasts);	// get the length of data
			data = lasts + 2;						// get the pointer of data
			len = strtol(len_str, NULL, 10);		// convert len_str to len

			buffer_size = posDelimiter + 2 + len;
			if (buffer_size > MAX_BUFF) {
				buffer = realloc(buffer, buffer_size);
				if (buffer == NULL) {
					free(buffer);
					exit(EXIT_FAILURE); //TODO: replace exit with something useful
				}
			}
			
			if (read_data(sock, buffer, buffer_size, nRead) == FALSE) {
				free(buffer);
				exit(EXIT_FAILURE); //TODO: replace exit with something useful
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

	printf("Message received:\n");
	// message_print(m);

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
			len_name = (name != NULL ? strlen(name) : 0);
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

	myWrite(sock, buffer, size);
}

// TODO: destroy con doppio puntatore?
void message_destroy(message *m)
{
	free(m->buff);
	free(m);
}
