// Client che comunica con il server attraverso la libreria protocol.h

// Per comunicare manda una richiesta di registrazione.
// Successivamente può mandare tre tipi di richieste al server:
// 	1.creare e memorizzare oggetti 
// 		(20  oggetti di dimensione 100 <= dim <= 100.000 bytes
// 		con nomi convenzionali contenenti dati facilmente verificabili)
//				TODO:scegliere il contenuto dei files
// 	2.recupero di oggetti e verifica della correttezza
// 	3.cancellare oggetti

// Il client riceve da linea di comando
// 	-nome del cliente
// 	-un numero 1-3 per sapere quale "test" eseguire
// Finito il test stampa su stout un rapporto contenente
// 	-#operazioni effettuate
// 	-#operazioni concluse con successo
// 	-#operazioni fallite
// 	-...
// La stampa del rapporto sarà affidata a stats.h
// Terminati i test il client deve disconnetersi.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "common.h"

#define N 1024

int main(int argc, char *argv[])
{
	// if (argc < 3) {
	// 	fprintf(stderr, "Not enough input");
	// 	exit(EXIT_FAILURE);
	// }

	// argc--;
	// argv++;

	//char *name = argv[0];
	//char *test_str = argv[1];

	int fd_skt;
	char buf[N];
	struct sockaddr_un sa;

	strncpy(sa.sun_path, SOCKNAME, sizeof(sa.sun_path));
	sa.sun_family = AF_UNIX;

	fd_skt = socket(AF_UNIX, SOCK_STREAM, 0);

	while (connect(fd_skt, (struct sockaddr*)&sa, sizeof(sa)) == -1) {
		if (errno == ENOENT)
			sleep(1);	//sock non esiste
		else exit(EXIT_FAILURE);
	}

	write(fd_skt, "REGISTER Marta \n", 15);
	read(fd_skt, buf, N);
	printf("Client got: %s\n", buf);


	// int result = TRUE;
	// int test_case = strtol(test_str, NULL, 10);

	// result = os_connect(name);

	// switch (test_case) {
	// 	case 1 : 
	// 		// Memorizzazione
	// 		break;
	// 	case 2 :
	// 		// Lettura
	// 		break;
	// 	case 3 :
	// 		// Cancellazione
	// 		break;
	// 	default:
	// 		break;
	// }

	//result = os_disconnect()

	close(fd_skt);
	exit(EXIT_SUCCESS);

	return 0;
}