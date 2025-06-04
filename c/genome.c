#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "trajet.h"
#include "matrix.h"

#define MAX_TRIALS 100

// Matrice des trajets utilisés dans les fonctions de l'algorithme génétique.
static double **g_matrix;

// Récupère le trajet entre deux genes donnés
double get_trajet_distance(int i, int j) {
    return g_matrix[i][j];
};

// Mélange la position des genes dans un genome
void shuffle(int* tab, int size, int nbShuffle) {
    for (int i = 0; i < nbShuffle-1; i++) {
        size_t index = rand() % (size-1);

        int temp = tab[index];
        tab[index] = tab[index+1];
        tab[index+1] = temp;
    }
}

// Crée un génome aléatoire
void init_genome(int* id_pharma, int size) {
    for (int i = 0; i < size; i++) {
        id_pharma[i] = i+1;
    }
    shuffle(id_pharma, size, size*5);
}

// Crée la première generation
void init_generation(int** generation, int size) {
    for (int i = 0; i < size; i++) {
        int* pharmas = (int*) malloc((NB_PHARMA-1)*sizeof(int));
        init_genome(pharmas, NB_PHARMA-1);
        generation[i] = pharmas;
    }
}

// Fonction qui calcule la distance totale parcourue par le chemin d'un genome
double calcul_fitness(int* genome, int size){
    int i;
    double sum = 0;
    
    for(i=0; i<size-1; i++){
        double distance = get_trajet_distance(genome[i], genome[i+1]);
        sum+=distance;
    }

    // Ajout du trajet entre le depot avec la première pharmacie
    sum += get_trajet_distance(0, genome[0]);

    // Ajout du trajet entre la dernière pharmacie et le depot
    sum += get_trajet_distance(genome[size-1], 0);

    return sum;
};

// Fonction de comparaison appelée par qsort afin de trier les genomes d'une génération
int compare_genomes(const void *a, const void *b) {
    int **genome_a = (int **)a;
    int **genome_b = (int **)b;
    
    double fitness_a = calcul_fitness(*genome_a, NB_PHARMA-1);

    double fitness_b = calcul_fitness(*genome_b, NB_PHARMA-1);    
    
    if (fitness_a < fitness_b) return -1;
    if (fitness_a > fitness_b) return 1;
    return 0;
}

// Fonction de tri des genomes dans une génération
void sort_genomes(int **generation, int population_size) {
    qsort(generation, population_size, sizeof(int*), compare_genomes);
}

// Vérifier si un gene est deja présent dans un genome
int is_present(int *child, int size, int value) {
    for (int i = 0; i < size; i++) {
        if (child[i] == value) {
            return 1;
        }
    }
    return 0;
}


// Renvoie un nouveau génome à partir de deux parents, suivant l'algorithme OX (Order Crossover)
int* crossover(int *parent1, int *parent2, int size) {
    int *child = malloc(size * sizeof(int));
    for (int i = 0; i < size; i++) child[i] = -1;

    // Choisir 2 points de coupe aléatoires
    int start = rand() % size;
    int end = rand() % size;
    if (start > end) { int tmp = start; start = end; end = tmp; }

    // Copier une sous-séquence de parent1 dans child
    for (int i = start; i <= end; i++) {
        child[i] = parent1[i];
    }

    // Compléter avec les gènes de parent2
    int c_idx = (end + 1) % size;
    int p2_idx = (end + 1) % size;
    while (c_idx != start) {
        int gene = parent2[p2_idx];
        // Si le gène n'est pas déjà dans child
        int present = 0;
        for (int j = 0; j < size; j++) {
            if (child[j] == gene) {
                present = 1;
                break;
            }
        }
        if (!present) {
            child[c_idx] = gene;
            c_idx = (c_idx + 1) % size;
        }
        p2_idx = (p2_idx + 1) % size;
    }

    return child;
}


// Echange deux gènes dans le genome
void swap_genes(int *genome, int i, int j) {
    int temp = genome[i];
    genome[i] = genome[j];
    genome[j] = temp;
}

// génère une mutation dans un génome en inversant une séquence aléatoire de ce dernier
void mutation(int *genome, int size) {
    int i = rand() % size;
    int j = rand() % size;
    while (i == j) j = rand() % size;
    if (i > j) { int tmp = i; i = j; j = tmp; }

    // inverser les éléments entre i et j
    while (i < j) {
        swap_genes(genome, i, j);
        i++;
        j--;
    }
}

// Fonction utilitaire qui duplique un genome
int* duplicate_genome(int *genome, int size) {
    int *copy = malloc(size * sizeof(int));
    memcpy(copy, genome, size * sizeof(int));
    return copy;
}

void free_population(int **population, int size) {
    for (int i = 0; i < size; i++) {
        free(population[i]);
    }
    free(population);
}

// Affiche tous les génomes d'une population avec leur fitness
void print_best_genome(int **generation, int population_size, int gen_number) {
    printf("\n=== Generation %d ===\n", gen_number);

    int best_idx = 0;
    double best_fitness = calcul_fitness(generation[0], NB_PHARMA - 1);
    double fitness_values[population_size];

    // Affichage des informations du meilleur genome
    printf("\n=== Meilleur resultat de la generation %d ===\n", gen_number);
    printf("IDX: %d | Fitness: %.2f\n", best_idx, best_fitness);
    printf("Parcours: 0 -> ");
    for (int j = 0; j < NB_PHARMA - 1; j++) {
        printf("%d", generation[best_idx][j]);
        if (j < NB_PHARMA - 2) printf(" -> ");
    }
    printf(" -> 0\n");
}

void launch_genetic(double **matrix_trajets, int duration_seconds, int population_size) {
    g_matrix = copy_board_trajet(matrix_trajets);
    int **population = malloc(population_size * sizeof(int*));
    init_generation(population, population_size);
    
    printf("=== DEBUT ALGORITHME GENETIQUE ===\n");
    printf("Pharmacies: %d | Taille population: %d | Duree max: %d secondes\n", 
           NB_PHARMA - 1, population_size, duration_seconds);

    clock_t start_time = clock();
    int gen = 1;
    double elapsed_seconds = 0;
    double best_fitness = 999999999.0;
    int* best_genome;
    
    while (elapsed_seconds < duration_seconds) {
        sort_genomes(population, population_size);
        
        double current_best_fitness = calcul_fitness(population[0], NB_PHARMA - 1);

        if (current_best_fitness < best_fitness) {
            print_best_genome(population, population_size, gen);
            best_fitness = current_best_fitness;
            best_genome = duplicate_genome(population[0], NB_PHARMA - 1);
            }

        int **new_pop = malloc(population_size * sizeof(int*));

        // 1. Élitisme : 2 meilleurs
        int elite_count = 1;
        for (int i = 0; i < elite_count; i++) {
            new_pop[i] = duplicate_genome(population[i], NB_PHARMA - 1);
        }

        // // 2. Croisement : 50% de la population
        int crossover_start = elite_count;
        int crossover_end = crossover_start + (int)(population_size * 0.7);
        int top_30_percent = (int)(population_size * 0.5);

        for (int i = crossover_start; i < crossover_end; i++) {
            int p1 = rand() % top_30_percent;
            int p2 = rand() % top_30_percent;
            while (p2 == p1) p2 = rand() % top_30_percent;

            new_pop[i] = crossover(population[p1], population[p2], NB_PHARMA - 1);

            // Mutation avec une probabilité de 30%
            if (rand() % 100 < 30) {
                mutation(new_pop[i], NB_PHARMA - 1);
            }
        }

        // 3. Génomes aléatoires : reste de la population
        for (int i = crossover_end; i < population_size; i++) {
            new_pop[i] = malloc((NB_PHARMA - 1) * sizeof(int));
            init_genome(new_pop[i], NB_PHARMA - 1);
        }

        free_population(population, population_size);
        population = new_pop;
        gen++;

        elapsed_seconds = (double)(clock() - start_time) / CLOCKS_PER_SEC;
    }

    printf("\n=== RESULTAT FINAL (Generation %d, %.2f secondes) ===\n", gen - 1, elapsed_seconds);
    sort_genomes(population, population_size);
    print_best_genome(population, population_size, gen - 1);
    
    free_population(population, population_size);
}


void print_10_genomes(double **matrix_trajets) {
    g_matrix = copy_board_trajet(matrix_trajets);
    int **generation = malloc(10*sizeof(int*));
    init_generation(generation, 10);
    sort_genomes(generation, 10);
    for (int i = 0; i < 10; i++) {
        int* genome = generation[i];
        printf("Genome #%d :\n", i + 1);
        for (int i = 0; i < NB_PHARMA-1; ++i) {
            printf("%d ", genome[i]);
        }
        printf("\n");
        printf("genome fitness : %.6f\n", calcul_fitness(genome, NB_PHARMA-1));

    }
}