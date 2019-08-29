// "Server", riceve richieste dai client

// Ha il compito di creare per ogni client una cartella, col nome del client,
// che conterrà gli "objects" dell'utente.
// Gli objects sono, in questo caso, array di caratteri che verranno
// memorizzati in appositi file (con lo stesso nome dell'object).
// Tutte le cartelle degli utenti sono localizzate nella cartella data,
// creata dal server.
// Tutti gli object e i client hanno nomi che rispettano lo standard POSIX
// e sono univoci all'interno dello stesso spazio di memorizzazione

// L'objStore attende il collegamento su objstore.sock.
// Crea  un thread per le richieste di ogni client.
// Quando riceve un messsaggio del client viene tradotto grazie alla lib message.h
// Che verrà utilizzata anche per inviare la risposta.
// Risponde sempre!!
// Quando riceve un segnale risponde il prima possibile mantenendo
// l'objstore in uno stato consistente.

// Quando il server riceve il segnare SIGUSR1 stampa su stout info
// sullo stato del server, quali:
// 	-#client connessi
// 	-#oggetti nello store
// 	-size totale dello store
// 	-...
// La libreria stats.h si occuperà di stampare tali statistiche.
// A differenza delle stats del client, per le stats del server servirà
// una struttura condivisa tra i thread e gestita in maniera threadsafe
// con apposite lock

// Necessito di una libreria per la gestione dei file
// fileops

#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>
#include <errno.h>
#include "user.h"
#include "list.h"
#include "message.h"
#include "stats.h"
#include "common.h"

#define PATH_DATA "./data"

volatile sig_atomic_t _is_exit = FALSE;

int _running_threads = 1;
pthread_mutex_t _running_threads_mux;
list *_users = NULL;


void incr_threads()
{
	pthread_mutex_lock(&_running_threads_mux);
	_running_threads++;
	pthread_mutex_unlock(&_running_threads_mux);
}

void decr_threads()
{
	pthread_mutex_lock(&_running_threads_mux);
	_running_threads--;
	pthread_mutex_unlock(&_running_threads_mux);
}

void handler(int sig)
{
	switch(sig){

		case SIGUSR1:
			_print_stats = TRUE;
			break;

		case SIGINT:
		case SIGTERM:
		case SIGQUIT:
			_is_exit = TRUE;
			break;

		default:
			break;
	}
}

void set_sigaction()
{
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sigemptyset(&sa.sa_mask);
	sigaddset(&sa.sa_mask, SIGUSR1);
	sigaddset(&sa.sa_mask, SIGINT);
	sigaddset(&sa.sa_mask, SIGTERM);
	sigaddset(&sa.sa_mask, SIGQUIT);
	sa.sa_handler = handler;

	if (sigaction(SIGUSR1, &sa, NULL) == -1) {
		perror("Signal error SIGUSR1");
		exit(EXIT_FAILURE);
	}
	if (sigaction(SIGINT, &sa, NULL) == -1) {
		perror("Signal error SIGINT");
		exit(EXIT_FAILURE);
	}
	if (sigaction(SIGTERM, &sa, NULL) == -1) {
		perror("Signal error SIGTERM");
		exit(EXIT_FAILURE);
	}
	if (sigaction(SIGQUIT, &sa, NULL) == -1) {
		perror("Signal error SIGQUIT");
		exit(EXIT_FAILURE);
	}
}

void makeDirectory(char *path)
{
	struct stat st = {0};

	if (stat(path, &st) == -1) {
		mkdir(path, 0700);
	}
}

int storeFile(void *data, char *dataname, size_t len, char *clientname)
{
	FILE *data_file = NULL;
	char path[MAX_BUFF];
	sprintf(path, "%s/%s/%s", PATH_DATA, clientname, dataname);

	// file already exists
	// TODO: to remove, used for testings
	if (access(path, F_OK) != -1) {
		fprintf(stderr, "%s: File already exists\n", path);
		return FALSE;
	}
	data_file = fopen(path, "w");
	if(data_file == NULL) return FALSE;

	fwrite(data, sizeof(char), len, data_file);
	fclose(data_file);
	return TRUE;
}

void *retrieveFile(char *dataname, size_t len, char *clientname)
{
	FILE *data_file = NULL;
	char *data = (char *)calloc(len, sizeof(char));
	check_calloc(data, NULL);
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

	fread(data, sizeof(char), len, data_file);
	fclose(data_file);
	return (void *)data;
}

int deleteFile(char *dataname, char *clientname)
{
	char path[MAX_BUFF];
	sprintf(path, "%s/%s/%s", PATH_DATA, clientname, dataname);

	// file doesn't exist
	if (access( path, F_OK ) == -1) return FALSE;

	if (remove(path) == 0) {
		return TRUE;
	} else {
		return FALSE;
	}
}

int handle_register(message *m, user **client)
{
	char *name = m->name;

	*client = (user *)list_search(_users, name, user_compare_name);

	if (*client == NULL) {
		char path[MAX_BUFF];

		sprintf(path, "%s/%s", PATH_DATA, name);
		makeDirectory(path);
		*client = user_create(name);
		list_result res = list_insert(_users, *client);
		if (res != list_success) return FALSE;
	}
	return TRUE;
}

int handle_store(message *m, user *client)
{
	if (client == NULL) return FALSE;

	char *dataname = m->name;
	size_t len = m->len;
	char *data = m->data;
	char *name = client->name;

	if (storeFile(data, dataname, len, name) == FALSE) return FALSE;

	list_result res = user_insert_object(client, dataname, len);
	if (res != list_success) return FALSE;

	stats_server_incr_obj();
	stats_server_incr_size(len);
	return TRUE;
}

void *handle_retrieve(message *m, user *client, size_t *len)
{
	if (client == NULL) return NULL;

	char *dataname = m->name;
	char *name = client->name;

	object *obj = user_search_object(client, dataname);
	if (obj == NULL) return NULL;
	*len = obj->len;

	return retrieveFile(dataname, *len, name);
}

int handle_delete(message *m, user *client)
{
	if (client == NULL) return FALSE;

	char *dataname = m->name;
	char *name = client->name;

	object *obj = user_search_object(client, dataname);
	if (obj == NULL) return FALSE;
	size_t len = obj->len;

	list_result res = user_delete_object(client, obj);
	if (res != list_success) return FALSE;

	if (deleteFile(dataname, name) == FALSE) return FALSE;

	stats_server_decr_obj();
	stats_server_decr_size(len);
	return TRUE;
}

void *handle_client(void *arg)
{
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
				if (message_send(fd_c, sent) == FALSE) {
					done = TRUE;
				}
				break;

			case message_store:							// STORE name len \n data

				if (handle_store(received, client) == TRUE) {
					sent = message_create(message_ok, NULL, 0, NULL);
				} else {
					sent = message_create(message_ko, "ERROR: Store failed", 0, NULL);
				}
				if (message_send(fd_c, sent) == FALSE) {
					done = TRUE;
				}
				break;

			case message_retrieve:						// RETRIEVE name \n
				data = handle_retrieve(received, client, &len);

				if (data != NULL) {
					sent = message_create(message_data, NULL, len, data); //TODO: memory leak per data
				} else {
					sent = message_create(message_ko, "ERROR: Retrieve failed", 0, NULL);
				}
				if (message_send(fd_c, sent) == FALSE) {
					done = TRUE;
				}
				break;

			case message_delete:						// DELETE name \n

				if (handle_delete(received, client) == TRUE) {
					sent = message_create(message_ok, NULL, 0, NULL);
				} else {
					sent = message_create(message_ko, "ERROR: Delete failed", 0, NULL);
				}
				if (message_send(fd_c, sent) == FALSE) {
					done = TRUE;
				}
				break;

			case message_leave:							// LEAVE \n
				sent = message_create(message_ok, NULL, 0, NULL);
				message_send(fd_c, sent);
				done = TRUE;
				break;

			case message_err:
			default:
				invalid_operation(NULL);
				break;
		}
		message_destroy(received);
		message_destroy(sent);
	}
	decr_threads();
	stats_server_decr_client();
	close(fd_c);
	return NULL;
}

void spawn_thread(long fd_c){

	sigset_t thread_set;
	sigemptyset(&thread_set);

	pthread_sigmask(SIG_BLOCK, &thread_set, NULL);

	pthread_t worker;
	pthread_create(&worker, NULL, &handle_client, (long *)fd_c);
	pthread_detach(worker);
}


int main(int argc, char *argv[])
{
	unlink(SOCKNAME);
	set_sigaction();
	stats_server_init();
	pthread_mutex_init(&_running_threads_mux, NULL);

	long fd_skt = 0;
	long fd_c = 0;
	struct sockaddr_un sa;
	memset(&sa, 0, sizeof(sa));

	// Creating data
	makeDirectory(PATH_DATA);
	// Creating connected users' list
	_users = list_create(user_compare,
						 user_compare_name,
						 user_destroy,
						 user_print);

	strncpy(sa.sun_path, SOCKNAME, sizeof(sa.sun_path));
	sa.sun_family = AF_UNIX;

	fd_skt = socket(AF_UNIX, SOCK_STREAM, 0);
	bind(fd_skt, (struct sockaddr *)&sa, sizeof(sa));
	listen(fd_skt, SOMAXCONN);

	while (!_is_exit) {
		fd_c = accept(fd_skt, NULL, 0);
		if (fd_c < 0) {
			if ((errno == EINTR) && (_print_stats == TRUE)) {
				stats_server_print();
				_print_stats = FALSE;
				continue;
			}
			break; //TODO is_exit = TRUE?
		}
		spawn_thread(fd_c);
		incr_threads();
	}

	//TODO: usare pthread_cond_t
	// if ) {
	// 	close(fd_skt);
	// 	exit(EXIT_SUCCESS);
	// }

	list_destroy(_users);
	pthread_mutex_destroy(&_running_threads_mux);
	stats_server_destroy();
	close(fd_skt);
	return 0;
}
