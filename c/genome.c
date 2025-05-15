#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#include "voyageur.h"


#define MAX_TRIALS 100

void print_trajet(Trajet* trajet){
    printf("ph1 : %d | ph2 : %d | distance : %f | durée : %f\n", trajet->ph1, trajet->ph2, trajet->distance, trajet->duration);
}

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


Trajet* init_genome(Trajet* trajets) {
    int cpt = 0;
    int trials = 0;
    Trajet* genome = malloc((NB_PHARMA - 1) * sizeof(Trajet));
    
    if (!genome) {
        fprintf(stderr, "Erreur d'allocation mémoire\n");
        exit(EXIT_FAILURE);
    }


    while (cpt < NB_PHARMA - 1) {
        if (trials > MAX_TRIALS) {
            free(genome);
            return NULL;
        }
        
        int alea = (rand() % 11) +1;
        Trajet t = trajets[alea];
        
        if (!is_in_genome(genome, cpt, &t)) {
            genome[cpt] = t;
            cpt++;
        }
        
        trials++;
    }

    return genome;
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