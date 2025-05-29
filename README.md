# Cerp Rouen - Optimisation de tournée des pharmacies par algorithme génétique

**Problème du Voyageur de Commerce (PVC) appliqué aux pharmacies de Rouen**

## 📦 Installation

### Prérequis

- Compilateur C (ex: `gcc`)
- Bibliothèque mathématique (`-lm`)
- Système UNIX/Linux (recommandé)

```bash
git clone https://github.com/Rainzerrr/cerp-rouen.git
cd cerp-rouen/c
make
```

### Lancement du programme

```bash
./voyageur [durée_max=60] [taille_population=100]

Exemples :
./voyageur          # 60s, population=100
./voyageur 30       # 30s, population=100
./voyageur 120 500  # 120s, population=500
```

### Notes d'implémentation :

L'algorithme actuel optimise les tournées pour un seul camion, avec une architecture conçue pour évoluer vers :

- Gestion de flotte : Répartition automatique sur plusieurs véhicules
- Contrainte opérationnelle : Limite stricte de 3 heures de service par camion
