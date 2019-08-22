CC = gcc
CFLAGS = -I. -std=c99 -Wall -g -pedantic
LIBS = -lprotocol
TARGET = client objStore libprotocol.a
OBJS = protocol.o message.o object.o user.o list.o stats.o tests

.PHONY = clean all test


client : libprotocol.a
	$(CC) $(CFLAGS) $@.c -o $@ $^ -L. $(LIBS)

objStore : stats.o message.o common.h
	$(CC) $(CFLAGS) $@.c -o $@ $^ -lpthread -lm

protocol.o : protocol.h message.o common.h

user.o : user.h object.o common.h

tests : object.o user.o list.o message.o common.h
	$(CC) $(CFLAGS) $@.c -o $@ $^ -lm

%.o : %.c %.h

libprotocol.a : protocol.o message.o stats.o common.h
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
	$(RM) $(OBJS) $(TARGET) *~ *.sock

test : tests
#  @echo "Testing ... "
	valgrind ./$^
