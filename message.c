// Traduce i messaggi tra client (request) e server (response) in strutture "leggibili".

// Messaggio = "header" ,codificato in ASCII, è una stringa di testo terminata da "\n"
// a cui può succedere un blocco di dati.

// Cosa posso trovare nei messaggi:
// 	OP 	= array di caratteri che indica l'operazione da eseguire
// 	name 	= array di caratteri che contiene il nome del client o del dato
// 	len 	= lunghezza in byte del dato, codificato in ASCII
// 	data 	= blocco binario i lunghezza len, posto dopo lo "\n"
// 	message = array di caratteri che contiene in messaggio di errore del server

// Struttura dati:
// -msg:
// 	-int OP
// 	-char* name
// 	-int len
// 	-void* data (che conterrà anche message in caso)

#define _POSIX_C_SOURCE 200112L // per strtok_r in c99

#include "message.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Creare un array di tringhe static (locale al modulo)
// per tenere traccia delle operazioni dei messaggi:
// A ogni indice dell'array corrisponde l'operazione dell'enum nell'header
// ops[msg->message_OP] per lo switch case
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

message_OP check_op(char *string)
{
	for(message_OP i = message_register; i <= message_ko; ++i) {
		if (strcmp(ops[i], string) == 0) {
			return i;
		}
	}
	return message_err;
}

message *message_create(message_OP OP, char *name, int len, void *data)
{
	message *m = (message *)calloc(1, sizeof(message));
	m->buff = NULL;
	m->OP = OP;
	m->name = name;
	m->len = len;
	m->data = data;

	return m;

}

message *string_to_message(char *header)
{
	char *save;
	char *token_op = strtok_r(header, " \n", &save);

	message *m = (message *)calloc(1, sizeof(message));

	m->buff = header;
	m->OP = check_op(token_op);

	switch(m->OP) {
		// register, retrieve, delete -->OP nome \n
		case message_register:
		case message_retrieve:
		case message_delete: {
			m->name = strtok_r(save, " ", &save);
			if (save[0] != '\n') {
				;//TODO: errore header non corretto
			}
			
			break;
		}
		// store-->OP name len \n data
		case message_store: {
			m->name = strtok_r(save, " ", &save);
			char *string_len = strtok_r(save, "\n", &save);
			m->len = strtol(string_len, NULL, 10);
			m->data = (void *)(save + 1);
			break;
		}
		// data-->OP len \n data
		case message_data: {
			char *string_len = strtok_r(save, "\n", &save);
			m->len = strtol(string_len, NULL, 10);
			m->data = (void *)(save + 1);
			break;
		}
		// leave, ok-->OP \n
		case message_leave:
		case message_ok:
			if (save[0] != '\n') {
				;//TODO: errore header non corretto
			}
			break;
		// ko-->OP message \n
		case message_ko: {
			m->data = (void *)strtok_r(save, "\n", &save);
			break;
		}

		case message_err:
		default: {
			break;
		}
	}
	return m;

}

// Crea, in base all'operazione, una stringa con i valori
// contenuti in msg
char *message_to_string(message *m)
{
	char *buff;
	
	switch (m->OP) {
		// register, retrieve, delete-->OP nome \n
		case message_register:
		case message_retrieve:
		case message_delete: {
			// 4 = 2 spazi, '\n' e '\0'
			int dim = strlen(ops[m->OP]) + strlen(m->name) + 4;
			buff = (char *)calloc(dim, sizeof(char));
			snprintf(buff, dim, "%s %s \n", ops[m->OP], m->name);
			//buff[dim] = '\0';
			break;
		}
		// store-->OP name len \n data
		case message_store: {
			// 5 = 4 spazi e \n
			int len_digits = 2;
			// TODO: controllare come calcolare digits e se funziona
			int dim = strlen(ops[m->OP]) + strlen(m->name) + len_digits + 5;

			buff = (char *)calloc(dim + m->len, sizeof(char));
			snprintf(buff, dim, "%s %s %d \n ", ops[m->OP], m->name, m->len);
			memcpy(buff + dim, m->data, m->len);

			break;
		}

		// data-->OP len \n data
		case message_data: {
			// 5 = 4 spazi e \n
			int len_digits = 2;
			// TODO: controllare come calcolare digits e se funziona
			int dim = strlen(ops[m->OP]) + len_digits + 5;

			buff = (char *)calloc(dim + m->len, sizeof(char));
			snprintf(buff, dim, "%s %d \n ", ops[m->OP], m->len);
			memcpy(buff + dim, m->data, m->len);
			break;
		}
		// leave, ok-->OP \n
		case message_leave:
		case message_ok: {
			// 3 = uno spazio, '\n' e '\0'
			int dim = strlen(ops[m->OP]) + 3;
			buff = (char *)calloc(dim, sizeof(char));
			snprintf(buff, dim, "%s \n", ops[m->OP]);
			break;
		}
		// ko-->OP message \n
		case message_ko: {
			//
			int dim = strlen(ops[m->OP]) + strlen(m->data) + 3;
			buff = (char *)calloc(dim, sizeof(char));
			snprintf(buff, dim, "%s %s \n", ops[m->OP], (char *)m->data);
			// TODO: controllare l'ultima riga
			break;
		}
		case message_err:
		default: {
			break;
		}
	}
	m->buff = buff;

	return buff;

}

void message_destroy(message *m)
{
	free(m->buff);
	free(m);
}