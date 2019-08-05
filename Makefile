CC = gcc
CFLAGS = -std=c99 -Wall -g -pedantic

.PHONY = clean all test

#Nomi simbolici
TARGETS = client objStore libprotocol.a
OBJECTS = protocol.o message.o

#Regole
client : libprotocol.a stats.h
	$(CC) $(CFLAGS) client.c -o client -L. -lprotocol

objStore : message.o stats.h

protocol.o : protocol.c protocol.h message.o

message.o : message.c message.h

libprotocol.a : $(OBJECTS)
	ar rvs $@ $^


runclient : client
	./client

runserver : objStore
	./objStore

valgrindclient : client
	valgrind ./$^

valgrindserver : objStore
	valgrind ./$^


all : $(TARGETS)

clean :
	@echo "Removing garbage"
	-rm $(OBJECTS) $(TARGETS)

test :
	@echo "Testing ... "


#Warnings so far:
# message.c:20:0: warning: ISO C forbids an empty translation unit [-Wpedantic]
#  //#include "ops.h"
#  ^

# stats.h:19:0: warning: ISO C forbids an empty translation unit [-Wpedantic]
#  #endif
#  ^
