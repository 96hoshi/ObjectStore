#ifndef STATS_H
#define STATS_H

#include <pthread.h>
#include <signal.h>


typedef struct {
	int served_clients;
	int connected_clients;
	int n_objects;
	int total_size;
	pthread_mutex_t mux;
} server_stats;


void stats_server_init();

void stats_server_incr_served();

void stats_server_incr_client();

void stats_server_decr_client();

void stats_server_incr_obj();

void stats_server_decr_obj();

void stats_server_incr_size(int len);

void stats_server_decr_size(int len);

void stats_server_print();

void stats_server_destroy();

#endif
