#ifndef MATRIX_H
#define MATRIX_H

#endif //MATRIX_H


typedef float BoardDuration[NB_PHARMA][NB_PHARMA];
typedef Trajet BoardTrajet[NB_PHARMA][NB_PHARMA];

void print_matrix_duration(BoardDuration matrix_duration);
void print_matrix_trajet(BoardTrajet matrix_trajets);

void initialize_matrix_duration(BoardDuration matrix, Trajet* trajets);
void initialize_matrix_trajets(BoardTrajet matrix, const Trajet* trajets);