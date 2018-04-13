CC = gcc
CFLAGS = -Wall -std=c11 -g
LIB = sharedLib.so

#UNAME Shell Variable
UNAME_S := $(shell uname -s)

# OS X Dependant Variables
ifeq ($(UNAME_S), Darwin)
    LIB := sharedLib.dylib
# GNU/Linux Depedant Variables
else ifeq ($(UNAME_S), Linux)
    LIB := ../sharedLib.so
endif

$(LIB): GEDCOMparser.o LinkedListAPI.o
	$(CC) -shared -o $(LIB) GEDCOMparser.o LinkedListAPI.o

GEDCOMparser.o: src/GEDCOMparser.c include/GEDCOMparser.h include/GEDCOMutilities.h
	$(CC) $(CFLAGS) -c -fpic src/GEDCOMparser.c -Iinclude

LinkedListAPI.o: src/LinkedListAPI.c include/LinkedListAPI.h
	$(CC) $(CFLAGS) -c -fpic src/LinkedListAPI.c -Iinclude

clean:
	rm $(LIB) *.o