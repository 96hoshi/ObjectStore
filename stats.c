
// typedef struct {
// 	int total_ops;
// 	int success_ops;
// 	int fail_ops;
// } client_stats;

// typedef struct {
// 	int connected_clients;
// 	int n_objects;
// 	int total_size;
// 	pthread_mutex_t mux;
// } server_stats;

#include "stats.h"
#include <stdio.h>
#include <stdlib.h>

client_stats stats_client_create()
{
	client_stats c_stats;
	c_stats.total_ops = 0;
	c_stats.success_ops = 0;
	c_stats.fail_ops = 0;

	return c_stats;
}

void stats_client_print(client_stats c_stats)
{
	printf("%d %d %d\n", c_stats.total_ops,
						 c_stats.success_ops,
						 c_stats.fail_ops);
}

void stats_server_create()
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
	printf("Connected clients: %d\t", _s_stats.connected_clients);
	printf("Number objects: %d\t", _s_stats.n_objects);
	printf("Total size: %d\n", _s_stats.total_size);
}

void stats_server_destroy()
{
	pthread_mutex_destroy(&(_s_stats.mux));
}
