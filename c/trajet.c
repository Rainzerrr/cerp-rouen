#include <stdio.h>

#include <trajet.h>

/* print struct trajet */
void print_trajets(Trajet* trajet){
    printf("ph1 : %d | ph2 : %d | distance : %f | durée : %f\n", trajet->ph1, trajet->ph2, trajet->distance, trajet->duration);
}
