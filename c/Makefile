CC=gcc
CFLAGS=-Wall -ansi -lMLV -g
LDFLAGS=
OBJ=voyageur.o

all: voyageur

voyageur: voyageur.o
	$(CC) -o voyageur voyageur.o $(LDFLAGS)

voyageur.o: voyageur.c voyageur.h
	$(CC) -c voyageur.c -o voyageur.o $(CFLAGS)

clean:
	rm -f *.o
	rm -f *~
	rm -f voyageur
