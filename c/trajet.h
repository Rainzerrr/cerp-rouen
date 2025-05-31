

#ifndef TRAJET_H
#define TRAJET_H

#define NB_PHARMA 21

typedef struct trajet {
    int ph1;
    int ph2;
    double distance;
    double duration;
} Trajet;

void print_trajets(Trajet* trajet);

#endif //TRAJET_H
