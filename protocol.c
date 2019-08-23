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
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "message.h"
#include "common.h"

int _fd_skt;

static int sendAndReceive(message *sent)
{
	int result = FALSE;
	message *received = NULL;

	message_send(_fd_skt, sent);
	received = message_receive(_fd_skt);

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
	struct sockaddr_un sa;
	memset(&sa, 0, sizeof(sa));

	strncpy(sa.sun_path, SOCKNAME, sizeof(sa.sun_path));
	sa.sun_family = AF_UNIX;
	_fd_skt = socket(AF_UNIX, SOCK_STREAM, 0);

	while (connect(_fd_skt, (struct sockaddr *)&sa, sizeof(sa)) == -1) {
		if (errno == ENOENT)
			sleep(1);
		else exit(EXIT_FAILURE);
	}

	sent = message_create(message_register, name, 0, NULL);
	result = sendAndReceive(sent);

	return result;
}

int os_store(char *name, void *block, size_t len)
{
	int result = FALSE;
	message *sent = NULL;

	sent = message_create(message_store, name, len, block);
	result = sendAndReceive(sent);

	return result;
}

void *os_retrieve(char *name)
{
	void *data = NULL;
	message *sent = NULL;
	message *received = NULL;

	sent = message_create(message_retrieve, name, 0, NULL);
	message_send(_fd_skt, sent);
	received = message_receive(_fd_skt);

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
	result = sendAndReceive(sent);

	return result;
}

int os_disconnect()
{
	int result = FALSE;
	message *sent = NULL;

	sent = message_create(message_leave, NULL, 0, NULL);
	result = sendAndReceive(sent);
	close(_fd_skt);

	return result;
}