#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "voyageur.h"

#define MAX_LINE_LENGTH 1024

void initialize_matrix(Trajet* trajets) {

};

Trajet* allocate_trajet(int ph1, int ph2, double distance, double duration) {
    Trajet* trajt = malloc(sizeof(Trajet));
    trajt->ph1 = ph1;
    trajt->ph2 = ph2;
    trajt->distance = distance;
    trajt->duration = duration;
    return trajt;
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
    int nb_lines = 11;
    Trajet* trajets = malloc(nb_lines * sizeof(Trajet));
    parse_csv_trajet(file, trajets);
    fclose(file);
    int i;
    for (i = 0; i < nb_lines - 1; i++) {
        printf("%d %d %f %f\n", trajets[i].ph1, trajets[i].ph2, trajets[i].distance, trajets[i].duration);
    }
    free(trajets);
    return 0;
}