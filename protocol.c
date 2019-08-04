// Contiene le funzioni che utilizzerà il client per comunicare col server
// Responsabile di ricevere, tradurre e mandare messaggi
// La traduzione avviene mediante le funzioni di libreria message.h

// Il protocollo deve controllare :
// -la correttezza dei nomi (utenti e dati)
// 	-standard POSIX
// 	-!= NULL
// 	-univoco nella cartella
// -la correttezza dei dati
// 	-len > 0
// -la correttezza della dim dei dati
// 	-len == dim del block ---> problemi socket?
// 	-block != NULL

// Se questi controlli sono verificati manda al server una socket con l'header
// creato da message.h
//Riceve poi l'esito da parte del server e manda la risposta al client

#include "protocol.h"
#include "message.h"
#include <stdio.h>
#include <stdlib.h>

// inizia la connessione all'object store, registrando il cliente con il
// name dato. Restituisce true se la connessione ha avuto successo,
// false altrimenti. Notate che la connessione all'object 
// store è globale per il client.
int os_connect(char *name)
{
	return 0;
}

// richiede all'object store la memorizzazione dell'oggetto puntato da block,
// per una lunghezza len, con il nome name. Restituisce true se la
// memorizzazione ha avuto successo, false altrimenti.
int os_store(char *name, void *block, size_t len)
{
	return 0;
}

// recupera dall'object store l'oggetto precedentemente
// memorizzatato sotto il nome name. Se il recupero ha avuto successo,
// restituisce un puntatore a un blocco di memoria,
// allocato dalla funzione, contenente i dati precedentemente memorizzati.
// In caso di errore, restituisce NULL.
void *os_retrieve(char *name)
{
 return NULL;
}

// cancella l'oggetto di nome name precedentemente memorizzato.
// Restituisce true se la cancellazione ha avuto successo, false altrimenti.
int os_delete(char *name)
{
	return 0;
}

// chiude la connessione all'object store.
// Restituisce true se la disconnessione ha avuto successo,
// false in caso contrario.
int os_disconnect()
{
	return 0;
}
