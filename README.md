# cerp-rouen
Projet pluridisciplinaire S2 ESIEE Paris E3

## Génération des distances entre les pharmacies

Plusieurs solutions ont été envisagé, appels API vers openstreetmap en JS, mais pas possible car limite d'appels. Ensuite, il a été envisagé de faire en serveur OSRM pour calculer nous même les distances, mais pas possible d'installer les dépendances nécessaires sur les ordinateurs de l'école.

Finalement, le calcul des distances sera fait en pythonn, notamment avec : 
- Nominatim, une librairie qui permet de trouver les coordonnées GPS à partir des adresses. Malheureusement, la recherche plantait quelque fois, une méthode manuelle à donc était ajoutée, pour les quelques adresses qui n'était pas géocodée. La recherche manuelle a été effectuée sur ce site : https://torop.net/coordonnees-gps.php.

Le fonctionnement de base utilisait l'API project-osrm.org, et plus précisément cette adresse : http://router.project-osrm.org/route/v1/driving/, qui permettait de récupérer la distance en véhicule roulant entre deux coordonnées géographiques.

Comme le processus était très long, un système de sauvegarde avait été ajouté, pour ne pas avoir à refaire tous les calculs en cas d'erreur. Tous les 5 géocodages, le progrès était sauvegardé. Les limites de Nominatim étaient strictes, il fallait donc ajouter des temps de *sleep* pour ne pas surcharger. C'était de même pour le calcul des distances, tous les 50 calculs, la matrice des distances était sauvegardée dans un fichier.

Finalement, une API permettant de réaliser directement la matrice des distances et des durées (demande supplémentaire) a été trouvée, il suffit de passer tous les points qu'on souhaite relié, et l'API s'en charge en moins de 5 secondes : http://router.project-osrm.org/table/v1/driving. 