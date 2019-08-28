CC          = gcc
CFLAGS      = -I. -std=c99 -Wall -g -pedantic
LDFLAGS     = -L.
LIBS        = -lprotocol -lm
TARGETS     = client objStore libprotocol.a
OBJS        = protocol.o message.o object.o user.o list.o stats.o #tests

.PHONY : clean all test


objStore : stats.o message.o list.o user.o object.o
	$(CC) $(CFLAGS) $@.c -o $@ $^ -lpthread -lm

client : libprotocol.a
	$(CC) $(CFLAGS) $@.c -o $@ $^ $(LDFLAGS) $(LIBS)

protocol.o : message.o

user.o : object.o

# tests : object.o user.o list.o message.o
# 	$(CC) $(CFLAGS) $@.c -o $@ $^ -lm

%.o : %.c %.h

libprotocol.a : protocol.o message.o common.h
	$(AR) rvs $@ $^

all : $(TARGETS)

clean :
#  @echo "Removing garbage"
	$(RM) $(OBJS) $(TARGETS) *~ *.sock

test :
#  @echo "Testing ... "
