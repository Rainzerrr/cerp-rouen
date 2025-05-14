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
    
    # Formater l'adresse complète
    full_address = f"{address}, {postal_code} {city}, France"
    
    # Initialiser le géocodeur avec un user-agent spécifique et un timeout plus long
    geolocator = Nominatim(user_agent="pharmacy_distance_calculator", timeout=timeout)
    
    try:
        # Obtenir les coordonnées
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
        # Attendre plus longtemps entre les tentatives
        wait_time = 2 ** attempt
        print(f"Attente de {wait_time} secondes avant nouvelle tentative...")
        time.sleep(wait_time)
        return geocode_address(address, city, postal_code, attempt + 1, max_attempts, timeout)
    except Exception as e:
        print(f"Erreur inattendue lors du géocodage: {e}")
        time.sleep(5)
        return geocode_address(address, city, postal_code, attempt + 1, max_attempts, timeout)

# Fonction pour obtenir la distance routière entre deux points
def get_driving_distance(origin_lat, origin_lon, dest_lat, dest_lon, timeout=10):
    if origin_lat is None or origin_lon is None or dest_lat is None or dest_lon is None:
        return -1
        
    url = f"http://router.project-osrm.org/route/v1/driving/{origin_lon},{origin_lat};{dest_lon},{dest_lat}?overview=false"
    
    try:
        response = requests.get(url, timeout=timeout)
        data = response.json()
        
        if data["code"] == "Ok":
            return data["routes"][0]["distance"]  # Distance en mètres
        else:
            print(f"Pas de route trouvée entre ({origin_lat},{origin_lon}) et ({dest_lat},{dest_lon})")
            return -1
    except Exception as e:
        print(f"Erreur lors de la requête API: {e}")
        return -1

def save_checkpoint(pharmacies, processed_count):
    """Sauvegarde les données traitées jusqu'à présent"""
    checkpoint_file = "pharmacies_checkpoint.csv"
    pharmacies.to_csv(checkpoint_file, index=False)
    print(f"Point de sauvegarde créé: {processed_count} pharmacies traitées. Fichier: {checkpoint_file}")

def main():
    # Vérifier s'il existe un fichier de point de contrôle pour reprendre le traitement
    checkpoint_file = "pharmacies_checkpoint.csv"
    if os.path.exists(checkpoint_file):
        print(f"Reprise du traitement à partir du point de sauvegarde: {checkpoint_file}")
        pharmacies = pd.read_csv(checkpoint_file)
        # Compter combien de pharmacies ont déjà des coordonnées
        processed = pharmacies.dropna(subset=['latitude', 'longitude']).shape[0]
        print(f"{processed} pharmacies déjà géocodées.")
    else:
        # Chargement des données des pharmacies
        csv_path = os.path.join(os.path.dirname(os.path.dirname(__file__)), "python", "data", "livraison85.csv")
        try:
            # Définir les noms de colonnes appropriés pour votre fichier CSV
            col_names = ['nom', 'adresse', 'code_postal', 'ville']
            pharmacies = pd.read_csv(csv_path, header=None, names=col_names)
            # Ajouter des colonnes pour les coordonnées géographiques
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
    
    # Création de la matrice de distances
    n = n_valid
    distances = np.zeros((n, n))
    
    # Calcul des distances routières pour chaque paire de pharmacies
    total_pairs = n * (n - 1) // 2
    print(f"Calcul de {total_pairs} distances routières entre {n} pharmacies...")
    
    pair_count = 0
    try:
        with tqdm(total=total_pairs) as pbar:
            for i in range(n):
                for j in range(i+1, n):
                    origin_lat = valid_pharmacies.iloc[i]['latitude']
                    origin_lon = valid_pharmacies.iloc[i]['longitude']
                    dest_lat = valid_pharmacies.iloc[j]['latitude']
                    dest_lon = valid_pharmacies.iloc[j]['longitude']
                    
                    # Calcul de la distance routière
                    distance = get_driving_distance(origin_lat, origin_lon, dest_lat, dest_lon)
                    
                    # Stockage dans la matrice (symétrique)
                    distances[i, j] = distance
                    distances[j, i] = distance
                    
                    pair_count += 1
                    # Sauvegarder périodiquement la matrice de distances
                    if pair_count % 50 == 0:
                        np.save("distances_pharmacies_partial.npy", distances)
                    
                    # Pause pour ne pas surcharger l'API
                    time.sleep(0.5)
                    
                    pbar.update(1)
    except KeyboardInterrupt:
        print("\nInterruption détectée. Sauvegarde des distances calculées jusqu'à présent...")
        np.save("distances_pharmacies_partial.npy", distances)
        print("Calcul interrompu. Les distances partielles ont été sauvegardées.")
        return
    except Exception as e:
        print(f"Erreur pendant le calcul des distances: {e}")
        np.save("distances_pharmacies_partial.npy", distances)
        print("Erreur: Les distances partielles ont été sauvegardées.")
        return
    
    # Utiliser les noms des pharmacies comme identifiants
    pharmacies_ids = valid_pharmacies['nom'].tolist()
    
    # Création d'un DataFrame avec la matrice de distances
    distance_df = pd.DataFrame(distances, index=pharmacies_ids, columns=pharmacies_ids)
    
    # Sauvegarde des résultats
    output_csv = "distances_pharmacies.csv"
    distance_df.to_csv(output_csv)
    print(f"Matrice de distances enregistrée dans '{output_csv}'")
    
    # Sauvegarde au format numpy pour utilisation future
    np.save("distances_pharmacies.npy", distances)

if __name__ == "__main__":
    main()