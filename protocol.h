#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdlib.h>


int os_connect(char *name);

int os_store(char *name, void *block, size_t len);

void *os_retrieve(char *name);

int os_delete(char *name);

int os_disconnect();

#endif
