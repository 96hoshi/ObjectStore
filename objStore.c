#define _POSIX_C_SOURCE 199506L // needed by pthread sigmask

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>
#include <pthread.h>
#include <errno.h>
#include "common.h"
#include "user.h"
#include "object.h"
#include "list.h"
#include "stats.h"
#include "worker.h"


#define PATH_DUMP PATH_DATA"/dump.dat"

volatile sig_atomic_t _print_stats;
volatile sig_atomic_t _is_exit;

pthread_mutex_t _workers_mux;
pthread_cond_t _workers_cond;
size_t _workers;
list *_users;


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

void users_dump_restore()
{
	if (access(PATH_DUMP, F_OK) == -1) {
		return;
	}

	FILE *f = fopen(PATH_DUMP, "r");
	if (f == NULL) {
		fprintf(stderr, "Error while reading "PATH_DUMP);
		exit(EXIT_FAILURE);
	}

	size_t users_count = 0;
	fscanf(f, "%zu", &users_count);

	for (size_t i = 0; i < users_count; ++i) {
		size_t name_size = 0;
		char *name = NULL;
		fscanf(f, "%zu", &name_size);

		if (name_size > 0) {
			name = (char *)calloc(name_size + 1, sizeof(char));
			if (name == NULL) {
				exit(EXIT_FAILURE);
			}
			fscanf(f, "%s", name);
		}
		user *u = user_create(name);
		list_insert_unsafe(_users, u);

		size_t objects_count = 0;
		fscanf(f, "%zu", &objects_count);
		for (size_t j = 0; j < objects_count; ++j) {
			size_t object_size = 0;
			char *object_name = NULL;
			size_t object_len = 0;

			fscanf(f, "%zu", &object_size);
			if (object_size > 0) {
				object_name = (char *)calloc(object_size + 1, sizeof(char));
				if (object_name == NULL) {
					exit(EXIT_FAILURE);
				}
				fscanf(f, "%s", object_name);
			}
			fscanf(f, "%zu", &object_len);

			user_insert_object(u, object_name, object_len);
			stats_server_incr_obj();
			stats_server_incr_size(object_len);
		}
	}
}

int main(int argc, char *argv[])
{
	_print_stats = FALSE;
	_is_exit = FALSE;

	set_sigaction();
	stats_server_init();

	pthread_mutex_init(&_workers_mux, NULL);
	pthread_cond_init(&_workers_cond, NULL);
	_workers = 0;

	// Creating connected users' list
	_users = list_create(user_compare,
						 user_destroy,
						 user_dump);
	if (_users == NULL) {
		exit(EXIT_FAILURE);
	}
	users_dump_restore();

	struct stat st = {0};
	if (stat(PATH_DATA, &st) == -1) {
		mkdir(PATH_DATA, 0700);
	}

	long fd_skt = 0;
	struct sockaddr_un sa;
	memset(&sa, 0, sizeof(sa));
	strncpy(sa.sun_path, SOCKNAME, sizeof(sa.sun_path));
	sa.sun_family = AF_UNIX;

	fd_skt = socket(AF_UNIX, SOCK_STREAM, 0);
	bind(fd_skt, (struct sockaddr *)&sa, sizeof(sa));
	listen(fd_skt, SOMAXCONN);

	while (!_is_exit) {
		long fd_c = accept(fd_skt, NULL, 0);
		if (fd_c < 0) {
			if (errno == EINTR) {
				if (_print_stats == TRUE) {
					stats_server_print();
					_print_stats = FALSE;
				}
				continue;
			}
			_is_exit = TRUE;
			break;
		}
		stats_server_incr_served();
		worker_spawn(fd_c);
	}

	// Awaits for workers
	pthread_mutex_lock(&_workers_mux);
	while(_workers > 0) {
		pthread_cond_wait(&_workers_cond, &_workers_mux);
	}
	pthread_mutex_unlock(&_workers_mux);

	pthread_mutex_destroy(&_workers_mux);
	pthread_cond_destroy(&_workers_cond);

	FILE *f = fopen(PATH_DUMP, "w");
	if (f == NULL) {
		fprintf(stderr, "Error while creating "PATH_DUMP);
	}
	list_dump(_users, f);
	fclose(f);

	list_destroy(_users);
	stats_server_destroy();

	close(fd_skt);
	unlink(SOCKNAME);

	return 0;
}
