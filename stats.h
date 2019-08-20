//Responsabile della stampa delle statistiche di client e server

//Client:
// 	-#operazioni effettuate
// 	-#operazioni concluse con successo
// 	-#operazioni fallite
// 	-...

// Con operazione identifico un unica chiamata di funzione
// della libreria lato client
// Considerato che un client di test esegue al massimo
// - una os_connect
// - 20+ test
// - una os_disconnect


//Server:
// 	-#client connessi
// 	-#oggetti nello store
// 	-size totale dello store
// 	-...

#ifndef STATS_H
#define STATS_H

#include <pthread.h>

typedef struct {
	int total_ops;
	int success_ops;
	int fail_ops;
} client_stats;

typedef struct {
	int connected_clients;
	int n_objects;
	int total_size;
	pthread_mutex_t mux;
} server_stats;

server_stats _s_stats;

client_stats stats_client_create();

void stats_client_print(client_stats c_stats);


void stats_server_create();

void stats_server_incr_client();

void stats_server_decr_client();

void stats_server_incr_obj();

void stats_server_decr_obj();

void stats_server_incr_size(int len);

void stats_server_decr_size(int len);

void stats_server_print();

void stats_server_destroy();

#endif
