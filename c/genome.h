#ifndef GENOME_H
#define GENOME_H
#include "trajet.h"
#include "matrix.h"

void print_10_genomes(int **trajets);

double calcul_fitness(int* genome, int size);

void launch_genetic(double **matrix_trajets, int duration, int population_size);
#endif // GENOME_H
