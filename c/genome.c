#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#include "voyageur.h"

void print_trajet(Trajet* trajet){
    printf("ph1 : %d | ph2 : %d | distance : %f | durée : %f\n", trajet->ph1, trajet->ph2, trajet->distance, trajet->duration);
}

int is_in_genome(Trajet* genome, int size, Trajet* t){
    print_trajet(t);
    int i;
    for(i = 0; i < size; i++){
        int genome1 = genome[i].ph1;
        int genome2 = genome[i].ph2;

        printf("genome 1 : %d | genome 2 : %d\n", genome1, genome2);

        if(genome1 == t->ph1 && genome2 == t->ph2){
            return 1;
        }
    }
    return 0;
}

Trajet* init_genome(Trajet* trajets){
    int cpt = 0;
    Trajet* genome = malloc((NB_PHARMA-1) * sizeof(Trajet));
    srand(time(NULL));
    while(cpt < NB_PHARMA-1){

        int alea = rand()%NB_PHARMA;
        Trajet t = trajets[alea];

        printf("%d\n", is_in_genome(genome, cpt, &t));
        if(!is_in_genome(genome, cpt, &t)){
            printf("while if\n");
            cpt++;
            genome[cpt] = t;
        }
    }
    return genome;
};

void value_genome(){
    
};

void sort_genome(){
    
};