#define _POSIX_C_SOURCE 199506L // needed by pthread sigmask

#include "worker.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include "common.h"
#include "user.h"
#include "object.h"
#include "list.h"
#include "message.h"
#include "stats.h"


extern volatile sig_atomic_t _is_exit;
extern list *_users;
extern pthread_mutex_t _workers_mux;
extern pthread_cond_t _workers_cond;
extern size_t _workers;


static void workersIncrement()
{
	pthread_mutex_lock(&_workers_mux);
	_workers++;
	pthread_mutex_unlock(&_workers_mux);
}

static void workersDecrement()
{
	pthread_mutex_lock(&_workers_mux);
	_workers--;
	if(_is_exit) {
		pthread_cond_signal(&_workers_cond);
	}
	pthread_mutex_unlock(&_workers_mux);
}

static int fileStore(void *data, char *dataname, size_t len, char *clientname)
{
	FILE *data_file = NULL;
	char path[MAX_BUFF];
	sprintf(path, "%s/%s/%s", PATH_DATA, clientname, dataname);

	// file already exists
	if (access(path, F_OK) != -1) {
		fprintf(stderr, "%s: File already exists\n", path);
		return FALSE;
	}
	data_file = fopen(path, "w");
	if(data_file == NULL) {
		return FALSE;
	}

	int result = fwrite(data, sizeof(char), len, data_file);
	fclose(data_file);
	return result == len;
}

static void *fileRetrive(char *dataname, size_t len, char *clientname)
{
	FILE *data_file = NULL;
	char *data = (char *)calloc(len, sizeof(char));
	if (data == NULL) {
		return NULL;
	}

	char path[MAX_BUFF];
	sprintf(path, "%s/%s/%s", PATH_DATA, clientname, dataname);

	// file doesn't exist
	if (access( path, F_OK ) == -1) {
		free(data);
		return NULL;
	}

	data_file = fopen(path, "r");
	if(data_file == NULL) {
		free(data);
		return NULL;
	}

	int result = fread(data, sizeof(char), len, data_file);
	fclose(data_file);

	if (result != len) {
		free(data);
		return NULL;
	}
	return (void *)data;
}

static int fileDelete(char *dataname, char *clientname)
{
	char path[MAX_BUFF];
	sprintf(path, "%s/%s/%s", PATH_DATA, clientname, dataname);

	// file doesn't exist
	if (access( path, F_OK ) == -1) {
		return FALSE;
	}

	if (remove(path) != 0) {
		return FALSE;
	}
	return TRUE;
}


static int handle_register(message *m, user **client)
{
	char *name = m->name;

	*client = (user *)list_search(_users, name);

	if (*client == NULL) {
		char path[MAX_BUFF];

		sprintf(path, "%s/%s", PATH_DATA, name);
		
		struct stat st = {0};
		if (stat(path, &st) == -1) {
			mkdir(path, 0700);
		}

		*client = user_create(name);
		list_result res = list_insert(_users, *client);
		if (res != list_success) {
			return FALSE;
		}
	}
	return TRUE;
}

static int handle_store(message *m, user *client)
{
	if (client == NULL) return FALSE;

	char *dataname = m->name;
	size_t len = m->len;
	char *data = m->data;
	char *name = client->name;

	if (fileStore(data, dataname, len, name) == FALSE) {
		return FALSE;
	}

	list_result res = user_insert_object(client, dataname, len);
	if (res != list_success) {
		return FALSE;
	}

	stats_server_incr_obj();
	stats_server_incr_size(len);

	return TRUE;
}

static void *handle_retrieve(message *m, user *client, size_t *len)
{
	if (client == NULL) {
		return NULL;
	}

	char *dataname = m->name;
	char *name = client->name;

	object *obj = user_search_object(client, dataname);
	if (obj == NULL) {
		return NULL;
	}
	*len = obj->len;

	return fileRetrive(dataname, *len, name);
}

static int handle_delete(message *m, user *client)
{
	if (client == NULL) {
		return FALSE;
	}

	char *dataname = m->name;
	char *name = client->name;

	object *obj = user_delete_object(client, dataname);
	if (obj == NULL) {
		return FALSE;
	}

	if (fileDelete(dataname, name) == FALSE) {
		return FALSE;
	}

	size_t len = obj->len;
	object_destroy(obj);

	stats_server_decr_obj();
	stats_server_decr_size(len);

	return TRUE;
}

static void *handle_client(void *arg)
{
	workersIncrement();

	long fd_c = (long)arg;
	user *client = NULL;
	int done = FALSE;

	while (!done && !_is_exit) {
		message *received = message_receive(fd_c);
		message *sent = NULL;

		if (received == NULL) {
			done = TRUE;
			continue;
		}

		message_op op = received->op;
		void *data = NULL;
		size_t len = 0;

		switch(op) {

			case message_register:						// REGISTER name \n

				if (handle_register(received, &client) == TRUE) {
					stats_server_incr_client();
					sent = message_create(message_ok, NULL, 0, NULL);
				} else {
					sent = message_create(message_ko, "ERROR: Register failed", 0, NULL);
					done = TRUE;
				}
				break;

			case message_store:							// STORE name len \n data

				if (handle_store(received, client) == TRUE) {
					sent = message_create(message_ok, NULL, 0, NULL);
				} else {
					sent = message_create(message_ko, "ERROR: Store failed", 0, NULL);
				}
				break;

			case message_retrieve:						// RETRIEVE name \n
				data = handle_retrieve(received, client, &len);

				if (data != NULL) {
					sent = message_create(message_data, NULL, len, data);
				} else {
					sent = message_create(message_ko, "ERROR: Retrieve failed", 0, NULL);
				}
				break;

			case message_delete:						// DELETE name \n

				if (handle_delete(received, client) == TRUE) {
					sent = message_create(message_ok, NULL, 0, NULL);
				} else {
					sent = message_create(message_ko, "ERROR: Delete failed", 0, NULL);
				}
				break;

			case message_leave:							// LEAVE \n
				sent = message_create(message_ok, NULL, 0, NULL);
				done = TRUE;
				break;

			case message_err:
			default:
				exit(EXIT_FAILURE);
				break;
		}

		if (message_send(fd_c, sent) == FALSE) {
			done = TRUE;
		}

		message_destroy(received);
		message_destroy(sent);
	}

	stats_server_decr_client();
	close(fd_c);
	workersDecrement();

	return NULL;
}


void worker_spawn(long fd_c)
{
	sigset_t thread_set;
	sigemptyset(&thread_set);
	pthread_sigmask(SIG_BLOCK, &thread_set, NULL);

	pthread_attr_t thread_attributes;

	if (pthread_attr_init(&thread_attributes) != 0) {
		fprintf(stderr, "pthread_attr_init FAILED\n");
		close(fd_c);
		return;
	}

	if (pthread_attr_setdetachstate(&thread_attributes, PTHREAD_CREATE_DETACHED) != 0) {
		fprintf(stderr, "pthread_attr_setdetachstate FALLITA\n");
		pthread_attr_destroy(&thread_attributes);
		close(fd_c);
		return;
	}

	pthread_t worker;
	pthread_create(&worker, &thread_attributes, &handle_client, (void *)fd_c);
}
