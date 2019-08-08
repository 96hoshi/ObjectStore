// Libreria che contiene la struttura dati per contenere gli utenti
// e le operazione per gestirla

// Struttura dati:
// 	-users:
// 	-char* name
// 	-objects* object
// 	-size_t fd

// Implementazione con liste e lock

#ifndef USER_H
#define USER_H

#include "object.h"

typedef struct {
	char *name;
	list_t *objects;
	int fd;
} user;

// - Alloca memoria per una struttura di tipo user
// - Alloca (strlen(name) + 1) byte in cui copiare la stringa name
// - Copia la stringa name nella struttura user
// - Crea la lista di object
// - Copia nel campo fd il valore della socket usata dall'user
user *user_create(char *name, int fd);

// - Cerca con una funzione di libreria l'oggetto di nome name
//   nella lista objs
object *user_search_object(char *name);

// - Se entrambi gli user sono NULL ritrona 0
// - Se il primo è NULL ritorna 1
// - Se il secondo è NULL ritorna -1
// - Se nessuno degli user è NULL ritorno il valore di strcmp()
//   applicato ai name dei due user
int user_compare(void *u1, void *u2);

// - Libera la memoria utilizzata per la stringa name
// - Libera la memoria utilizzata dalla lista di object
// - Libera la memoria utilizzata per la struttura user
void user_destroy(user *u);

#endif
