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

#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>
#include <errno.h>
#include "common.h"
#include "user.h"
#include "list.h"
#include "stats.h"
#include "worker.h"


volatile sig_atomic_t _print_stats;
volatile sig_atomic_t _is_exit;
list *_users;
list *_threads;


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

int main(int argc, char *argv[])
{
	unlink(SOCKNAME);
	set_sigaction();
	stats_server_init();

	long fd_skt = 0;
	long fd_c = 0;
	struct sockaddr_un sa;
	memset(&sa, 0, sizeof(sa));

	// Creating data
	struct stat st = {0};
	if (stat(PATH_DATA, &st) == -1) {
		mkdir(PATH_DATA, 0700);
	}

	_is_exit = FALSE;
	// Creating connected users' list
	_users = list_create(user_compare,
						 user_destroy,
						 user_print);
	_threads = NULL;
	// _threads = list_create(worker_compare,
	// 						  worker_destroy,
	// 						  NULL);

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
	}

	//TODO: usare pthread_cond_t
	// if ) {
	// 	close(fd_skt);
	// 	exit(EXIT_SUCCESS);
	// }

	list_destroy(_users);
	stats_server_destroy();
	close(fd_skt);
	return 0;
}
