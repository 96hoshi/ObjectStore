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
//#include "message.h"
#include "stats.h"

#define MAX_BUFF 1024
#define UNIX_PATH_MAX 108
#define SOCKNAME "./objstore.sock"

volatile int _is_exit;
volatile int _print_stats;

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

// char *read_from_client(int fd_c)
// {
// 	int n = 0;
// 	char *buff = (char*)calloc(MAX_BUFF, sizeof(char));
// 	if (buff == NULL) {
// 		perror("ERR calloc\n");
// 		exit(EXIT_FAILURE);
// 	}

// 	while ((n = read(fd_c, buff, MAX_BUFF)) != 0) {
// 		if (n < 0 && errno == EINTR) {
// 			if (_print_stats == TRUE) {
// 				stats_server_print();
// 			continue;
// 			}
// 			break;
// 		}
// 		if (n == MAX_BUFF) {
// 			buff = (char*)realloc((char *)buff, 2 * MAX_BUFF);
// 			if (buff == NULL) {
// 				perror("ERR realloc\n");
// 				exit(EXIT_FAILURE);
// 			}
// 		}
// 	}

// 	return buff;
// }

// void *handle_client(void *arg)
// {
// 	int fd_c = (int)arg;
// 	client_stats c_stats = stats_client_create();

// 	char *buff = read_from_client(fd_c);

// 	// TODO: 
// 	if (_is_exit == TRUE) {
// 		;
// 		//manda risposta ko
// 		//exit();
// 	}

// 	//TODO
// 	//message m = string_to_messag(buff);
// 	//switch_case per capire l'operazione da fare ed eseguirla
// 	//manda la risposta

// 	close(fd_c);
// 	return NULL;
// }


void *test_routine(void *arg)
{
	int fd_c = (int)arg;
	int n = 0;
	char *buff = (char*)calloc(MAX_BUFF, sizeof(char));
	if (buff == NULL) {
		perror("ERR calloc\n");
		exit(EXIT_FAILURE);
	}

	while ((n = read(fd_c, buff, MAX_BUFF)) != 0) {
		if (n < 0 && errno == EINTR) {
			if (_print_stats == TRUE) {
				stats_server_print();
			continue;
			}
			break;
		}
		if (n == MAX_BUFF) {
			buff = (char*)realloc((char *)buff, 2 * MAX_BUFF);
			if (buff == NULL) {
				perror("ERR realloc\n");
				exit(EXIT_FAILURE);
			}
		}
	}
	printf("SERVER GOTS: %s\n", buff);

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

	int fd_skt = 0;
	int fd_c = 0;
	struct sockaddr_un sa;

	strncpy(sa.sun_path, SOCKNAME, sizeof(sa.sun_path));
	sa.sun_family = AF_UNIX;

	fd_skt = socket(AF_UNIX, SOCK_STREAM, 0);
	bind(fd_skt, (struct sockaddr *)&sa, sizeof(sa));
	listen(fd_skt, SOMAXCONN);

	while (!_is_exit) {
		fd_c = accept(fd_skt, NULL, 0);
		pthread_t worker;
		//pthread_create(&worker, NULL, &handle_client, (int *)fd_c);
		pthread_create(&worker, NULL, &test_routine, (int *)fd_c);

	}


	//TODO: usare pthread_cond_t
	// if ) {
	// 	close(fd_skt);
	// 	exit(EXIT_SUCCESS);
	// }

	return 0;
}
