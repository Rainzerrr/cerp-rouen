#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "trajet.h"
#include "matrix.h"
#include "genome.h"


#define MAX_TRIALS 100

// Matrice des trajets utilisés dans les fonctions de l'algorithme génétique.
static double **matrix_distance;
static double **matrix_duration;


Genome* allocate_genome(){
    Genome* genome = malloc(sizeof(Genome));
    genome->size = 0;
    genome->trajets = malloc((NB_PHARMA*2)*sizeof(int));
    genome->fitness = 0;

    return genome;

}

// Récupère le trajet entre deux genes donnés
double get_trajet_distance(int i, int j) {
    return matrix_distance[i][j];
};

// Récupère le trajet entre deux genes donnés
double get_trajet_duration(int i, int j) {
    return matrix_duration[i][j];
};


int count_trucks(int* tab, int size){ // Ajouter le nb de camions dans la struct
    int cpt = 1;
    for (int i = 0; i < size; i++)
    {
        if(tab[i] == -1){
            cpt++;
        }
    }
    return cpt;

}

void find_truck(int* tab, int size, int index, int* truck, int* size_truck){
    int position = 1;
    for (int i = 0; i < size; i++)
    {
        int j = 0;
        if(position >= index){
            while(tab[i+j] != -1){
                truck[j] = tab[i+j];
                (*size_truck)++;
                j++;
            }
            truck[j] = -1;
            (*size_truck)++;
            return;
        }

        if(tab[i] == -1){
            position++;
        }
    }

}

void random_mutation(Genome* genome) {
    int size = genome->size;
    int i = 0;

    size_t index1 = rand() % (size-1);
    size_t index2 = rand() % (size-1);


    while (index1 == index2 || genome->trajets[index1] == -1 || genome->trajets[index2] == -1) {
        index1 = rand() % (size-1);
        index2 = rand() % (size-1);
    }

    int temp = genome->trajets[index1];
    genome->trajets[index1] = genome->trajets[index2];
    genome->trajets[index2] = temp;

}

void trajet_mutation(Genome* genome) {
    int size = genome->size;
    int i = 0;
    while (i <= size) {
        int j = i;
        int cpt = 0;
        while (genome->trajets[j] != -1 && j < size) {
            cpt++;
            j++;
        }

        int index1 = j - (rand() % cpt)-1;
        int index2 = j - (rand() % cpt)-1;

        while (index1 == index2) {
            index1 = j - (rand() % cpt)-1;
            index2 = j - (rand() % cpt)-1;
        }


        int temp = genome->trajets[index1];
        genome->trajets[index1] = genome->trajets[index2];
        genome->trajets[index2] = temp;

        i = j+1;
    }

}

// Mélange la position des genes dans un genome
void shuffle(int* tab, int size, int nbShuffle) {
    for (int i = 0; i < nbShuffle-1; i++) {
        size_t index1 = rand() % (size-1);
        size_t index2 = rand() % (size-1);


        int temp = tab[index1];
        tab[index1] = tab[index2];
        tab[index2] = temp;
    }
}

void gen_pharma(int *pharmacies, int size)
{
    for (int i = 0; i < size; i++)
    {
        pharmacies[i] = i + 1;
    }
    shuffle(pharmacies, size, size * 10);
}

void init_genome(Genome* genome) {
    int nb_pharma = NB_PHARMA - 1;
    int *pharmacies = (int *)malloc(nb_pharma * sizeof(int));
    gen_pharma(pharmacies, nb_pharma);

    double duration = 0;
    int i = 0;

    while (i < nb_pharma) {

        int index = genome->size;

        if (duration == 0) {
            duration = get_trajet_duration(0, pharmacies[i]);
            genome->trajets[index] = pharmacies[i];
            // printf("Ajout de %d, durée actuelle = %.2f secondes\n", pharmacies[i], duration);
            i++;
        } else {
            int prev = pharmacies[i - 1];
            int curr = pharmacies[i];

            double d_prev_curr = get_trajet_duration(prev, curr);
            double d_curr_depot = get_trajet_duration(curr, 0);
            double total_duration = duration + d_prev_curr + d_curr_depot;

            if (total_duration > 10800.0) {
                // printf("\n--- Dépassement détecté ---\n");
                // printf("Pharmacie précédente : %d\n", prev);
                // printf("Pharmacie courante   : %d\n", curr);
                // printf("Trajet %d → %d : %.2f s\n", prev, curr, d_prev_curr);
                // printf("Trajet %d → dépôt : %.2f s\n", curr, d_curr_depot);
                // printf("Durée actuelle       : %.2f s\n", duration);
                // printf("Durée totale si ajouté : %.2f s (limite = 10800)\n", total_duration);
                // printf("---------------------------\n");

                genome->trajets[index] = -1;
                duration = 0;
            } else {
                genome->trajets[index] = curr;
                duration += d_prev_curr;
                // printf("Ajout de %d, durée actuelle = %.2f secondes\n", curr, duration);
                i++;
            }
        } 

        genome->size++;
    }

    free(pharmacies);
}



// Crée la première generation
void init_generation(Genome** generation, int size) {
    for (int i = 0; i < size; i++) {
        Genome *genome;
        genome = allocate_genome();
        init_genome(genome);
        generation[i] = genome;
    }
}

// Fonction qui calcule la distance totale parcourue par le chemin d'un genome
double calcul_fitness(Genome *genome) {
    double sum = 0;
    int i = 0;

    while (i < genome->size) {
        // Ignorer les -1 (au cas où il y en a plusieurs à la suite)
        while (i < genome->size && genome->trajets[i] == -1) {
            i++;
        }

        if (i >= genome->size) break;

        // Nouvelle tournée
        int start = i;

        // Ajoute distance du dépôt vers première pharmacie
        sum += get_trajet_distance(0, genome->trajets[start]);

        // Avancer jusqu’au prochain -1 ou fin
        while (i < genome->size && genome->trajets[i] != -1) {
            if (i + 1 < genome->size && genome->trajets[i + 1] != -1) {
                sum += get_trajet_distance(genome->trajets[i], genome->trajets[i + 1]);
            }
            i++;
        }

        sum += get_trajet_distance(genome->trajets[i - 1], 0);
    }

    return sum;
}

// Fonction de comparaison appelée par qsort afin de trier les genomes d'une génération
int compare_genomes(const void *a, const void *b) {
    Genome *genome_a = *(Genome **)a;
    Genome *genome_b = *(Genome **)b;

    double fitness_a = calcul_fitness(genome_a);
    double fitness_b = calcul_fitness(genome_b);

    if (fitness_a < fitness_b) return -1;
    if (fitness_a > fitness_b) return 1;
    return 0;
}


void sort_genomes(Genome **generation, int population_size) {
    qsort(generation, population_size, sizeof(Genome*), compare_genomes);
}

// Vérifier si un gene est deja présent dans un genome
int is_in_genome(int *child, int size, int value) {
    for (int i = 0; i < size; i++) {
        if (child[i] == value) {
            return 1;
        }
    }
    return 0;
}

// Genome* crossover(Genome* first, Genome* second){

//     int size_genome1 = first->size;
//     int size_genome2 = second->size;

//     Genome* new_genome = allocate_genome();

//     int nb_trucks = count_trucks(first->trajets, size_genome1);
//     int index_first_genome = rand()%nb_trucks;
    

//     int size_truck = 0;
//     int* truck = malloc(30*sizeof(int));
//     find_truck(first->trajets, size_genome1, index_first_genome, truck, &size_truck);



//     int nb_pharma_in_new_genome = 0;
//     int i = 0;
//     for(i = 0; i < size_truck; i++){
//         int id_pharma = truck[i];
//         new_genome->trajets[i] = id_pharma;
//         if(id_pharma != -1){
//             nb_pharma_in_new_genome++;
//         }
//         new_genome->size++;
//     }

//     int j = 0;  
//     while(j < size_genome2 && nb_pharma_in_new_genome < NB_PHARMA){
//         int id_pharma = second->trajets[j];
//         int size_new_genome = new_genome->size;

        
//         id_pharma = second->trajets[j];
//         int last = new_genome->trajets[size_new_genome-1];

//         if(id_pharma == -1 && last != -1){
//             new_genome->trajets[size_new_genome] = id_pharma;
//             new_genome->size++;
//         }
//         else if(!is_in_genome(new_genome->trajets, new_genome->size, id_pharma)){
//             new_genome->trajets[size_new_genome] = id_pharma;
//             new_genome->size++;
//             nb_pharma_in_new_genome++;
//         }
//         j++;
//     }

//     free(truck);
//     return new_genome;

// }

Genome* crossover(Genome* first, Genome* second) {
    int size_genome1 = first->size;
    int size_genome2 = second->size;

    Genome* new_genome = allocate_genome();
    int* used = calloc(NB_PHARMA, sizeof(int));

    int nb_pharma_in_new_genome = 0;

    // 🔁 Copie linéaire de pharmacies du parent 1 (sans trop de -1)
    for (int i = 0; i < size_genome1 && nb_pharma_in_new_genome < NB_PHARMA; i++) {
        int id = first->trajets[i];

        if (id == -1) {
            // On évite d'ajouter trop de -1
            if (new_genome->size > 0 && new_genome->trajets[new_genome->size - 1] != -1)
                new_genome->trajets[new_genome->size++] = -1;
        } else if (!used[id]) {
            new_genome->trajets[new_genome->size++] = id;
            used[id] = 1;
            nb_pharma_in_new_genome++;
        }
    }

    // 🧩 On complète à partir du parent 2
    for (int i = 0; i < size_genome2 && nb_pharma_in_new_genome < NB_PHARMA; i++) {
        int id = second->trajets[i];

        if (id == -1) {
            // Même logique : pas de doublon de -1
            if (new_genome->size > 0 && new_genome->trajets[new_genome->size - 1] != -1)
                new_genome->trajets[new_genome->size++] = -1;
        } else if (!used[id]) {
            new_genome->trajets[new_genome->size++] = id;
            used[id] = 1;
            nb_pharma_in_new_genome++;
        }
    }

    // ✅ On vérifie si tout est là (fail-safe)
    for (int i = 0; i < NB_PHARMA; i++) {
        if (!used[i]) {
            if (new_genome->size > 0 && new_genome->trajets[new_genome->size - 1] != -1)
                new_genome->trajets[new_genome->size++] = -1;
            new_genome->trajets[new_genome->size++] = i;
        }
    }

    // ❌ Nettoie les -1 de fin s’il y en a trop
    while (new_genome->size > 1 &&
           new_genome->trajets[new_genome->size - 1] == -1 &&
           new_genome->trajets[new_genome->size - 2] == -1) {
        new_genome->size--;
    }

    free(used);
    return new_genome;
}


// Echange deux gènes dans le genome
void swap_genes(int *genome, int i, int j) {
    int temp = genome[i];
    genome[i] = genome[j];
    genome[j] = temp;
}

Genome* duplicate_genome(Genome *original) {
    Genome *copy = malloc(sizeof(Genome));
    copy->size = original->size;
    copy->fitness = original->fitness;

    copy->trajets = malloc(copy->size * sizeof(int));
    memcpy(copy->trajets, original->trajets, copy->size * sizeof(int));

    return copy;
}

void free_population(Genome **population, int size) {
    for (int i = 0; i < size; i++) {
        if (population[i]) {
            free(population[i]->trajets);
            free(population[i]);
        }
    }
    free(population);
}

// Affiche le meilleur genome d'une génération
void print_best_genome(Genome **generation, int gen_number) {
    printf("\n=== Generation %d ===\n", gen_number);

    int best_idx = 0;
    double best_fitness = calcul_fitness(generation[0]);

    printf("\n=== Meilleur resultat de la generation %d ===\n", gen_number);
    printf("IDX: %d | Fitness: %.2f\n", best_idx, best_fitness);
    printf("Flotte de camions :\n");

    Genome *best_genome = generation[best_idx];

    int camion = 1;
    printf("Camion %d : 0 -> ", camion);

    for (int i = 0; i < best_genome->size; i++) {
        if (best_genome->trajets[i] == -1) {
            printf("0\n");
            camion++;
            if (i + 1 < best_genome->size) {
                printf("Camion %d : 0 -> ", camion);
            }
        } else {
            printf("%d -> ", best_genome->trajets[i]);
        }
    }

    printf("0\n"); // Fin du dernier trajet
}

void launch_genetic(double **matrix_trajets, double** matrix_durations, int duration_seconds, int population_size) {
    matrix_distance = copy_board_trajet(matrix_trajets);
    matrix_duration = copy_board_trajet(matrix_durations);
    Genome **population = malloc(population_size * sizeof(Genome*));
    init_generation(population, population_size);
    
    printf("=== DEBUT ALGORITHME GENETIQUE ===\n");
    printf("Pharmacies: %d | Taille population: %d | Duree max: %d secondes\n", 
           NB_PHARMA - 1, population_size, duration_seconds);

    clock_t start_time = clock();
    int gen = 1;
    double elapsed_seconds = 0;
    double best_fitness = 999999999.0;
    Genome* best_genome;
    
    while (elapsed_seconds < duration_seconds) {
        sort_genomes(population, population_size);
        
        double current_best_fitness = calcul_fitness(population[0]);

        if (current_best_fitness < best_fitness) {
            print_best_genome(population, gen);
            best_fitness = current_best_fitness;
            best_genome = duplicate_genome(population[0]);
            }

        Genome **new_pop = malloc(population_size * sizeof(int*));

        // 1. Élitisme : 10% meilleurs
        int elite_count = (int)(population_size * 0.1);;
        for (int i = 0; i < elite_count; i++) {
            new_pop[i] = duplicate_genome(population[i]);
        }

         // 2. Croisement : 80% de la population
        int crossover_start = elite_count;
        int crossover_end = crossover_start + (int)(population_size * 0.8);
        int top_40_percent = (int)(population_size * 0.4);

        for (int i = crossover_start; i < crossover_end; i++) {
            int p1 = rand() % top_40_percent;
            int p2 = rand() % top_40_percent;
            while (p2 == p1) p2 = rand() % top_40_percent;

            new_pop[i] = crossover(population[p1], population[p2]);

            // Mutation avec une probabilité de 10%
            if (rand() % 100 < 10) {
                random_mutation(new_pop[i]);
            }
        }

        // 2. Génomes aléatoires : 10% de la population
        for (int i = crossover_end; i < population_size; i++) {
            Genome *genome;
            genome = allocate_genome();
            init_genome(genome);
            new_pop[i] = genome;
        }

        free_population(population, population_size);
        population = new_pop;
        gen++;

        elapsed_seconds = (double)(clock() - start_time) / CLOCKS_PER_SEC;
    }

    printf("\n=== RESULTAT FINAL (Generation %d, %.2f secondes) ===\n", gen - 1, elapsed_seconds);
    sort_genomes(population, population_size);
    print_best_genome(population, gen - 1);
    
    free_population(population, population_size);
}

void test(double **matrix_trajets, double** matrix_durations){
    matrix_distance = copy_board_trajet(matrix_trajets);
    matrix_duration = copy_board_trajet(matrix_durations);

    //Genome* genome = allocate_genome();

    // init_genome(genome);
    // print_genome(genome->trajets, genome->size);

    // printf("\n\n");

    // random_mutation(genome);
    // print_genome(genome->trajets, genome->size);

    Genome* genome1 = allocate_genome();
    Genome* genome2 = allocate_genome();
    init_genome(genome1);
    init_genome(genome2);

    printf("Genome 1 : \n");
    print_genome(genome1->trajets, genome1->size);

    printf("\n\n");

    printf("Genome 2 : \n");
    print_genome(genome2->trajets, genome2->size);
    printf("\n\n");

    Genome* new_genome = crossover(genome1, genome2);

    printf("new genome : \n");
    print_genome(new_genome->trajets, new_genome->size);
}