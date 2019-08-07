// Libreria che contiene la struttura dati per contenere gli oggetti
// e le operazione per gestirla

// -objects: lista di
// 		-char* name
// 		-size_t len
// 		-objects* next

#ifndef OBJECT_H
#define OBJECT_H

#include <stdlib.h>


typedef struct {
	char *name;
	size_t len;
} object;

// - Alloca memoria per una struttura di tipo object
// - Alloca (len + 1) byte in cui copiare la stringa name
// - Copia la stringa name nella struttura object e mette '\0'
//   come ultimo carattere della stringa copiata
// - Copia la variabile len nella struttura object
object *object_create(char *name, size_t len);

// - Se entrambi gli object sono NULL ritrona 0
// - Se il primo è NULL ritorna 1
// - Se il secondo è NULL ritorna -1
// - Se nessuno degli object è NULL ritorno il valore di strcmp()
//   applicato ai name dei due object
int object_compare(void *o1, void *o2);

// - Libera la memoria utilizzata per la stringa name
// - Libera la memoria utilizzata per l'object o
void object_destroy(object * o);

#endif
