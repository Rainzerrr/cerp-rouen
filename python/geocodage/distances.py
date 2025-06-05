import pandas as pd
import requests
import time
import numpy as np
import os
from tqdm import tqdm
from geopy.geocoders import Nominatim, ArcGIS

def geocode_address(address, city, postal_code):
    full_address = f"{address}, {postal_code} {city}, France"
    try:
        geolocator = Nominatim(user_agent="pharmacy_distance_calculator", timeout=10)
        location = geolocator.geocode(full_address)
        if location:
            print(f"Adresse trouvée par Nominatim: {full_address}")
            return location.latitude, location.longitude
    except:
        print(f"Échec avec Nominatim pour: {full_address}")
    
    try:
        geolocator = ArcGIS(timeout=10)
        location = geolocator.geocode(full_address)
        if location:
            print(f"Adresse trouvée par ArcGIS: {full_address}")
            return location.latitude, location.longitude
    except:
        print(f"Échec avec ArcGIS pour: {full_address}")
    
    print(f"Aucun service n'a trouvé l'adresse: {full_address}")
    print("Adresse ignorée.")
    return None, None

def get_infos_mat(coordinates, chunk_size=100, timeout=30):
    n = len(coordinates)
    
    # Initialisation des matrices de résultats
    mat_distances = np.zeros((n, n))
    mat_durees = np.zeros((n, n))
    
    # Division des coordonnées en chunks pour respecter les limites de l'API
    for i in range(0, n, chunk_size):
        sources_chunk = list(range(i, min(i + chunk_size, n)))
        
        for j in range(0, n, chunk_size):
            destinations_chunk = list(range(j, min(j + chunk_size, n)))
            
            # Construction des coordonnées pour cette partie de la matrice
            coords_str = ";".join([f"{lon},{lat}" for lon, lat in coordinates])
            # Les sources et destinations sont les indices dans la matrice
            sources_str = ";".join(map(str, sources_chunk))
            destinations_str = ";".join(map(str, destinations_chunk))
            
            url = f"http://router.project-osrm.org/table/v1/driving/{coords_str}?sources={sources_str}&destinations={destinations_str}"
            
            try:
                print(f"Requête OSRM pour sources {i}:{i+len(sources_chunk)}, destinations {j}:{j+len(destinations_chunk)}")
                response = requests.get(url, timeout=timeout)
                data = response.json()
                
                if data.get("code") == "Ok":
                    # Récupération des matrices de distances et de durées
                    durations = np.array(data.get("durations", []))
                    distances = np.array(data.get("distances", []))
                    
                    if len(distances) == 0 and "durations" in data:
                        url_with_distances = f"{url}&annotations=distance,duration"
                        response_dist = requests.get(url_with_distances, timeout=timeout)
                        data_dist = response_dist.json()
                        if data_dist.get("code") == "Ok":
                            distances = np.array(data_dist.get("distances", []))
                    
                    # Ajout des valeurs dans les matrices
                    for idx_i, source_idx in enumerate(sources_chunk):
                        for idx_j, dest_idx in enumerate(destinations_chunk):
                            mat_durees[source_idx, dest_idx] = durations[idx_i][idx_j]
                            # Si nous avons des distances
                            if len(distances) > 0:
                                mat_distances[source_idx, dest_idx] = distances[idx_i][idx_j]
                else:
                    print(f"Erreur avec l'API OSRM: {data.get('message', 'Erreur inconnue')}")
                    return -1, -1
                
                # Pour éviter de surcharger l'API
                time.sleep(1)
                
            except Exception as e:
                print(f"Erreur lors de la requête à l'API Table: {e}")
                return -1, -1
    
    return mat_distances, mat_durees

def main():
    # Chargement des données des pharmacies
    csv_path ="data/livraison85.csv"
    
    # Définir les noms de colonnes appropriés pour votre fichier CSV
    col_names = ['nom', 'adresse', 'code_postal', 'ville']
    pharmacies = pd.read_csv(csv_path, header=None, names=col_names)
    
    # Ajouter des colonnes pour les coordonnées géographiques
    pharmacies['latitude'] = None
    pharmacies['longitude'] = None
    
    # Géocoder chaque adresse pour obtenir les coordonnées
    print("Géocodage des adresses...")
    for i, row in tqdm(pharmacies.iterrows(), total=len(pharmacies)):
        print(f"\nTraitement de: {row['nom']}, {row['adresse']}, {row['code_postal']} {row['ville']}")
        lat, lon = geocode_address(row['adresse'], row['ville'], row['code_postal'])
        pharmacies.at[i, 'latitude'] = lat
        pharmacies.at[i, 'longitude'] = lon
        time.sleep(1)
    
    # Filtrer les pharmacies pour lesquelles le géocodage a échoué
    valid_pharmacies = pharmacies.dropna(subset=['latitude', 'longitude'])
    n_valid = len(valid_pharmacies)
    n_total = len(pharmacies)
    
    if n_valid < n_total:
        print(f"Attention: {n_total - n_valid} pharmacies n'ont pas pu être géocodées")
    
    if n_valid == 0:
        print("Aucune pharmacie avec des coordonnées valides. Impossible de calculer les distances.")
        return
    
    # Sauvegarder les coordonnées pour une utilisation future
    pharmacies.to_csv("pharmacies_avec_coordonnees.csv", index=False)
    print("Coordonnées des pharmacies enregistrées dans 'pharmacies_avec_coordonnees.csv'")
    
    # Préparer les coordonnées au format attendu par l'API OSRM (longitude, latitude)
    coordinates = [(row['longitude'], row['latitude']) for _, row in valid_pharmacies.iterrows()]
    
    # Déterminer la taille optimale des chunks en fonction du nombre de pharmacies
    chunk_size = min(100, n_valid)
    
    print(f"Calcul de la matrice de distances et durées pour {n_valid} pharmacies...")
    
    try:
        # Calculer la matrice complète en une seule fois ou par chunks
        distances, durations = get_infos_mat(coordinates, chunk_size=chunk_size)
        
        if isinstance(distances, int) and distances == -1:
            print("Erreur lors du calcul de la matrice. Impossible de continuer.")
            return
        
        # Création des DataFrames avec les matrices
        distance_df = pd.DataFrame(distances)
        durations_df = pd.DataFrame(durations)
    
        # Sauvegarder la relation entre pharmacies et indices
        indices_mapping = pd.DataFrame({
            'indice': range(len(valid_pharmacies)),
            'nom': valid_pharmacies['nom'].tolist(),
            'adresse': valid_pharmacies['adresse'].tolist(),
            'code_postal': valid_pharmacies['code_postal'].tolist(),
            'ville': valid_pharmacies['ville'].tolist()
        })
        indices_mapping.to_csv("pharmacies_indices_mapping.csv", index=False)
        
        # Sauvegarde des résultats
        distance_csv = "distances_pharmacies.csv"
        durations_csv = "durations_pharmacies.csv"
        distance_df.to_csv(distance_csv)
        durations_df.to_csv(durations_csv)
        print(f"Matrice de distances enregistrée dans '{distance_csv}'")
        print(f"Matrice de durées enregistrée dans '{durations_csv}'")
        
    except Exception as e:
        print(f"Erreur pendant le calcul des informations: {e}")
        print("Les informations n'ont pas pu être sauvegardées complètement.")

if __name__ == "__main__":
    main()