# cerp-rouen
Projet pluridisciplinaire S2 ESIEE Paris E3

## Génération des distances entre les pharmacies

Plusieurs solutions ont été envisagé, appels API vers openstreetmap en JS, mais pas possible car limite d'appels. Ensuite, il a été envisagé de faire en serveur OSRM pour calculer nous même les distances, mais pas possible d'installer les dépendances nécessaires sur les ordinateurs de l'école.

Finalement, le calcul des distances sera fait avec le package openstreetmap de python, plus particulierement l'outil distances_matrix. Cela rend malheureusement obsolète une bonne partie du code C déjà réalisé, qui s'occupait de générer la matrice à partir du CSV généré en JS.
