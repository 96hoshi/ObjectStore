CC = gcc
CFLAGS = -std=c99 -Wall -g -pedantic
LIBS = -lprotocol
TARGET = client objStore libprotocol.a
OBJS = protocol.o message.o object.o user.o list.o tests

.PHONY = clean all test


client : stats.o libprotocol.a
 $(CC) $(CFLAGS) $@.c -o $@ $^ -L. $(LIBS)

objStore : stats.o common.h
 $(CC) $(CFLAGS) $@.c -o $@ $^ -L. -lpthread

protocol.o : protocol.h message.o

user.o : user.h object.o

tests : object.o user.o list.o message.o

%.o : %.c %.h

libprotocol.a : protocol.o message.o stats.o
 $(AR) rvs $@ $^

runclient : client
 ./$^

runserver : objStore
 ./$^

valgrindclient : client
 valgrind ./$^

valgrindserver : objStore
 valgrind ./$^


all : $(TARGET)

clean :
#  @echo "Removing garbage"
 $(RM) $(OBJS) $(TARGET) *~

test : tests
#  @echo "Testing ... "
 valgrind ./$^
 