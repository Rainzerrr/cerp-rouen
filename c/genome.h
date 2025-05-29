#ifndef GENOME_H
#define GENOME_H
#include "trajet.h"
#include "matrix.h"

void print_10_genomes(BoardTrajet trajets);

void launch_genetic(BoardTrajet matrix_trajets, int duration, int population_size);
#endif // GENOME_H
