// Traduce i messaggi tra client (request) e server (response) in strutture "leggibili".

// Messaggio = "header" ,codificato in ASCII, è una stringa di testo terminata da "\n"
// a cui può succedere un blocco di dati.

// Cosa posso trovare nei messaggi:
// 	 	= array di caratteri che indica l'operazione da eseguire
// 	name 	= array di caratteri che contiene il nome del client o del dato
// 	len 	= lunghezza in byte del dato, codificato in ASCII
// 	data 	= blocco binario i lunghezza len, posto dopo lo "\n"
// 	message = array di caratteri che contiene in messaggio di errore del server

// Struttura dati:
// -msg:
// 	-int 
// 	-char* name
// 	-int len
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
// ops[msg->message_] per lo switch case
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

static void write_buffer(int sock, char *buffer, size_t len) {
	ssize_t n = 0;
	ssize_t written = 0;
	while ((n = write(sock, buffer + written, len - written)) != 0) {
		if (n < 0 && errno == EINTR) {
			if (_print_stats == TRUE) {
				continue;
			}
			break;
		}
		written += n;
	}
}

static char *read_buffer(int sock)
{
	ssize_t n = 0;
	int max_len = MAX_BUFF;
	int len = 0;
	char *buff = (char*)calloc(MAX_BUFF, sizeof(char));
	check_calloc(buff, NULL);

	while ((n = read(sock, buff + len, max_len)) != 0) {
		if (n < 0 && errno == EINTR) {
			if (_print_stats == TRUE) {
				continue;
			}
			break;
		}
		if (n == max_len) {
			len += n;
			max_len *= 2;
			char *newbuff = (char*)realloc(buff, max_len);
			check_calloc(newbuff, NULL);
			buff = newbuff;
		}
	}

	return buff;
}

static message_ getOp(char *op_str)
{
	for(message_ i = message_register; i <= message_ko; ++i) {
		if (strcmp(ops[i], op_str) == 0) {
			return i;
		}
	}
	return message_err;
}

message *message_create(char *buff,
						message_ ,
						char *name,
						int len,
						void *data)
{
	message *m = (message *)calloc(1, sizeof(message));
	check_calloc(m, NULL);
	m->buff = buff;
	m-> = ;
	m->name = name;
	m->len = len;
	m->data = (char *)data;

	return m;
}


message *message_receive(int sock)
{
	char *header = read_buffer(sock);

	message_ op = message_err;
	char *name = NULL;
	int len = -1;
	char *data = NULL;
	char *string_len = NULL;

	char *save = NULL;
	char *op_str = strtok_r(header, " \n", &save);
	op = getOp(op_str);


	switch(op) {

		case message_register:			// "REGISTER nome \n"
		case message_retrieve:			// "RETRIEVE nome \n"
		case message_delete:			// "DELETE nome \n"
			name = strtok_r(NULL, " ", &save);
			if (save[0] != '\n') {
				invalid_operation(NULL);
			}
			break;

		case message_store:				// "STORE name len \n data"
			name = strtok_r(NULL, " ", &save);
			string_len = strtok_r(NULL, "\n", &save);
			len = strtol(string_len, NULL, 10);
			data = save + 1;
			break;

		case message_data:				// "DATA len \n data"
			string_len = strtok_r(NULL, "\n", &save);
			len = strtol(string_len, NULL, 10);
			data = save + 1;
			break;

		case message_leave:				// "LEAVE \n"
		case message_ok:				// "OK \n"
			if (save[0] != '\n') {
				invalid_operation(NULL);
			}
			break;

		case message_ko:				// "KO message \n"
			data = strtok_r(NULL, "\n", &save);
			break;

		case message_err:
		default:
			invalid_operation(NULL);
			break;
	}
	message *m = message_create(header, op, name, len, (void *)data);

	return m;
}

// Crea, in base all'operazione, una stringa con i valori contenuti in m
void message_send(int sock, message *m)
{
	char *buff = NULL;
	message_ op = m->;
	char *name = m->name;
	int len = m->len;
	char *data = m->data;

	int size = 0;
	int len_op = 0;
	int len_name = 0;
	int len_digits = 0;
	int offset = 0;

	switch (op) {

		case message_register:			// "REGISTER nome \n"
		case message_retrieve:			// "RETRIEVE nome \n"
		case message_delete:			// "DELETE nome \n"
			len_op = strlen(ops[op]);
			len_name = strlen(name);
			size =  len_op + 1 + len_name + 1 + 1;

			buff = (char *)calloc(size, sizeof(char));
			check_calloc(buff, NULL);
			sprintf(buff, "%s %s \n", ops[op], name);
			break;

		case message_store:				// "STORE name len \n data"
			if (len < 0)	invalid_operation(NULL);
			if (len == 0)	len_digits = 1;
			if (len > 0)	len_digits = (int)(log10(len) + 1);
			len_op = strlen(ops[op]);
			len_name = strlen(name); 
			size = len_op + 1 + len_name + 1 + len_digits + 3;

			buff = (char *)calloc(size + len, sizeof(char));
			check_calloc(buff, NULL);
			offset = sprintf(buff, "%s %s %d \n ", ops[op], name, len);
			memcpy(buff + offset, data, len);
			break;

		case message_data:				// "DATA len \n data"
			if (len < 0)	invalid_operation(NULL);
			if (len == 0)	len_digits = 1;
			if (len > 0)	len_digits = (int)(log10(len) + 1);
			len_op = strlen(ops[op]);
			size = len_op + 1 + len_digits + 3;

			buff = (char *)calloc(size + len, sizeof(char));
			check_calloc(buff, NULL);
			offset = sprintf(buff, "%s %d \n ", ops[op], len);
			memcpy(buff + offset, data, len);
			break;

		case message_leave:				// "LEAVE \n"
		case message_ok:				// "OK \n"
			len_op = strlen(ops[op]);
			size = len_op + 2;

			buff = (char *)calloc(size, sizeof(char));
			check_calloc(buff, NULL);
			sprintf(buff, "%s \n", ops[op]);
			break;

		case message_ko:				// "KO message \n"
			len_op = strlen(ops[op]);
			size = len_op + 1 + len + 2;

			buff = (char *)calloc(size, sizeof(char));
			check_calloc(buff, NULL);
			sprintf(buff, "%s %s \n", ops[op], data);
			break;

		case message_err:
		default:
			invalid_operation(NULL);
			break;
	}
	m->buff = buff;

	write_buffer(sock, buff, len);
}

// TODO: destroy con doppio puntatore?
void message_destroy(message *m)
{
	free(m->buff);
	free(m);
}
