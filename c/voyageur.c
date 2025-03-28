#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "voyageur.h"

#define MAX_LINE_LENGTH 1024

void initialize_matrix(Board matrix, Trajet* trajets) {
    int i;
    for (i = 0 ; i < NB_PHARMA * NB_PHARMA ; i++) {
        matrix[trajets[i].ph1][trajets[i].ph2] = trajets[i];
        matrix[trajets[i].ph2][trajets[i].ph1] = trajets[i];
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
                case 0:
                    trajets->ph1 = atoi(token);
                    break;
                case 2:
                    trajets->ph2 = atoi(token);
                    break;
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
    int i;
    for (i = 0; i < nb_lines; i++) {
        printf("%d %d %f %f\n", trajets[i].ph1, trajets[i].ph2, trajets[i].distance, trajets[i].duration);
    }
    Board matrix;
    initialize_matrix(matrix, trajets);
    free(trajets);
    return 0;
}