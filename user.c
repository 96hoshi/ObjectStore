#include "user.h"
#include <stdlib.h>
#include <string.h>
#include "object.h"


// - Alloca memoria per una struttura di tipo user
// - Alloca (strlen(name) + 1) byte in cui copiare la stringa name
// - Copia la stringa name nella struttura user
// - Crea la lista di object
// - Copia nel campo fd il valore della socket usata dall'user
user *user_create(char *name, int fd) 
{
	if(name == NULL) {
		return NULL;
	}

	size_t name_len = strlen(name) + 1;
	user *u = (user *)calloc(1, sizeof(user));
	u->name = (char *)calloc(name_len, sizeof(char));
	u->name = strcpy(u->name, name);
	//u->objects = list_create(); TODO: uncomment when list_t is created
	u->fd = fd;

	return u;
}


// - Cerca con una funzione di libreria l'oggetto di nome name
//   nella lista objs
object *user_search_object(char *name) //TODO: implement it when list_t is created
{
	return NULL;
}

// - Se entrambi gli user sono NULL ritrona 0
// - Se il primo è NULL ritorna 1
// - Se il secondo è NULL ritorna -1
// - Se nessuno degli user è NULL ritorno il valore di strcmp()
//   applicato ai name dei due user
int user_compare(void *u1, void *u2)
{
	user *user1 = (user *)u1;
	user *user2 = (user *)u2;

	if(user1 == NULL && user2 == NULL) return 0;
	if(user1 == NULL) return 1;
	if(user2 == NULL) return -1;

	return strcmp(user1->name, user2->name);
}

// - Libera la memoria utilizzata per la stringa name
// - Libera la memoria utilizzata dalla lista di object
// - Libera la memoria utilizzata per la struttura user
void user_destroy(user *u)
{
	if(u == NULL) return;
	if(u->name != NULL) free(u->name);
	//list_destroy(objects); TODO: uncomment when list_t is created
	free(u);
}
