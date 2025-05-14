CC=gcc
CFLAGS=-Wall -ansi -lMLV -g
LDFLAGS=
OBJ=voyageur.o genome.o

all: voyageur

voyageur: ${OBJ}
	$(CC) -o voyageur ${OBJ} $(LDFLAGS)

voyageur.o: voyageur.c voyageur.h
	$(CC) -c voyageur.c -o voyageur.o $(CFLAGS)

genome.o: genome.c genome.h
	$(CC) -c genome.c -o $(CFLAGS)

clean:
	rm -f *.o
	rm -f *~
	rm -f voyageur
