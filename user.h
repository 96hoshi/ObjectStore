// Libreria che contiene la struttura dati per contenere gli utenti
// e le operazione per gestirla

// Struttura dati:
// 	-users:
// 	-char* name
// 	-char* path dir ?
// 	-objects* object
// 	-users* next

// Implementazione con liste e lock

#ifndef USER_H
#define USER_H

typedef struct us {
	char *name;
	object *obj;
	struct us *next;
}user;

// Aggiunge un utente di nome nameUs alla lista 
// Ritora 0 se l'inserimento è avvenuto in maniera corretta, -1 altrimenti
// La lista non deve contenere duplicati
int addUs(user **list, char *nameUs);

// Cerca un utente e lo restituisce, se esiste
// NULL altrimenti
user searchUs(user *list, char *nameUs);

// Elimina un utente dalla lista
// Restituisce 0 se effettua la cancellazione con successo, -1 altrimenti
int deleteUs(user **list, char *nameUs);

// Stampa tutta la lista
void printUsers(user **list);

// Procedure di supporto
// Libera la memoria di un singolo nodo
void freeNode(user *u);

// Cancella tutti gli user
void cleanUsers(user **list);

#endif
