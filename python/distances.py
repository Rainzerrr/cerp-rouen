import pandas as pd
import requests
import time
import numpy as np
import os
from tqdm import tqdm
from geopy.geocoders import Nominatim
from geopy.exc import GeocoderTimedOut, GeocoderServiceError

# Fonction permettant de géocoder une adresse
def geocode_address(address, city, postal_code, attempt=1, max_attempts=5, timeout=10):
    if attempt > max_attempts:
        print(f"Échec après {max_attempts} tentatives pour: {address}, {postal_code} {city}")
        print("Veuillez entrer manuellement les coordonnées (latitude,longitude) ou appuyez sur Entrée pour ignorer:")
        print(f"Format attendu: lat,lon")
        user_input = input("> ")
        if user_input.strip():
            try:
                lat, lon = map(float, user_input.split(','))
                print(f"Coordonnées manuelles enregistrées: {lat}, {lon}")
                return lat, lon
            except ValueError:
                print("Format invalide. L'adresse sera ignorée.")
                return None, None
        else:
            print("Adresse ignorée.")
            return None, None
    
    # Formattage de l'adresse complète
    full_address = f"{address}, {postal_code} {city}, France"
    
    # Initialisation du géocodeur avec un user-agent spécifique et un timeout plus long
    geolocator = Nominatim(user_agent="pharmacy_distance_calculator", timeout=timeout)
    
    try:
        location = geolocator.geocode(full_address)
        if location:
            return location.latitude, location.longitude
        else:
            print(f"Adresse non trouvée: {full_address}")
            print("Veuillez entrer manuellement les coordonnées (latitude,longitude) ou appuyez sur Entrée pour ignorer:")
            print(f"Format attendu: lat,lon")
            user_input = input("> ")
            
            if user_input.strip():
                try:
                    lat, lon = map(float, user_input.split(','))
                    print(f"Coordonnées manuelles enregistrées: {lat}, {lon}")
                    return lat, lon
                except ValueError:
                    print("Format invalide. L'adresse sera ignorée.")
                    return None, None
            else:
                print("Adresse ignorée.")
                return None, None
    except (GeocoderTimedOut, GeocoderServiceError) as e:
        print(f"Erreur de géocodage (tentative {attempt}/{max_attempts}): {e}")
        # Attente exponentielle avant de réessayer
        wait_time = 2 ** attempt
        print(f"Attente de {wait_time} secondes avant nouvelle tentative...")
        time.sleep(wait_time)
        return geocode_address(address, city, postal_code, attempt + 1, max_attempts, timeout)
    except Exception as e:
        print(f"Erreur inattendue lors du géocodage: {e}")
        time.sleep(5)
        return geocode_address(address, city, postal_code, attempt + 1, max_attempts, timeout)

# Fonction pour obtenir la matrice de distances et durées
def get_infos_matrix(coordinates, chunk_size=100, timeout=30):
    n = len(coordinates)
    
    # Initialisation des matrices de résultats
    matrix_distances = np.zeros((n, n))
    matrix_durations = np.zeros((n, n))
    
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
                    
                    # Si les distances ne sont pas incluses (par défaut OSRM ne renvoie que les durées)
                    # alors on refait une requête spécifique pour les distances
                    if len(distances) == 0 and "durations" in data:
                        url_with_distances = f"{url}&annotations=distance,duration"
                        response_dist = requests.get(url_with_distances, timeout=timeout)
                        data_dist = response_dist.json()
                        if data_dist.get("code") == "Ok":
                            distances = np.array(data_dist.get("distances", []))
                    
                    # Ajout des valeurs dans les matrices
                    for idx_i, source_idx in enumerate(sources_chunk):
                        for idx_j, dest_idx in enumerate(destinations_chunk):
                            matrix_durations[source_idx, dest_idx] = durations[idx_i][idx_j]
                            # Si nous avons des distances
                            if len(distances) > 0:
                                matrix_distances[source_idx, dest_idx] = distances[idx_i][idx_j]
                else:
                    print(f"Erreur avec l'API OSRM: {data.get('message', 'Erreur inconnue')}")
                    return -1, -1
                
                # Pour éviter de surcharger l'API
                time.sleep(1)
                
            except Exception as e:
                print(f"Erreur lors de la requête à l'API Table: {e}")
                return -1, -1
    
    return matrix_distances, matrix_durations

def save_checkpoint(pharmacies, processed_count):
    """Sauvegarde les données traitées jusqu'à présent"""
    checkpoint_file = "pharmacies_checkpoint.csv"
    pharmacies.to_csv(checkpoint_file, index=False)
    print(f"Point de sauvegarde créé: {processed_count} pharmacies traitées. Fichier: {checkpoint_file}")

def main():
    # Vérification de l'existance d'un fichier de point de contrôle pour reprendre le traitement
    checkpoint_file = "pharmacies_checkpoint.csv"
    if os.path.exists(checkpoint_file):
        print(f"Reprise du traitement à partir du point de sauvegarde: {checkpoint_file}")
        pharmacies = pd.read_csv(checkpoint_file)
        # Comptage des pharmacies déjà géocodées
        processed = pharmacies.dropna(subset=['latitude', 'longitude']).shape[0]
        print(f"{processed} pharmacies déjà géocodées.")
    else:
        # Chargement des données des pharmacies
        csv_path = os.path.join(os.path.dirname(os.path.dirname(__file__)), "python", "data", "livraison85.csv")
        try:
            col_names = ['nom', 'adresse', 'code_postal', 'ville']
            pharmacies = pd.read_csv(csv_path, header=None, names=col_names)
            pharmacies['latitude'] = None
            pharmacies['longitude'] = None
            processed = 0
        except FileNotFoundError:
            print(f"Fichier {csv_path} non trouvé")
            return
    
    # Géocoder chaque adresse pour obtenir les coordonnées
    print("Géocodage des adresses...")
    try:
        for i, row in tqdm(pharmacies.iterrows(), total=len(pharmacies), initial=processed):
            # Si cette pharmacie a déjà des coordonnées, ignorer
            if pd.notna(pharmacies.at[i, 'latitude']) and pd.notna(pharmacies.at[i, 'longitude']):
                continue
                
            print(f"\nTraitement de: {row['nom']}, {row['adresse']}, {row['code_postal']} {row['ville']}")
            lat, lon = geocode_address(row['adresse'], row['ville'], row['code_postal'])
            pharmacies.at[i, 'latitude'] = lat
            pharmacies.at[i, 'longitude'] = lon
            
            # Sauvegarder le progrès tous les 5 géocodages
            if (i + 1) % 5 == 0:
                save_checkpoint(pharmacies, i + 1)
                
            # Pause pour respecter les limites d'utilisation de l'API
            time.sleep(1.5)
    except KeyboardInterrupt:
        print("\nInterruption détectée. Sauvegarde du progrès actuel...")
        save_checkpoint(pharmacies, processed)
        print("Vous pouvez reprendre plus tard en relançant le script.")
        return
    except Exception as e:
        print(f"Erreur pendant le géocodage: {e}")
        save_checkpoint(pharmacies, processed)
        print("Point de sauvegarde créé avant l'erreur. Vous pouvez reprendre plus tard.")
        return
    
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
        distances, durations = get_infos_matrix(coordinates, chunk_size=chunk_size)
        
        if isinstance(distances, int) and distances == -1:
            print("Erreur lors du calcul de la matrice. Impossible de continuer.")
            return
            
        # Utiliser les noms des pharmacies comme identifiants
        pharmacies_ids = valid_pharmacies['nom'].tolist()
        
        # Création des DataFrames avec les matrices
        distance_df = pd.DataFrame(distances, index=pharmacies_ids, columns=pharmacies_ids)
        durations_df = pd.DataFrame(durations, index=pharmacies_ids, columns=pharmacies_ids)
    
        # Sauvegarde des résultats
        distance_csv = "distances_pharmacies.csv"
        durations_csv = "durations_pharmacies.csv"
        distance_df.to_csv(distance_csv)
        durations_df.to_csv(durations_csv)
        print(f"Matrice de distances enregistrée dans '{distance_csv}'")
        print(f"Matrice de durées enregistrée dans '{durations_csv}'")
        
        # Sauvegarde au format numpy pour utilisation future
        np.save("distances_pharmacies.npy", distances)
        np.save("durations_pharmacies.npy", durations)
        
    except KeyboardInterrupt:
        print("\nInterruption détectée. Sauvegarde des informations partielles...")
        if 'distances' in locals() and isinstance(distances, np.ndarray):
            np.save("distances_pharmacies_partial.npy", distances)
        if 'durations' in locals() and isinstance(durations, np.ndarray):
            np.save("durations_pharmacies_partial.npy", durations)
        print("Calcul interrompu. Les informations partielles ont été sauvegardées.")
    except Exception as e:
        print(f"Erreur pendant le calcul des informations: {e}")
        if 'distances' in locals() and isinstance(distances, np.ndarray):
            np.save("distances_pharmacies_partial.npy", distances)
        if 'durations' in locals() and isinstance(durations, np.ndarray):
            np.save("durations_pharmacies_partial.npy", durations)
        print("Erreur: Les informations partielles ont été sauvegardées.")

if __name__ == "__main__":
    main()