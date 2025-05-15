
#include <stdio.h>

#include "voyageur.h"
#include "matrix.h"

/* print trajet matrix */
void print_matrix_trajet(BoardTrajet matrix_trajets) {
    printf("Matrice des trajets :\n");
    for (int i = 0; i < NB_PHARMA; i++) {
        for (int j = 0; j < NB_PHARMA; j++) {
            printf("%f ", (matrix_trajets[i][j]).duration);
        }
        printf("\n");
    }
}

/* Allow to initialize matrix of trajets */
void initialize_matrix_trajets(BoardTrajet matrix, const Trajet* trajets) {
    for (int i = 0 ; i < NB_PHARMA ; i++) {
        for (int j = 0 ; j < NB_PHARMA ; j++) {
            matrix[i][j] = trajets[i * NB_PHARMA + j];
        }
    }
};