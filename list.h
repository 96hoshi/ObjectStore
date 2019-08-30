#ifndef LIST_H
#define LIST_H

#include <stdlib.h>
#include <pthread.h>


typedef enum {
	list_success = 0,
	list_null = -10,
	list_not_found
} list_result;

typedef int (*fun_info_compare)(void *, void *);
typedef void (*fun_info_destroy)(void *);
typedef void (*fun_info_print)(void *);

typedef struct _node{
	void *info;
	struct _node *next;
} node;

typedef struct {
	node *head;
	size_t count;
	pthread_mutex_t mux;
	fun_info_compare info_compare;
	fun_info_destroy info_destroy;
	fun_info_print info_print;
} list;


//Thread Unsafe functions

// - Alloca memoria per una struttura di tipo list
// - Inizializzo la variabile head a NULL
// - Inizializza la variabile count a 0
// - Copia le variabili delle due funzioni nella struttura list
list *list_create(fun_info_compare info_compare,
				  fun_info_destroy info_destroy,
				  fun_info_print info_print);

// - Inserzione in testa
// - Alloca spazio per una struttura di tipo node
// - Copio la variabile info nella struttura
// - Come campo next pongo il valore di l->head
// - Cambio il valore di head con il puntatore alla struttura appena creata
list_result list_insert_unsafe(list *l, void *info);

// Per ogni elemento di l
//   confronta con la funzione info_compare della struttura il valore info
//   con l->(*head)->info
// Restituisco il nodo trovato
//TODO: Se non la uso da nessuna parte togliere il prototipo dall'header
void *list_search_unsafe(list *l, void *info);

// - Tenendo un puntatore al nodo precedente (inizialmente NULL)
// - Per ogni elemento di l
//    Confronta con la funzione info_compare della struttura il valore info
//    con l->(*head)->info
// - Se trova l'elemento collega il nodo precedente al successivo
// - Controllo ulteriore se l'elemento da eliminare è *head
//    In tal caso cambio il valore di head nella struttura l
// - Invoca la funzione info_destroy su quel nodo
// - Libero la memoria per quel nodo
// - Decremento count
//TODO: Se non la uso da nessuna parte togliere il prototipo dall'header
void* list_delete_unsafe(list *l, void *info);


// - Per ogni elemento di l
//    Invoca la funzione info_destroy sul nodo
//    Cambio il valore di *head facendolo puntare al successivo
//    Libero memoria per il nodo
//    Decremento count - unnecessary ma lo fo lo stesso
// - Liero la memoria per l
list_result list_destroy(list *l);

// - Stampa il contenuto della lista just in case
list_result list_print(list *l);

// Thread Safe functions

list_result list_insert(list *l, void *info);

void *list_search(list *l, void *info);

void *list_delete(list *l, void *info);

#endif
