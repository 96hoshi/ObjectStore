CC			= gcc
CFLAGS		= -std=c99 -Wall -g -pedantic -I.
LDFLAGS		= -L.
LIBS		= -lprotocol -lm
TARGETS		= objStore libprotocol.a client
OBJS		= protocol.o message.o object.o user.o list.o stats.o worker.o
SOCKNAME	= objstore.sock
LOG			= testout.log
DIR			= ./data

.PHONY: all test clean


all: $(TARGETS)


objStore: stats.o message.o list.o user.o object.o worker.o
	$(CC) $(CFLAGS) $@.c -o $@ $^ -lpthread -lm


client: libprotocol.a
	$(CC) $(CFLAGS) $@.c -o $@ $^ $(LDFLAGS) $(LIBS)


worker.o: stats.o message.o list.o user.o object.o


protocol.o: message.o


user.o: object.o


%.o: %.c %.h


libprotocol.a: protocol.o message.o common.h
	$(AR) rvs $@ $^


test: objStore client
	@./test_clients.sh


clean:
	$(RM) $(TARGETS) $(OBJS) $(SOCKNAME) ${LOG} *~
	$(RM) -r ${DIR}
