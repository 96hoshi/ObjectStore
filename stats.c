#include "stats.h"
#include <stdio.h>
#include <stdlib.h>


server_stats _s_stats;

void stats_server_init()
{
	server_stats s_stats;
	s_stats.connected_clients = 0;
	s_stats.n_objects = 0;
	s_stats.total_size = 0;
	pthread_mutex_init(&(s_stats.mux), NULL);

	_s_stats = s_stats;
}

void stats_server_incr_client()
{
	pthread_mutex_lock(&(_s_stats.mux));
	_s_stats.connected_clients++;
	pthread_mutex_unlock(&(_s_stats.mux));
}

void stats_server_decr_client()
{
	pthread_mutex_lock(&(_s_stats.mux));
	_s_stats.connected_clients--;
	pthread_mutex_unlock(&(_s_stats.mux));
}

void stats_server_incr_obj()
{
	pthread_mutex_lock(&(_s_stats.mux));
	_s_stats.n_objects++;
	pthread_mutex_unlock(&(_s_stats.mux));
}

void stats_server_decr_obj()
{
	pthread_mutex_lock(&(_s_stats.mux));
	_s_stats.n_objects--;
	pthread_mutex_unlock(&(_s_stats.mux));
}

void stats_server_incr_size(int len)
{
	pthread_mutex_lock(&(_s_stats.mux));
	_s_stats.total_size += len;
	pthread_mutex_unlock(&(_s_stats.mux));
}

void stats_server_decr_size(int len)
{
	pthread_mutex_lock(&(_s_stats.mux));
	_s_stats.total_size -= len;
	pthread_mutex_unlock(&(_s_stats.mux));
}

void stats_server_print()
{
	fprintf(stdout, "Connected clients: %d\n", _s_stats.connected_clients);
	fprintf(stdout, "Number objects: %d\n", _s_stats.n_objects);
	fprintf(stdout, "Total size: %d\n", _s_stats.total_size);
}

void stats_server_destroy()
{
	pthread_mutex_destroy(&(_s_stats.mux));
}
