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
	char *buff;		// puntatore al buffer su cui fare la free
	message_op op;	// enum per il tipo di operazione
	char *name;		// puntatore al name
	size_t len;		// strtol(puntatore a len)
	char *data;		// data
} message;

// Tipi di messaggi

// register, retrieve, delete -->OP nome \n
// ko -------------------------->OP message \n
// store ----------------------->OP name len \n data
// data ------------------------>OP len \n data
// leave, ok ------------------->OP \n

// Alloca spazio per una struttura di tipo message
// Assegna i valori passati ai campi della struttura
// inizializza buff
// copia OP
// copia puntatore a name
// copia len
// copia il riferimento a data
message *message_create(message_op op,
						char *name,
						size_t len,
						void *data);

// Tokenizza la stringa letta da sock per creare un messaggio
// Al primo token (l'operazione) confronta con le possibili op da eseguire
// se OP == register, retrieve o delete dovrò inserire solo il nome
// se OP == ko mi serve solo message che salverò in data
// se OP == store mi servono i campi name, len e data
// se OP == data mi servono len e data
// se OP == leave o ok non mi serve altro
message *message_receive(int sock);


// Crea, in base all'operazione, una stringa con i valori
// contenuti in msg
// e poi la manda alla sock con una write
void message_send(int sock, message *m);

// Libera la memoria per
// name
// struttura
void message_destroy(message *m);

#endif
