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


all : $(TARGETS)

clean :
	@echo "Removing garbage"
	-rm $(OBJECTS) $(TARGETS)

test : 
	@echo "Eseguo i test ... "


#---------------
#fin qui funziona

# CC = gcc
# CFLAGS = -std=c99 -Wall -pedantic

# .PHONY = clean all test

# #nomi simbolici
# TARGETS = client objStore libprotocol.a
# EXE = client

# OBJECTS = protocol.o message.o

# #regole
# $(EXE) : libprotocol.a
# 		$(CC) $(CFLAGS) $(EXE).c -o $(EXE) -L. -lprotocol

# protocol.o : protocol.c protocol.h

# message.o : message.c message.h

# libprotocol.a : $(OBJECTS)
# 		ar rvs $@ $^

# all : $(TARGETS)

# clean :
# 		@echo "Removing garbage"
# 		-rm  $(EXE) $(OBJECTS) *.a

# test : 
# 		@echo "Eseguo i test ... "


# #-----------------------------
# #client compila

# CC = gcc
# CFLAGS = -std=c99 -Wall -pedantic

# .PHONY = clean

# #nomi simbolici
# EXE = client
# OBJECTS = protocol.o message.o

# #regole
# $(EXE) : $(EXE).c $(OBJECTS)

# protocol.o : protocol.c protocol.h

# message.o : message.c message.h

# clean :
# 		@echo "Removing garbage"
# 		-rm $(EXE) $(OBJECTS)
