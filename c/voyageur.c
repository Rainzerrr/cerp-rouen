#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "voyageur.h"
#include "genome.h"
#include "matrix.h"

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

void print_10_genomes(Trajet* trajets) {
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



int main(int argc, char* argv[]) {
    
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
    
    // Décommenter dès que la génération de la matrice ne bug plus
    Trajet trajets[NB_PHARMA*NB_PHARMA];
    FILE *file = fopen("../js/data/distances.csv", "r");
    if (!file) {
        perror("Erreur lors de l'ouverture du fichier CSV");
        return 1;
    }
    parse_csv_trajet(file, trajets);
    fclose(file);


    Board matrix;
    initialize_matrix(matrix, trajets);

    BoardTrajet matrix_trajets;
    initialize_matrix_trajets(matrix_trajets, trajets);
    printf("Matrice des distances :\n");
    for (int i = 0; i < NB_PHARMA; i++) {
        for (int j = 0; j < NB_PHARMA; j++) {
            printf("%8.1f ", matrix[i][j]);
        }
        printf("\n");
    }

    printf("\n");
    print_matrix_trajet(matrix_trajets);

    print_10_genomes(trajetsGenome);

    return 0;
}