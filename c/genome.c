#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "trajet.h"
#include "matrix.h"

#define MAX_TRIALS 100

// Matrice des trajets utilisés dans les fonctions de l'algorithme génétique.
static BoardTrajet g_matrix;

// Récupère le trajet entre deux genes donnés
Trajet* get_trajet(int i, int j) {
    return &g_matrix[i][j];
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
        Trajet* trajet = get_trajet(genome[i], genome[i+1]);
        sum+=trajet->distance;

    }

    // Ajout du trajet entre le depot avec la première pharmacie
    sum += get_trajet(0, genome[0])->distance;

    // Ajout du trajet entre la dernière pharmacie et le depot
    sum += get_trajet(genome[size-1], 0)->distance;

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

// Crée un genome en fusionnant deux autres genomes
int* alternate_crossover(int *parent1, int *parent2, int size) {
    int *child = malloc(size * sizeof(int));
    int child_index = 0;
    int p1_index = 0, p2_index = 0;
    
    for (int i = 0; i < size; i++) {
        child[i] = -1;
    }

    while (child_index < size) {
        // Essayer d'ajouter un element de parent1
        if (p1_index < size) {
            int gene = parent1[p1_index];
            if (!is_present(child, size, gene)) {
                child[child_index++] = gene;
            }
            p1_index++;
        }

        // Essayer d'ajouter un element de parent2
        if (child_index < size && p2_index < size) {
            int gene = parent2[p2_index];
            if (!is_present(child, size, gene)) {
                child[child_index++] = gene;
            }
            p2_index++;
        }
    }

    return child;
}

// Echange deux gènes dans le genome
void swap_genes(int *genome, int i, int j) {
    int temp = genome[i];
    genome[i] = genome[j];
    genome[j] = temp;
}

// Applique une mutation sur 10% des gènes d'un genome (au moins 1)
void mutatation(int *genome, int size) {
    // Calcul du nombre de mutations (10%, min 1)
    int num_mutations = (int)ceil(size * 0.1);
    if (num_mutations < 1) num_mutations = 1;

    // Appliquer les mutations
    for (int n = 0; n < num_mutations; n++) {
        int i = rand() % size; // Gène aleatoire
        int j = rand() % size; // Gène à echanger
        
        // eviter les echanges inutiles (i == j)
        while (i == j) {
            j = rand() % size;
        }

        swap_genes(genome, i, j);
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

void launch_genetic(BoardTrajet matrix_trajets, int duration_seconds, int population_size) {
    copy_board_trajet(g_matrix, matrix_trajets);
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
        int elite_count = 2;
        for (int i = 0; i < elite_count; i++) {
            new_pop[i] = duplicate_genome(population[i], NB_PHARMA - 1);
        }

        // 2. Croisement : 50% de la population
        int crossover_start = elite_count;
        int crossover_end = crossover_start + (population_size / 2);
        int top_30_percent = population_size * 30 / 100;

        for (int i = crossover_start; i < crossover_end; i++) {
            int p1 = rand() % top_30_percent;
            int p2 = rand() % top_30_percent;
            while (p2 == p1) p2 = rand() % top_30_percent;

            new_pop[i] = alternate_crossover(population[p1], population[p2], NB_PHARMA - 1);

            // Mutation avec une probabilité de 30%
            if (rand() % 100 < 30) {
                mutatation(new_pop[i], NB_PHARMA - 1);
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


void print_10_genomes(BoardTrajet matrix_trajets) {
    copy_board_trajet(g_matrix, matrix_trajets);
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
        // for (int k = 0; k < NB_PHARMA - 1 - 1; k++) { // -1 parce que 4 elements et -1 parce que boucle 2 à 2 (+1 dans la boucle)
        //     print_trajets(get_trajet(genome[k], genome[k+1]));
        // }
        printf("genome fitness : %.6f\n", calcul_fitness(genome, NB_PHARMA-1));

    }
}