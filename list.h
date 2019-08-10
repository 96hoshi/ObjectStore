#ifndef LIST_H
#define LIST_H

#include <stdlib.h>
#include <pthread.h>

typedef enum {
	list_success = 0,
	list_null = -10,
	list_not_found,
	list_duplicate,
	list_err_mux
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
	fun_info_compare info_compare_field;
	fun_info_destroy info_destroy;
	fun_info_print info_print;
} list;


//Thread Unsafe functions

// - Alloca memoria per una struttura di tipo list
// - Inizializzo la variabile head a NULL
// - Inizializza la variabile count a 0
// - Copia le variabili delle due funzioni nella struttura list
list *list_create(fun_info_compare info_compare,
				  fun_info_compare info_compare_field,
				  fun_info_destroy info_destroy,
				  fun_info_print info_print);

// - Inserzione in testa
// - Alloca spazio per una struttura di tipo node
// - Copio la variabile info nella struttura
// - Come campo next pongo il valore di l->head
// - Cambio il valore di head con il puntatore alla struttura appena creata
// NB: La lista deve ammettere solo elementi diversi!
list_result list_insert_unsafe(list *l, void *info);

void *list_search_info_unsafe(list *l, void *info);

void *list_search_field_unsafe(list *l, void *field);

// Per ogni elemento di l
//   confronta con la funzione info_compare della struttura il valore info
//   con l->(*head)->info
// Restituisco il nodo trovato
//TODO: Se non la uso da nessuna parte togliere il prototipo dall'header
void *list_search_unsafe(list *l, void *info, fun_info_compare info_compare);

// - Ricerca per info
list_result list_delete_info_unsafe(list *l, void *info);

// - Ricerca per uno specifico field
list_result list_delete_field_unsafe(list *l, void *field);

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
list_result list_delete_unsafe(list *l, void *info, fun_info_compare info_compare);

// - Per ogni elemento di l
//    Invoca la funzione info_destroy sul nodo
//    Cambio il valore di *head facendolo puntare al successivo
//    Libero memoria per il nodo
//    Decremento count - unnecessary ma lo fo lo stesso
// - Liero la memoria per l
list_result list_destroy(list *l);

// - Fa la lock della var mux della lista l
list_result list_lock(list *l);

// - Fa la unlock della var mux della lista l
list_result list_unlock(list *l);

// - Stampa il contenuto della lista
list_result list_print(list *l);

// Thread Safe functions

list_result list_insert(list *l, void *info);

node *list_search(list *l, void *info, fun_info_compare info_compare);

list_result list_delete(list *l, void *info, fun_info_compare info_compare);

#endif
