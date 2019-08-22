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
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>
#include <errno.h>
#include "common.h"
#include "message.h"
#include "stats.h"

volatile int _is_exit;

int _running_threads;
pthread_mutex_t _running_threads_mux;

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


// TODO: ogni thread gestisce tutte le richieste del client
// utilizzare un while finchè non ottiene il comando disconnect
void *handle_client(void *arg)
{
	int fd_c = (int)arg;
	message *m = message_receive(fd_c);

	if (_is_exit == TRUE) {
		exit(EXIT_FAILURE); // TODO: gestire la chiusura di tutti i thread
	}
	message_send(fd_c, m);

	// TODO definire la funzione decr_threads() protetta da lock
	close(fd_c);
	return NULL;
}


int main(int argc, char *argv[])
{
	unlink(SOCKNAME);
	stats_server_create();
	set_sigaction();

	_print_stats = FALSE;
	_is_exit = FALSE;
	_running_threads = 1;
	pthread_mutex_init(&_running_threads_mux, NULL);

	int fd_skt = 0;
	int fd_c = 0;
	struct sockaddr_un sa;
	int err = 0;

	strncpy(sa.sun_path, SOCKNAME, sizeof(sa.sun_path));
	sa.sun_family = AF_UNIX;

	fd_skt = socket(AF_UNIX, SOCK_STREAM, 0);
	bind(fd_skt, (struct sockaddr *)&sa, sizeof(sa));
	listen(fd_skt, SOMAXCONN);

	while (!_is_exit) {
		fd_c = accept(fd_skt, NULL, 0);
		pthread_t worker;
		err = pthread_create(&worker, NULL, &handle_client, (int *)fd_c);
		if (err != 0) {
			exit(EXIT_FAILURE);
		}
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
	pthread_mutex_destroy(&_running_threads_mux);
	stats_server_destroy();
	close(fd_skt);

	return 0;
}







// char *read_from_client(int fd_c)
// {
// 	int fd_c = (int)arg;
// 	ssize_t n = 0;
// 	int max_len = MAX_BUFF;
// 	int len = 0;
// 	char *buff = (char*)calloc(MAX_BUFF, sizeof(char));
// 	check_calloc(buff, NULL);


// 	while ((n = read(fd_c, buff + len, max_len)) != 0) {
// 		if (n < 0 && errno == EINTR) {
// 			if (_print_stats == TRUE) {
// 				continue;
// 			}
// 			break;
// 		}
// 		if (n == max_len) {
// 			len += n;
// 			max_len *= 2;
// 			char *newbuff = (char*)realloc((char *)buff, max_len);
// 			if (newbuff == NULL) {
// 				check_calloc(newbuff, NULL);
// 				//manda risposta ko
// 				//close(fd_c);
// 			}
// 			buff = newbuff;
// 		}
// 	}

// 	return buff;
// }


// Worker di test

//pthread_create(&worker, NULL, &test_routine, (int *)fd_c);


// void *test_routine(void *arg)
// {
// 	int fd_c = (int)arg;
// 	int n = 0;
// 	int max_len = MAX_BUFF;
// 	int len = 0;
// 	char *buff = (char*)calloc(MAX_BUFF, sizeof(char));
// 	if (buff == NULL) {
// 		perror("ERR calloc\n");
// 		exit(EXIT_FAILURE);
// 	}

// 	while ((n = read(fd_c, buff + len, max_len)) != 0) {
// 		if (n < 0 && errno == EINTR) {
// 			if (_print_stats == TRUE || _is_exit == TRUE) {
// 				continue;
// 			}
// 			break;
// 		}
// 		if (n == max_len) {
// 			len += n;
// 			max_len *= 2;
// 			char *newbuff = (char*)realloc((char *)buff, max_len);
// 			if (newbuff == NULL) {
// 				perror("ERR realloc\n");
// 				exit(EXIT_FAILURE);
// 			}
// 			buff = newbuff;
// 		}
// 	}
// 	printf("SERVER GOT: %s\n", buff);

// 	close(fd_c);
// 	return NULL;
// }