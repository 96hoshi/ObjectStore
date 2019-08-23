// Contiene le funzioni che utilizzerà il client per comunicare col server
// Responsabile di ricevere, tradurre e mandare messaggi
// La traduzione e l'invio avviene mediante le funzioni di libreria message.h

// 	-univoco nella cartella ----> controllo lato server

// Se questi controlli sono verificati manda al server una socket con l'header
// creato da message.h
// Riceve poi l'esito da parte del server e manda la risposta al client

#include "protocol.h"
#include <stdio.h>
#include <stdlib.h>
#include "message.h"


static int sendAndRecieve(message *sent)
{
	int result = FALSE;
	message *received = NULL;

	message_send(_client_skt, sent);
	received = message_receive(_client_skt);

	if (received->op == message_ok) {
		result = TRUE;
	}

	message_destroy(sent);
	message_destroy(received);

	return result;
}

int os_connect(char *name)
{
	int result = FALSE;
	message *sent = NULL;

	sent = message_create(message_register, name, 0, NULL);
	result = sendAndRecieve(sent);

	return result;
}

int os_store(char *name, void *block, size_t len)
{
	int result = FALSE;
	message *sent = NULL;

	sent = message_create(message_store, name, len, block);
	result = sendAndRecieve(sent);

	return result;
}

void *os_retrieve(char *name)
{
	void *data = NULL;
	message *sent = NULL;
	message *received = NULL;

	sent = message_create(message_retrieve, name, 0, NULL);
	message_send(_client_skt, sent);
	received = message_receive(_client_skt);

	if (received->op == message_data) {
		data = (void *)received->data;
	}

	message_destroy(sent);
	message_destroy(received);

	return data;
}

int os_delete(char *name)
{
	int result = FALSE;
	message *sent = NULL;

	sent = message_create(message_delete, name, 0, NULL);
	result = sendAndRecieve(sent);

	return result;
}

int os_disconnect()
{
	int result = FALSE;
	message *sent = NULL;

	sent = message_create(message_leave, NULL, 0, NULL);
	result = sendAndRecieve(sent);

	return result;
}