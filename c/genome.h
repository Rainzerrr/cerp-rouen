#ifndef GENOME_H
#define GENOME_H
#include "trajet.h"
#include "matrix.h"
Trajet* init_genome(Trajet* trajets);

double calcul_fitness(Trajet* genome, int size);

void sort_genomes();

void print_10_genomes(BoardTrajet trajets);
#endif // GENOME_H
