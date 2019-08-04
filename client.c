#include "protocol.h"
#include "message.h"
//#include "stats.h"
#include <stdio.h>

int main(int argc, char *argv[]) {

	printf("Sono passato da protocol : %d\n", protocolfun());
	printf("E anche da message : %d\n", messagefun());

	return 0;
}