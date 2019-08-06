// Traduce i messaggi tra client (request) e server (response) in strutture "leggibili".

// Messaggio = "header" ,codificato in ASCII, è una stringa di testo terminata da "\n"
// a cui può succedere un blocco di dati.

// Cosa posso trovare nei messaggi:
// 	OP 	= array di caratteri che indica l'operazione da eseguire
// 	name 	= array di caratteri che contiene il nome del client o del dato
// 	len 	= lunghezza in byte del dato, codificato in ASCII
// 	data 	= blocco binario i lunghezza len, posto dopo lo "\n"
// 	message = array di caratteri che contiene in messaggio di errore del server

// Struttura dati:
// -msg:
// 	-int OP
// 	-char* name
// 	-int len
// 	-void* data (che conterrà anche message in caso)

#include "message.h"
