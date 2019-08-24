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

volatile int _is_exit = FALSE; //TODO: usare volatile sig_atomic_t?

int _running_threads;
pthread_mutex_t _running_threads_mux;
list *_users = NULL;


void handler(int sig)
{
	if (sig == SIGUSR1) {
		_print_stats = TRUE;
	}

	if (sig == SIGINT) {
		_is_exit = TRUE;
	}
}

void set_sigaction()
{
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sigemptyset(&sa.sa_mask);
	sigaddset(&sa.sa_mask, SIGUSR1);
	sigaddset(&sa.sa_mask, SIGINT);
	sa.sa_handler = handler;

	if (sigaction(SIGUSR1, &sa, NULL) == -1) {
		perror("Signal error SIGUSR1");
		exit(EXIT_FAILURE);
	}

	if (sigaction(SIGINT, &sa, NULL) == -1) {
		perror("Signal error SIGINT");
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

int handle_register(message *m, user **client)
{
	char *name = m->name;

	node *n = list_search(_users, name, user_compare_name);
	*client = (n != NULL ? n->info : NULL);
	if (*client == NULL) {
		char buff[MAX_BUFF];

		sprintf(buff, "%s/%s", PATH_DATA, name);
		makeDirectory(buff);
		*client = user_create(name);
		list_result res = list_insert(_users, *client);

		if (res != list_success) return FALSE;
	}
	return TRUE;
}

// int handle_store(message *m, user **client)
// {
// 	char *dataname = m->name;
// 	size_t len = m->len;
// 	char *data = m->data;

// 	//TODO: creazione del file che conterrà data!

// 	object *obj = object_create(dataname, len);
// 	// TODO: Non funge l'inserzione dei dati
// 	//list_result res = list_insert_unsafe((*client)->objects, obj);

// 	if (res != list_success) return FALSE;
// 	printf("Object added successfully!\n");
// 	stats_server_incr_obj();
// 	stats_server_incr_size(len);
// 	return TRUE;
// }

void *handle_client(void *arg)
{
	int fd_c = (int)arg;
	int done = FALSE;
	client_stats c_stats = stats_client_create();

	while (!done && !_is_exit) {
		message *received = message_receive(fd_c);
		message *sent = NULL;

		if (_is_exit == TRUE) {
			message_destroy(received);
			//mando la ko?
			stats_client_print(c_stats);
			stats_server_decr_client();
			close(fd_c);
			exit(EXIT_FAILURE); // TODO: gestire la chiusura di tutti i thread
		}

		message_op op = received->op;
		int result = FALSE;
		user *client = NULL;

		switch(op) {

			case message_register:		// REGISTER name \n
				result = handle_register(received, &client);
				c_stats.total_ops++;

				if (result == TRUE) {
					stats_server_incr_client();
					c_stats.success_ops++;
					sent = message_create(message_ok, NULL, 0, NULL);
					printf("Connect tutto ok!!\n");
				} else {
					c_stats.fail_ops++;
					sent = message_create(message_ko, NULL, 0, "ERROR: Register failed");
					done = TRUE;
				}
				message_send(fd_c, sent);

				message_destroy(received);
				message_destroy(sent);
				break;

			case message_store:			// STORE name len \n data
				printf("Server got: Message_store\n");
				//result = handle_store(received, &client);
				c_stats.total_ops++;

				if (result == TRUE) {
					c_stats.success_ops++;
					sent = message_create(message_ok, NULL, 0, NULL);
					printf("Store tutto ok!!\n");
				} else {
					c_stats.fail_ops++;
					sent = message_create(message_ko, NULL, 0, "ERROR: Store failed");
					done = TRUE;
				}
				message_send(fd_c, sent);

				message_destroy(received);
				message_destroy(sent);

				break;
			case message_retrieve:		// RETRIEVE name \n

				break;
			case message_delete:		// DELETE name \n

				break;
			case message_leave:			// LEAVE \n


				break;
			case message_err:
			default:
				invalid_operation(NULL);
				break;
		}
	}
	// TODO definire la funzione decr_threads() protetta da lock
	stats_server_decr_client();
	stats_client_print(c_stats);
	close(fd_c);
	return NULL;
}


int main(int argc, char *argv[])
{
	unlink(SOCKNAME);
	stats_server_create();
	set_sigaction();

	_print_stats = FALSE;
	_running_threads = 1;
	pthread_mutex_init(&_running_threads_mux, NULL);

	int err = 0;
	int fd_skt = 0;
	int fd_c = 0;
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
		if (fd_c == -1) {
			exit(EXIT_FAILURE); //TODO handle this case
		}
		pthread_t worker;
		err = pthread_create(&worker, NULL, &handle_client, (int *)fd_c);
		if (err != 0) {
			exit(EXIT_FAILURE);
		}
		// pthread_detach(worker); TODO: verificare se utile
		// TODO definire la funzione incr_threads() protetta da lock

		if (_print_stats == TRUE) {
			stats_server_print();
		}
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
