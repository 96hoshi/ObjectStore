#include "object.h"
#include <string.h>

// - Alloca memoria per una struttura di tipo object
// - Alloca (strlen(name) + 1) byte in cui copiare la stringa name
// - Copia la stringa name nella struttura object
// - Copia la variabile len nella struttura object
object *object_create(char *name, size_t len)
{
	if(name == NULL) {
		return NULL;
	}

	size_t name_len = strlen(name) + 1;
	object *o = (object *)calloc(1, sizeof(object));
	o->name = (char *)calloc(name_len, sizeof(char));
	o->name = strcpy(o->name, name);
	o->len = len;

	return o;

}

// - Se entrambi gli object sono NULL ritrona 0
// - Se il primo è NULL ritorna 1
// - Se il secondo è NULL ritorna -1
// - Se nessuno degli object è NULL ritorno il valore di strcmp()
//   applicato ai name dei due object
int object_compare(void *o1, void *o2)
{
	object *obj1 = (object *)o1;
	object *obj2 = (object *)o2;

	if(obj1 == NULL && obj2 == NULL) {
		return 0;
	}

	if(obj1 == NULL) {
		return 1;
	}

	if(obj2 == NULL) {
		return -1;
	}

	return strcmp(obj1->name, obj2->name);
}

// - Libera la memoria utilizzata per la stringa name
// - Libera la memoria utilizzata per l'object o
void object_destroy(object *o)
{
	if (o == NULL) {
		return;
	}

	free(o->name);
	free(o);
}
