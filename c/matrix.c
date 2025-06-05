
#include <stdio.h>
#include <stdlib.h>
#include "trajet.h"
#include "matrix.h"

/* print duration matrix */
void print_matrix_duration(BoardDuration matrix_duration) {
    printf("Matrice des distances :\n");
    for (int i = 0; i < NB_PHARMA; i++) {
        for (int j = 0; j < NB_PHARMA; j++) {
            printf("%8.1f ", matrix_duration[i][j]);
        }
        printf("\n");
    }
}

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

/* Allow to initilize matrix duration */
void initialize_matrix_duration(BoardDuration matrix, Trajet* trajets) {
    for (int i = 0 ; i < NB_PHARMA ; i++) {
        for (int j = 0 ; j < NB_PHARMA ; j++) {
            matrix[i][j] = trajets[i * NB_PHARMA + j].distance;
        }
    }
};

/* Allow to initialize matrix of trajets */
void initialize_matrix_trajets(BoardTrajet matrix, const Trajet* trajets) {
    for (int i = 0 ; i < NB_PHARMA ; i++) {
        for (int j = 0 ; j < NB_PHARMA ; j++) {
            matrix[i][j] = trajets[i * NB_PHARMA + j];
        }
    }
};

double **copy_board_trajet(double **src) {
    double **dest = malloc(NB_PHARMA * sizeof(double *));
    if (!dest) return NULL;

    for (int i = 0; i < NB_PHARMA; i++) {
        dest[i] = malloc(NB_PHARMA * sizeof(double));
        if (!dest[i]) {
            for (int k = 0; k < i; k++) {
                free(dest[k]);
            }
            free(dest);
            return NULL;
        }

        for (int j = 0; j < NB_PHARMA; j++) {
            dest[i][j] = src[i][j];
        }
    }

    return dest;
}

void print_tab(int* tab, int size)
{
    for (int i = 0; i < size; i++)
    {
        printf("%d ", tab[i]);
    }
    printf("\n");
}

void print_genome(int* tab, int size)
{
    for (int i = 0; i < size; i++)
    {
        if(tab[i] == -1){
            printf(" | ");

        }else{
            printf("%d ", tab[i]);
        }
    }
    printf("\n");
}