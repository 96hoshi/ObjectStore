#ifndef WORKER_H
#define WORKER_H

#include <stdlib.h>
#include <signal.h>


void *handle_client(void *arg);

void spawn_thread(long fd_c);

#endif
