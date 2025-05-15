#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "trajet.h"
#include "genome.h"
#include "matrix.h"

#define MAX_LINE_LENGTH 1024



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
    if (!file) {
        perror("Erreur lors de l'ouverture du fichier CSV");
        return 1;
    }

    Trajet trajets[NB_PHARMA*NB_PHARMA];
    parse_csv_trajet(file, trajets);
    fclose(file);


    BoardDuration matrix_duration;
    initialize_matrix_duration(matrix_duration, trajets);
    print_matrix_duration(matrix_duration);

    BoardTrajet matrix_trajets;
    initialize_matrix_trajets(matrix_trajets, trajets);
    print_matrix_trajet(matrix_trajets);




    srand(time(NULL));
    Trajet trajetsGenome[] = {
        {1, 2, 3200.5, 420.3},
        {1, 3, 8500.1, 780.2},
        {1, 4, 4500.2, 500.1},

        {2, 1, 3200.5, 420.3},
        {2, 3, 2600.8, 300.4},
        {2, 4, 9000.4, 810.5},

        {3, 1, 8500.1, 780.2},
        {3, 2, 2600.8, 300.4},
        {3, 4, 7000.9, 650.2},

        {4, 1, 4500.2, 500.1},
        {4, 2, 9000.4, 810.5},
        {4, 3, 7000.9, 650.2}
    };

    print_10_genomes(trajetsGenome);

    return 0;
}