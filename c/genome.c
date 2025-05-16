#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#include "trajet.h"
#include "matrix.h"

#define MAX_TRIALS 100


int is_in_genome(Trajet* genome, int size, Trajet* t) {
    for (int i = 0; i < size; i++) {
        int g1 = genome[i].ph1;
        int g2 = genome[i].ph2;
        if(t->ph2 == g2 || t->ph1 == g1){
            return 1;
        }
        if(t->ph2 == g1 && t->ph1 == g2){
            return 1;
        }
    }
    return 0;
}

Trajet* init_genome(BoardTrajet matrix_trajets) {



    Trajet last_trajet;


    return &last_trajet;
}

double calcul_fitness(Trajet* genome, int size){
    int i;
    double sum = 0;
    for(i=0; i<size; i++){
        sum += genome[i].distance;
    }
    return sum;
};

void sort_genomes(){
    
};

void print_10_genomes(BoardTrajet trajets) {
    for (int i = 0; i < 10; i++) {
        Trajet* genome = NULL;

        // Tenter de générer un genome valide, relancer si NULL
        do {
            genome = init_genome(trajets);
        } while (genome == NULL);

        printf("Genome #%d :\n", i + 1);
        for (int k = 0; k < NB_PHARMA - 1; k++) {
            print_trajets(&(genome[k]));
        }
        printf("genome fitness : %.6f\n", calcul_fitness(genome, NB_PHARMA -1));

        free(genome);
    }
}