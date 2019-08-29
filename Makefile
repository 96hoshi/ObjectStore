CC			= gcc
CFLAGS		= -std=c99 -Wall -g -pedantic -I.
LDFLAGS		= -L.
LIBS		= -lprotocol -lm
TARGETS		= client objStore libprotocol.a
OBJS		= protocol.o message.o object.o user.o list.o stats.o worker.o

.PHONY: all test clean


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


all: $(TARGETS)


test: objStore client
	@./test_clients.sh


clean:
	$(RM) $(OBJS) $(TARGETS) *~ *.sock