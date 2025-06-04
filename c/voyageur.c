#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "trajet.h"
#include "genome.h"
#include "matrix.h"

#define MAX_LINE_LENGTH 1500



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

double **allouer_matrice(int n) {
    double **matrice = malloc(n * sizeof(double *));
    if (!matrice) return NULL;

    for (int i = 0; i < n; ++i) {
        matrice[i] = malloc(n * sizeof(double));
        if (!matrice[i]) {
            // Libération partielle en cas d'échec
            for (int j = 0; j < i; ++j)
                free(matrice[j]);
            free(matrice);
            return NULL;
        }
    }

    return matrice;
}

/**
 * Libère une matrice allouée dynamiquement.
 */
void liberer_matrice(double **matrice, int n) {
    for (int i = 0; i < n; ++i)
        free(matrice[i]);
    free(matrice);
}

int load_matrix_distances(const char *nom_fichier, double **distances, int n) {
    FILE *file = fopen(nom_fichier, "r");
    if (!file) {
        perror("Erreur d'ouverture du fichier");
        return 1;
    }

    char line[MAX_LINE_LENGTH];
    int row = 0;

    fgets(line, sizeof(line), file);

    while (row < n && fgets(line, sizeof(line), file)) {
        if (strlen(line) <= 1) continue;
        
        if (row == 0){
            row++;
            continue;
        }

        char *token = strtok(line, ",");  // Skip the name of the pharmacy
        int col = 0;

        while ((token = strtok(NULL, ",")) != NULL && col < n) {
            while (*token == ' ') token++;  // Trim start
            distances[row-1][col++] = atof(token);
        }

        row++;
    }

    fclose(file);
    return 0;
}

int load_matrix_durations(const char *nom_fichier, double **distances, int n) {
    FILE *file = fopen(nom_fichier, "r");
    if (!file) {
        perror("Erreur d'ouverture du fichier");
        return 1;
    }

    char line[MAX_LINE_LENGTH];
    int row = 0;

    fgets(line, sizeof(line), file);

    while (row < n && fgets(line, sizeof(line), file)) {
        if (strlen(line) <= 1) continue;
        
        if (row == 0){
            row++;
            continue;
        }

        char *token = strtok(line, ",");  // Skip the name of the pharmacy
        int col = 0;

        while ((token = strtok(NULL, ",")) != NULL && col < n) {
            while (*token == ' ') token++;  // Trim start
            distances[row-1][col++] = atof(token);
        }

        row++;
    }

    fclose(file);
    return 0;
}

int main(int argc, char* argv[]) {
    double **distances = allouer_matrice(NB_PHARMA);
    double **durations = allouer_matrice(NB_PHARMA);

    int duration = 60;
    int nb_pop = 100;

    if(argc > 1) {
        duration = atoi(argv[1]);
    }
    if(argc > 2) {
        nb_pop = atoi(argv[2]);
    }

    srand(time(NULL));


    if (!distances) {
        fprintf(stderr, "Erreur d'allocation mémoire\n");
        return 1;
    }

    if (load_matrix_distances("../js/data/distances_pharmacies.csv", distances, NB_PHARMA +1) != 0) {
        fprintf(stderr, "Erreur de chargement de la matrice\n");
        liberer_matrice(distances, NB_PHARMA);
        return 1;
    }

    if (load_matrix_distances("../js/data/durations_pharmacies.csv", durations, NB_PHARMA +1) != 0) {
        fprintf(stderr, "Erreur de chargement de la matrice\n");
        liberer_matrice(distances, NB_PHARMA);
        return 1;
    }

    // Exemple d’utilisation
    printf("Distance entre [0][0] = %.2f\n", distances[0][0]);
    printf("Distance entre [0][1] = %.2f\n", distances[0][1]);
    printf("Distance entre [0][2] = %.2f\n", distances[0][2]);
    printf("Distance entre [0][3] = %.2f\n", distances[0][3]);

    // Exemple d’utilisation
    printf("Duree entre [0][0] = %.2f\n", durations[0][0]);
    printf("Duree entre [0][1] = %.2f\n", durations[0][1]);
    printf("Duree entre [0][2] = %.2f\n", durations[0][2]);
    printf("Duree entre [0][3] = %.2f\n", durations[83][82]);

    launch_genetic(distances, durations, duration, nb_pop);

    // Libération mémoire
    liberer_matrice(distances, NB_PHARMA);
    liberer_matrice(durations, NB_PHARMA);

    return 0;
}