#ifndef VOYAGEUR_H
#define VOYAGEUR_H
#define NB_PHARMA 5

typedef int Board[NB_PHARMA][NB_PHARMA];

typedef struct trajet {
    int ph1;
    int ph2;
    double distance;
    double duration;
} Trajet;

#endif
