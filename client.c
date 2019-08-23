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
#include <common.h>
#include <message.h>

#define N 1024

static void message_print(message *m) {
	printf("m->buff = %s\n", m->buff);
	printf("m->op = %d\n", m->op);
	printf("m->name = %s\n", m->name);
	printf("m->len = %zu\n", m->len);
	if (m->data != NULL)
		printf("m->data = %p %c\n", m->data, m->data[0]);
}

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
	memset(&sa, 0, sizeof(sa));


	strncpy(sa.sun_path, SOCKNAME, sizeof(sa.sun_path));
	sa.sun_family = AF_UNIX;

	fd_skt = socket(AF_UNIX, SOCK_STREAM, 0);

	while (connect(fd_skt, (struct sockaddr*)&sa, sizeof(sa)) == -1) {
		if (errno == ENOENT)
			sleep(1);	//sock non esiste
		else exit(EXIT_FAILURE);
	}

	// message *m = message_create(NULL, message_register, "Marta", 0, NULL); 			done	// REGISTER name \n  
	//message *m = message_create(NULL, message_store, "DataName", 9, "Some Data"); 	done		// STORE name len \n data
								//"STORE DataName 9 \n Some Data"
	message *m = message_create(NULL, message_data, NULL, 9, "Some Data");					// DATA len \n data
	// message *m = message_create(NULL, message_ko, "Bad String", 0, NULL);			done	// KO message \n
	// message *m = message_create(NULL, message_leave, NULL, 0, NULL);					done	// LEAVE \n


	message_send(fd_skt, m);
	 message *received = message_receive(fd_skt);

	message_print(received);
	message_destroy(m);
	message_destroy(received);


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