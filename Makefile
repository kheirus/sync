#cible particulière 
.PHONY : clean


CFLAGS = -Wall -pedantic

all : sync.o functions.o 
	gcc sync.o functions.o -o synchroniser $(CFLAGS)

sync.o : sync.c functions.h
	gcc -c sync.c -o sync.o $(CFLAGS)

functions.o : functions.c
	gcc -c functions.c -o functions.o $(CFLAGS)

clean : 
	rm -rf *.o synchroniser
