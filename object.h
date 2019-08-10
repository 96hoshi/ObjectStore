// Libreria che contiene la struttura dati per contenere l'oggetto
// e le operazioni per gestirla

#ifndef OBJECT_H
#define OBJECT_H

#include <stdlib.h>

typedef struct {
	char *name;
	size_t len;
} object;

// - Alloca memoria per una struttura di tipo object
// - Alloca (strlen(name) + 1) byte in cui copiare la stringa name
// - Copia la stringa name nella struttura object
// - Copia la variabile len nella struttura object
object *object_create(char *name, size_t len);

// TODO: Scrivere la specifica
int object_compare(void *obj1, void *obj2);

// TODO: Scrivere la specifica
int object_compare_name(void *obj, void *obj_name);

// - Libera la memoria utilizzata per la stringa name
// - Libera la memoria utilizzata per l'object o
void object_destroy(void *obj);

void object_print(void *obj);

#endif
