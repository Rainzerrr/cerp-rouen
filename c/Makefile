CC=gcc
CFLAGS=
LDFLAGS=
OBJ=voyageur.o genome.o matrix.o

all: voyageur

voyageur: ${OBJ}
	$(CC) -o voyageur ${OBJ} $(LDFLAGS)

voyageur.o: voyageur.c voyageur.h
	$(CC) -c voyageur.c -o voyageur.o $(CFLAGS)

genome.o: genome.c genome.h
	$(CC) -c genome.c -o genome.o $(CFLAGS)

matrix.o: matrix.c matrix.h
	$(CC) -c matrix.c -o matrix.o $(CFLAGS)
clean:
	rm -f *.o
	rm -f *~
	rm -f voyageur
