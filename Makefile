CC = gcc
CFLAGS = -std=c99 -Wall -pedantic

.PHONY = clean all test

#nomi simbolici
TARGETS = client objStore libprotocol.a
OBJECTS = protocol.o message.o

#regole
client : libprotocol.a
	$(CC) $(CFLAGS) client.c -o client -L. -lprotocol

objStore : message.o

protocol.o : protocol.c protocol.h

message.o : message.c message.h

libprotocol.a : $(OBJECTS)
	ar rvs $@ $^


runclient : client
	./client

runserver : objStore
	./objStore


all : $(TARGETS)

clean :
	@echo "Removing garbage"
	-rm $(OBJECTS) $(TARGETS)

test : 
	@echo "Testing ... "
