#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#include "trajet.h"
#include "matrix.h"

#define MAX_TRIALS 100

Trajet* getTrajet(BoardTrajet matrix_trajet, int i, int j) {
    return &matrix_trajet[i][j];
};

void shuffle(int* tab, int size, int nbShuffle) {
    for (int i = 0; i < nbShuffle-1; i++) {
        size_t index = rand() % (size-1);

        int temp = tab[index];
        tab[index] = tab[index+1];
        tab[index+1] = temp;
    }
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

void init_genome(int* id_pharma, int size) {

    for (int i = 0; i < size; i++) {
        id_pharma[i] = i;
    }

    shuffle(id_pharma, size, size*5);
}

void init_generation(int** generation, int size) {

    for (int i = 0; i < size; i++) {
        int* pharmas = (int*) malloc((NB_PHARMA-1)*sizeof(int));
        init_genome(pharmas, NB_PHARMA-1);
        generation[i] = pharmas;
    }
}

double calcul_fitness(int* genome, int size, BoardTrajet matrix_trajets){
    int i;
    double sum = 0;
    for(i=0; i<size-1; i++){
        Trajet* trajet = getTrajet(matrix_trajets, genome[i], genome[i+1]);
        sum+=trajet->distance;
    }
    return sum;
};

void sort_genomes(){
    
};

void print_10_genomes(BoardTrajet matrix_trajets) {
    int **generation = malloc(10*sizeof(int*));
    init_generation(generation, 10);
    for (int i = 0; i < 10; i++) {
        int* genome = generation[i];
        printf("Genome #%d :\n", i + 1);
        for (int i = 0; i < NB_PHARMA-1; ++i) {
            printf("%d ", genome[i]);
        }
        printf("\n");
        for (int k = 0; k < NB_PHARMA - 1 - 1; k++) { // -1 parce que 4 éléments et -1 parce que boucle 2 à 2 (+1 dans la boucle)
            print_trajets(getTrajet(matrix_trajets, genome[k], genome[k+1]));
        }
        printf("genome fitness : %.6f\n", calcul_fitness(genome, NB_PHARMA-1, matrix_trajets));

    }
}