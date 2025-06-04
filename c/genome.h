#ifndef GENOME_H
#define GENOME_H
#include "trajet.h"
#include "matrix.h"

typedef struct genome { // Flotte de camions
    int* trajets;
    int size;
    double fitness;
} Genome;

void test(double **matrix_trajets, double** matrix_durations);

void launch_genetic(double **matrix_trajets, double** matrix_durations, int duration, int population_size);
#endif // GENOME_H
