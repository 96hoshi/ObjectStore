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
} message_OP;

typedef struct {
	char *buff;		// puntatore al buffer su cui fare la free
	message_OP OP;	// enum per il tipo di operazione
	char *name;		// puntatore al name
	int len;		// strtol(puntatore a len)
	char *data;		// TODO: controlla se posso salvare i dati direttamente
					// 		 con un void *
} message;

// Tipi di messaggi

// register, retrieve, delete -->OP nome \n
// ko -------------------------->OP message \n
// store ----------------------->OP name len \n data
// data ------------------------>OP len \n data
// leave, ok ------------------->OP \n


// Alloca spazio per una struttura di tipo message
// Assegna i valori passati ai campi della struttura
// copia il puntatore al buff
// copia OP
// copia puntatore a name
// copia len
// copia il riferimento a data
message *message_create(message_OP OP, char *name, int len, void *data);

// Tokenizza la stringa per creare un messaggio
// Al primo token (l'operazione) confronta con le possibili op da eseguire
// se OP == register, retrieve o delete dovrò inserire solo il nome
// se OP == ko mi serve solo message che salverò in data
// se OP == store mi servono i campi name, len e data
// se OP == data mi servono len e data
// se OP == leave o ok non mi serve altro
message *string_to_message(char *header);


// Crea, in base all'operazione, una stringa con i valori
// contenuti in msg
// utilizzerò il reverse delle funzioni usate da string_to_message
char* message_to_string(message *m);

// Libera la memoria per
// name
// struttura
void message_destroy(message *m);

#endif
