#include "protocol.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <message.h>
#include <common.h>


static long _fd_skt = -1;


static int sendAndReceive(message *sent)
{
	if (message_send(_fd_skt, sent) == FALSE) {
		return FALSE;
	}

	message *received = message_receive(_fd_skt);
	if (received == NULL) {
		return FALSE;
	}

	int result = FALSE;

	if (received->op == message_ok) {
		result = TRUE;
	}
	if (received->op == message_ko) {
		fprintf(stderr,"%s\n", received->name);
		result = FALSE;
	}

	if (sent->op == message_store) {
		message_extract_data(sent); // avoid to release not owned data
	}

	message_destroy(sent);
	message_destroy(received);

	return result;
}

int os_connect(char *name)
{
	if (_fd_skt < 0) {
		struct sockaddr_un sa;
		memset(&sa, 0, sizeof(sa));

		strncpy(sa.sun_path, SOCKNAME, sizeof(sa.sun_path));
		sa.sun_family = AF_UNIX;
		if ((_fd_skt = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
			return FALSE;
		}

		if (connect(_fd_skt, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
			return FALSE;
		}
	}

	message *sent = message_create(message_register, name, 0, NULL);
	return sendAndReceive(sent);
}

int os_store(char *name, void *block, size_t len)
{
	if (_fd_skt < 0) return FALSE;
	message *sent = message_create(message_store, name, len, block);
	return sendAndReceive(sent);
}

void *os_retrieve(char *name)
{
	if (name == NULL) {
		return NULL;
	}

	if (_fd_skt < 0) {
		return NULL;
	}

	void *data = NULL;

	message *sent = message_create(message_retrieve, name, 0, NULL);
	if (message_send(_fd_skt, sent) == FALSE) {
		message_destroy(sent);
		return NULL;
	}

	message *received = message_receive(_fd_skt);
	if (received == NULL) {
		message_destroy(sent);
		return NULL;
	}

	if (received->op == message_data) {
		data = message_extract_data(received);
	}

	if (received->op == message_ko) {
		fprintf(stderr,"%s\n", received->name);
	}

	message_destroy(sent);
	message_destroy(received);

	return data;
}

int os_delete(char *name)
{
	if (_fd_skt < 0) {
		return FALSE;
	}
	message *sent = message_create(message_delete, name, 0, NULL);
	return sendAndReceive(sent);
}

int os_disconnect()
{
	if (_fd_skt < 0) {
		return FALSE;
	}

	message *sent = message_create(message_leave, NULL, 0, NULL);
	int result = sendAndReceive(sent);
	close(_fd_skt);

	return result;
}
