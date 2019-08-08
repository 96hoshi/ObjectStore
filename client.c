// Client che comunica con il server attraverso la libreria protocol.h

// Per comunicare manda una richiesta di registrazione.
// Successivamente può mandare tre tipi di richieste al server:
// 	1.creare e memorizzare oggetti 
// 		(20  oggetti di dimensione 100 <= dim <= 100.000 bytes
// 		con nomi convenzionali contenenti dati facilmente verificabili)
//				TODO:scegliere il contenuto dei files
// 	2.recupero di oggetti e verifica della correttezza
// 	3.cancellare oggetti

// Il client riceve da linea di comando
// 	-nome del cliente
// 	-un numero 1-3 per sapere quale "test" eseguire
// Finito il test stampa su stout un rapporto contenente
// 	-#operazioni effettuate
// 	-#operazioni concluse con successo
// 	-#operazioni fallite
// 	-...
// La stampa del rapporto sarà affidata a stats.h
// Terminati i test il client deve disconnetersi.

#include "protocol.h"
#include "stats.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{

	return 0;
}
