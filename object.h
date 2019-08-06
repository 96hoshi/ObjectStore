// Libreria che contiene la struttura dati per contenere gli oggetti
// e le operazione per gestirla

// -objects: lista di
// 		-char* name
// 		-size_t len
// 		-objects* next

#ifndef OBJECT_H
#define OBJECT_H

typedef struct o {
	char *name;
	int len;
	struct o *next;
}object;

// Aggiunge un oggetto di nome nameObj alla lista 
// Ritora 0 se l'inserimento è avvenuto in maniera corretta, -1 altrimenti
// La lista non deve contenere duplicati
int addObj(object **list, char *nameObj);

object *searchObj(object *list, char *nameObj);

int deleteObj(object **list, char *nameObj);

// Stampa tutta la lista
void printObjects(object *list);

// Procedure di supporto
// Cancella tutti i dati :eyes:
void cleanObjects(object *list);

#endif
