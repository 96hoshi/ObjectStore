// Contiene le funzioni che utilizzerà il client per comunicare col server
// Responsabile di ricevere, tradurre e mandare messaggi
// La traduzione avviene mediante le funzioni di libreria message.h

// Il protocollo deve controllare :
// -la correttezza dei nomi (utenti e dati)
// 	-standard POSIX
// 	-!= NULL
// 	-univoco nella cartella ----> controllo lato server
// -la correttezza dei dati
// 	-len >= 0 ------------------> un utente può salvare oggetti vuoti
// -la correttezza della dim dei dati
// 	-block == NULL <=> len == 0

// Se questi controlli sono verificati manda al server una socket con l'header
// creato da message.h
//Riceve poi l'esito da parte del server e manda la risposta al client

#include "protocol.h"
#include "message.h"
#include <stdio.h>
#include <stdlib.h>

int os_connect(char *name)
{

	return 0;
}

int os_store(char *name, void *block, size_t len)
{
	return 0;
}

void *os_retrieve(char *name)
{
 return NULL;
}

int os_delete(char *name)
{
	return 0;
}

int os_disconnect()
{
	return 0;
}