#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdlib.h>

// inizia la connessione all'object store, registrando il cliente con il
// name dato. Restituisce true se la connessione ha avuto successo,
// false altrimenti. Notate che la connessione all'object 
// store è globale per il client.

// creazione stringa del messaggio
// creazione fd_socket client "globale"
// connessione su socket locale col server
// scrittura del messaggio nel canale
// libero memoria per il messaggio ?????
// creo un buffer per la lettura
// lettura del messaggio di risposta
// traduzione messaggio
// libero buffer
// ritorno messaggio tradotto (true o false)
int os_connect(char *name);

// richiede all'object store la memorizzazione dell'oggetto puntato da block,
// per una lunghezza len, con il nome name. Restituisce true se la
// memorizzazione ha avuto successo, false altrimenti.

// creazione del messaggio con i parametri dati
// connessione server
int os_store(char *name, void *block, size_t len);

// recupera dall'object store l'oggetto precedentemente
// memorizzatato sotto il nome name. Se il recupero ha avuto successo,
// restituisce un puntatore a un blocco di memoria,
// allocato dalla funzione, contenente i dati precedentemente memorizzati.
// In caso di errore, restituisce NULL.

// creazione del messaggio con il nome
// connessione server
void *os_retrieve(char *name);

// cancella l'oggetto di nome name precedentemente memorizzato.
// Restituisce true se la cancellazione ha avuto successo, false altrimenti.

// creazione del messaggio con il nome
// connessione server
int os_delete(char *name);

// chiude la connessione all'object store.
// Restituisce true se la disconnessione ha avuto successo,
// false in caso contrario.

// creazione del messaggio
// connessione server
// chiude la socket
int os_disconnect();

#endif
