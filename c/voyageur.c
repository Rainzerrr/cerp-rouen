#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "voyageur.h"
#include "genome.h"

#define MAX_LINE_LENGTH 1024

void initialize_matrix(Board matrix, Trajet* trajets) {
    int i, j;
    for (i = 0 ; i < NB_PHARMA ; i++) {
        for (j = 0 ; j < NB_PHARMA ; j++) {
            matrix[i][j] = trajets[i * NB_PHARMA + j].distance;
        }
    }
};

void parse_csv_trajet(FILE* file, Trajet* trajets) {
    char line[MAX_LINE_LENGTH];
    fgets(line, sizeof(line), file);
    while (fgets(line, sizeof(line), file)) {
        char* token = strtok(line, ",");
        int col = 0;
        while (token != NULL) {
            switch (col) {
                case 4:
                    trajets->distance = atof(token);
                    break;
                case 5:
                    trajets->duration = atof(token);
                    break;
            }
            token = strtok(NULL, ",");
            col++;
        }
        trajets++;
    }
}



int main(int argc, char* argv[]) {
    FILE *file = fopen("../js/data/distances.csv", "r");
    int nb_lines = NB_PHARMA * NB_PHARMA;
    Trajet* trajets = malloc(nb_lines * sizeof(Trajet));
    parse_csv_trajet(file, trajets);
    fclose(file);
    int i, j;
    Board matrix;
    initialize_matrix(matrix, trajets);
    for (i = 0; i < NB_PHARMA; i++) {
        for (j = 0; j < NB_PHARMA; j++) {
            printf("%f ", matrix[i][j]);
        }
        printf("\n");
    }

    init_genome();
    value_genome();
    sort_genome();

    free(trajets);
    return 0;
}