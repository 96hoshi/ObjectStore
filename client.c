// // Client che comunica con il server attraverso la libreria protocol.h

// // Per comunicare manda una richiesta di registrazione.
// // Successivamente può mandare tre tipi di richieste al server:
// // 	1.creare e memorizzare oggetti 
// // 		(20  oggetti di dimensione 100 <= dim <= 100.000 bytes
// // 		con nomi convenzionali contenenti dati facilmente verificabili)
// //				TODO:scegliere il contenuto dei files
// // 	2.recupero di oggetti e verifica della correttezza
// // 	3.cancellare oggetti

// // Il client riceve da linea di comando
// // 	-nome del cliente
// // 	-un numero 1-3 per sapere quale "test" eseguire
// // Finito il test stampa su stout un rapporto contenente
// // 	-#operazioni effettuate
// // 	-#operazioni concluse con successo
// // 	-#operazioni fallite
// // 	-...
// // La stampa del rapporto sarà affidata a stats.h
// // Terminati i test il client deve disconnetersi.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <common.h>
#include <protocol.h>
#include <stats.h>

#define N_OBJECTS 20
#define MIN_SIZE 100
#define MAX_SIZE 100000
#define MAX_NAME_LEN 255


void *createData(size_t len) {
	char *data = (char *)calloc(len, sizeof(char));

	for (size_t i = 0; i < len; ++i) {
		//char c = (char)i;
		data[i] = 'b';
	}
	return (void *)data;
}

int checkData(void *block, char *dataname, size_t len) {
	char *data = (char *)block;

	for (size_t i = 0; i < len; ++i) {
		char c = (char)i;
		if (data[i] != c) {
			return FALSE;
		}
	}
	return TRUE;
}

int main(int argc, char *argv[])
{
	if (argc < 3) {
		fprintf(stderr, "Not enough inputs");
		exit(EXIT_FAILURE);
	}

	argc--;
	argv++;

	char *name = argv[0];
	char *test_str = argv[1];
	int test_case = strtol(test_str, NULL, 10);

	int result = FALSE;
	size_t len = 0;
	void *data = NULL;
	char *dataname = NULL;
	size_t dataname_size = 7;
	size_t k = (MAX_SIZE - MIN_SIZE)/N_OBJECTS;
	client_stats c_stats = stats_client_create();


	result = os_connect(name);
	c_stats.total_ops++;
	if (result == FALSE) {
		c_stats.fail_ops++;
		stats_client_print(c_stats);
		exit(EXIT_FAILURE);
	}
	c_stats.success_ops++;

	switch (test_case) {

		case 1 :		// Memorizzazione
			for (size_t i = 0; i < N_OBJECTS; ++i) {
				printf("Sending object number: %zu\n", i);
				len = k * i + MIN_SIZE;
				data = createData(len);
				dataname = (char *)calloc(dataname_size, sizeof(char));
				snprintf(dataname, dataname_size, "%02zu.txt", i);

				result = os_store(dataname, data, len);
				c_stats.total_ops++;

				if (result == FALSE) {
					c_stats.fail_ops++;
				} else {
					c_stats.success_ops++;
				}

				free(data);
				data = NULL;
				free(dataname);
				dataname = NULL;
			}
			break;

		case 2 :		// Lettura
			for (size_t i = 0; i < N_OBJECTS; ++i) {
				len = k * i + MIN_SIZE;
				dataname = (char *)calloc(dataname_size, sizeof(char));
				snprintf(dataname, dataname_size, "%02zu.txt", i);

				data = os_retrieve(dataname);
				c_stats.total_ops++;

				result = checkData(data, dataname, i);

				if (result == FALSE) {
					c_stats.fail_ops++;
				} else {
					c_stats.success_ops++;
				}

				free(data);
				data = NULL;
				free(dataname);
				dataname = NULL;
			}
			break;

		case 3 :		// Cancellazione
			result = os_delete(dataname);
			c_stats.total_ops++;

			if (result == FALSE) {
				c_stats.fail_ops++;
			} else {
				c_stats.success_ops++;
			}
			break;

		default:
			invalid_operation(NULL);
			break;
	}

	printf("disconnect\n");
	os_disconnect();
	c_stats.total_ops++;
	c_stats.success_ops++;

	stats_client_print(c_stats);
	return 0;
}
