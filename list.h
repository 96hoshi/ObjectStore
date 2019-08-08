#ifndef LIST_H
#define LIST_H

#include <stdlib.h>
#include <pthread.h>

typedef int (*fun_info_compare)(void *, void *);
typedef void (*fun_info_destroy)(void *);

typedef struct _node{
	void *info;
	struct _node *next;
} node;

typedef struct {
	node **head;
	size_t count;
	pthread_mutex_t mux;
	fun_info_compare info_compare;
	fun_info_destroy info_destroy;
} list;


//Thread Unsafe functions

// - Alloca memoria per una struttura di tipo list
// - Inizializzo la variabile head a NULL
// - Inizializza la variabile count a 0
// - Copia le variabili delle due funzioni nella struttura list
list *list_create(fun_info_compare info_compare,
				  fun_info_destroy info_destroy);

// - Inserzione in testa
// - Alloca spazio per una struttura di tipo node
// - Copio la variabile info nella struttura
// - Come campo next pongo il valore di l->head
// - Cambio il valore di head con il puntatore alla struttura appena creata
//NB: La lista deve ammettere solo elementi diversi!
void list_insert(list *l, void *info);

// Per ogni elemento di l
//   confronta con la funzione info_compare della struttura il valore info
//   con l->(*head)->info
// Restituisco il nodo trovato
node *list_search(list *l, void *info);

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
void list_delete(list *l, void *info);

// - Per ogni elemento di l
//    Invoca la funzione info_destroy sul nodo
//    Cambio il valore di *head facendolo puntare al successivo
//    Libero memoria per il nodo
//    Decremento count - unnecessary ma lo fo lo stesso
// - Liero la memoria per l
void list_destroy(list *l);

//Fa la lock della var mux della lista l
void list_lock(list *l);

//Fa la unlock della var mux della lista l
void list_unlock(list *l);


// Thread Safe functions

list *list_create_s(fun_info_compare info_compare,
					fun_info_destroy info_destroy);

void list_insert_s(list *l, void *info);

node *list_search_s(list *l, void *info);

void list_delete_s(list *l, void *info);

void list_destroy_s(list *l);

#endif
