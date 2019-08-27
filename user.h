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

#include "list.h"
#include "object.h"


typedef struct {
	char *name;
	list *objects;
} user;

// - Alloca memoria per una struttura di tipo user
// - Alloca (strlen(name) + 1) byte in cui copiare la stringa name
// - Copia la stringa name nella struttura user
// - Crea la lista di object
// - Copia nel campo fd il valore della socket usata dall'user
user *user_create(char *name);

// - Cerca con una funzione di libreria l'oggetto di nome name
//   nella lista objs
object *user_search_object(user *u, void *name);

// - Inserisce con una funzione di libreria l'oggetto di nome name
//   e lunghezza len nella lista objs
list_result user_insert_object(user *u, char *name, size_t len);

// - Elimina con una funzione di libreria l'oggetto di nome name
//   nella lista objs
list_result user_delete_object(user *u, object *obj);

// - Confronta un user con un atro user
int user_compare(void *usr1, void *usr2);

// - Confronta un user tramite la stringa usr_name
int user_compare_name(void *usr, void *usr_name);

// - Libera la memoria utilizzata per la stringa name
// - Libera la memoria utilizzata dalla lista di object
// - Libera la memoria utilizzata per la struttura user
void user_destroy(void *usr);

void user_print(void *usr);

#endif
