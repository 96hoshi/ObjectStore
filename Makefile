CC = gcc
CFLAGS = -std=c99 -Wall -g -pedantic

.PHONY = clean all test

#Nomi simbolici
TARGETS = client objStore libprotocol.a tests object.o user.o list.o
OBJECTS = protocol.o message.o 

#Regole
client : libprotocol.a stats.h
	$(CC) $(CFLAGS) $@.c -o $@ -L. -lprotocol

objStore : message.o object.o stats.h

protocol.o : protocol.c protocol.h message.o

message.o : message.c message.h

user.o : user.c user.h object.o

object.o : object.c object.h

list.o : list.c list.h


libprotocol.a : $(OBJECTS)
	ar rvs $@ $^

tests : tests.c object.o user.o list.o

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

test : tests
	@echo "Testing ... "
	valgrind ./tests


#Warnings so far:
# message.c:20:0: warning: ISO C forbids an empty translation unit [-Wpedantic]
#  //#include "ops.h"
#  ^

# stats.h:19:0: warning: ISO C forbids an empty translation unit [-Wpedantic]
#  #endif
#  ^
