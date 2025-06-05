import pandas as pd
from datetime import datetime, timedelta
from reportlab.lib.pagesizes import A4
from reportlab.platypus import SimpleDocTemplate, Table, TableStyle, Paragraph, Spacer, Image
from reportlab.lib.styles import getSampleStyleSheet
from reportlab.lib import colors
import os
from staticmap import StaticMap, CircleMarker, Line


# Constantes
DUREE_LIVRAISON = 3  # minutes
HEURE_DEPART_MATIN = "09:00"
HEURE_DEPART_APRES_MIDI = "15:00"
CONSOMMATION_CARBURANT = 6.5  # L/100km
PRIX_DIESEL = 1.72  # euros/L

def charger_donnees():
    # Chargement des données
    pharmacies_df = pd.read_csv('../geocodage/pharmacies_avec_coordonnees.csv')
    durations_df = pd.read_csv('../geocodage/durations_pharmacies.csv')
    distances_df = pd.read_csv('../geocodage/distances_pharmacies.csv')
    
    # Suppression des indices
    if 'Unnamed: 0' in durations_df.columns:
        durations_df = durations_df.drop('Unnamed: 0', axis=1)
    if 'Unnamed: 0' in distances_df.columns:
        distances_df = distances_df.drop('Unnamed: 0', axis=1)
    
    print(f"Nombre de pharmacies: {len(pharmacies_df)}")
    print(f"Dimensions de la matrice de durées: {durations_df.shape}")
    
    return pharmacies_df, durations_df, distances_df

def calculer_heures_parcours(parcours_indices, durations_df, est_matin=True):
    """Calcule les heures d'arrivée et de départ pour chaque pharmacie du parcours"""
    heures = []
    
    # Définir l'heure de départ
    if est_matin:
        heure_actuelle = datetime.strptime(HEURE_DEPART_MATIN, "%H:%M")
    else:
        heure_actuelle = datetime.strptime(HEURE_DEPART_APRES_MIDI, "%H:%M")
    
    # Ajouter l'heure de départ de l'entrepôt
    heures.append({
        "arrivee": heure_actuelle.strftime("%H:%M"),
        "depart": heure_actuelle.strftime("%H:%M")
    })
    
    # Pour chaque pharmacie du parcours
    for i in range(1, len(parcours_indices)):
        idx_prec = parcours_indices[i-1]
        idx_act = parcours_indices[i]
        duree_trajet = float(durations_df.iloc[idx_prec, idx_act]) / 60
            
        # Calculer l'heure d'arrivée
        heure_actuelle += timedelta(minutes=duree_trajet)
        heure_arrivee = heure_actuelle.strftime("%H:%M")
        
        # Ajouter la durée de livraison
        heure_actuelle += timedelta(minutes=DUREE_LIVRAISON)
        heure_depart = heure_actuelle.strftime("%H:%M")
        
        heures.append({
            "arrivee": heure_arrivee,
            "depart": heure_depart
        })
    
    # Calculer l'heure de retour à l'entrepôt
    idx_derniere = parcours_indices[-1]
    duree_retour = float(durations_df.iloc[idx_derniere, 0]) / 60  # Retour vers l'entrepôt (indice 0) - A supprimer plus tard
    heure_actuelle += timedelta(minutes=duree_retour)
    heure_retour = heure_actuelle.strftime("%H:%M")
    
    return heures, heure_retour

def calculer_distance_parcours(parcours_indices, distances_df):
    """Calcule la distance totale du parcours en km"""
    distance_totale = 0
    
    # Pour chaque paire de pharmacies consécutives
    for i in range(len(parcours_indices) - 1):
        idx_depart = parcours_indices[i]
        idx_arrivee = parcours_indices[i + 1]
        distance = float(distances_df.iloc[idx_depart, idx_arrivee]) / 1000 # Convertir en km
        distance_totale += distance
    
    # Ajouter la distance de retour à l'entrepôt
    idx_derniere = parcours_indices[-1]
    distance_retour = float(distances_df.iloc[idx_derniere, 0]) / 1000  # Retour vers l'entrepôt (indice 0) - A supprimer plus tard
    distance_totale += distance_retour
    
    return distance_totale

def creer_carte(parcours_indices, pharmacies_df, nom_fichier):
    """Crée une carte statique du parcours"""
    # Définir la taille de la carte
    m = StaticMap(800, 500)
    
    # Extraire les coordonnées
    coords = []
    for idx in parcours_indices:
        lat = float(pharmacies_df.iloc[idx]['latitude'])
        lon = float(pharmacies_df.iloc[idx]['longitude'])
        coords.append((lon, lat)) # Static Map inverse les coordonnées (longitude, latitude)
        
        # Marquer chaque pharmacie
        if idx == 0:  # Entrepôt
            m.add_marker(CircleMarker((lon, lat), '#FF0000', 10))  # Rouge pour l'entrepôt
        else:
            m.add_marker(CircleMarker((lon, lat), '#0000FF', 8))   # Bleu pour les pharmacies
    
    # Ajouter les coordonnées du retour à l'entrepôt
    entrepot_lat = float(pharmacies_df.iloc[0]['latitude'])
    entrepot_lon = float(pharmacies_df.iloc[0]['longitude'])
    coords.append((entrepot_lon, entrepot_lat))
    
    # Tracer le parcours
    line = Line(coords, '#FF0000', 4)
    m.add_line(line)
    
    # Générer l'image
    img_path = f"{nom_fichier}.png"
    image = m.render()
    image.save(img_path)
    
    return os.path.abspath(img_path)

def generer_pdf_parcours(parcours_indices, pharmacies_df, durations_df, distances_df, num_camionnette, est_matin=True):
    """Génère un PDF pour un parcours donné"""
    # Calculer les heures d'arrivée et de départ
    heures, heure_retour = calculer_heures_parcours(parcours_indices, durations_df, est_matin)
    # Calculer la distance totale
    distance_totale = calculer_distance_parcours(parcours_indices, distances_df)
    
    # Calculer la consommation de carburant et le coût
    consommation = distance_totale * CONSOMMATION_CARBURANT / 100
    cout_carburant = consommation * PRIX_DIESEL
    
    # Créer le PDF
    nom_pdf = f"parcours_camionnette_{num_camionnette}.pdf"
    doc = SimpleDocTemplate(nom_pdf, pagesize=A4)
    elements = []
    
    # Styles pour le PDF
    styles = getSampleStyleSheet()
    titre_style = styles["Heading1"]
    
    # Titre
    elements.append(Paragraph(f"Camionnette {num_camionnette}", titre_style))
    elements.append(Spacer(1, 12))
    
    # Tableau récapitulatif
    data = [["Pharmacie", "Adresse", "Arrivée", "Départ"]]
    
    # Ajouter l'entrepôt
    entrepot = pharmacies_df.iloc[0]
    adresse_entrepot = f"{entrepot['adresse']}, {entrepot['code_postal']} {entrepot['ville']}"
    data.append([
        "Départ entrepôt Cerp",
        adresse_entrepot,
        "",
        heures[0]['depart']
    ])
    
    # Ajouter chaque pharmacie
    for i in range(1, len(parcours_indices)):
        idx = parcours_indices[i]
        pharmacie = pharmacies_df.iloc[idx]
        adresse = f"{pharmacie['adresse']}, {pharmacie['code_postal']} {pharmacie['ville']}"
        data.append([
            pharmacie['nom'],
            adresse,
            heures[i]['arrivee'],
            heures[i]['depart']
        ])
    
    # Ajouter le retour à l'entrepôt
    data.append([
        "Retour entrepôt Cerp",
        adresse_entrepot,
        heure_retour,
        ""
    ])
    
    # Créer le tableau
    table = Table(data)
    table.setStyle(TableStyle([
        ('BACKGROUND', (0, 0), (-1, 0), colors.darkslateblue),
        ('TEXTCOLOR', (0, 0), (-1, 0), colors.whitesmoke),
        ('ALIGN', (0, 0), (-1, -1), 'CENTER'),
        ('FONTNAME', (0, 0), (-1, 0), 'Helvetica'),
        ('BOTTOMPADDING', (0, 0), (-1, 0), 5),
        ('BACKGROUND', (0, 1), (-1, -1), colors.whitesmoke),
        ('GRID', (0, 0), (-1, -1), 1, colors.black),
    ]))
    
    elements.append(table)
    elements.append(Spacer(1, 12))
    
    # Informations sur la distance et le carburant
    elements.append(Paragraph(f"Distance totale : {distance_totale:.2f} km", styles["Normal"]))
    elements.append(Paragraph(f"Carburant estimé : {consommation:.2f}L ({CONSOMMATION_CARBURANT} L / 100km)", styles["Normal"]))
    elements.append(Paragraph(f"Coût carburant : {cout_carburant:.2f} euros (diesel : {PRIX_DIESEL} euros / L)", styles["Normal"]))
    
    # Générer et inclure la carte
    elements.append(Spacer(1, 12))
    elements.append(Paragraph("Carte du parcours:", styles["Heading2"]))
    
    try:
        carte_path = creer_carte(parcours_indices, pharmacies_df, f"carte_camionnette_{num_camionnette}")
        img = Image(carte_path, width=500, height=300)
        elements.append(img)
        print(f"Une carte a été intégrée au PDF: {carte_path}")
    except Exception as e:
        print(f"Erreur lors de la création de la carte: {e}")
        print("La carte n'a pas pu être générée.")
        elements.append(Paragraph("La carte n'a pas pu être générée.", styles["Normal"]))
    
    # Terminer le PDF
    doc.build(elements)
    
    print(f"Le fichier PDF '{nom_pdf}' a été généré avec succès.")
    
    return nom_pdf

def traiter_tous_parcours(parcours_camionnettes, pharmacies_df, durations_df, distances_df):
    """Traite tous les parcours et génère les PDF"""
    for i, parcours in enumerate(parcours_camionnettes):
        # Première moitié des camionnettes le matin, seconde moitié l'après-midi
        est_matin = i < len(parcours_camionnettes) // 2  
        generer_pdf_parcours(parcours, pharmacies_df, durations_df, distances_df, i+1, est_matin)
    
    print(f"Tous les PDF ont été générés avec succès.")

def main():
    # Charger les données
    print("Chargement des données...")
    pharmacies_df, durations_df, distances_df = charger_donnees()
    
    parcours_camionnettes = [
        [0, 10, 24, 11, 26, 37],
        [0, 7, 47, 15, 19]
    ]
        
    print("Génération des fichiers PDF...")
    traiter_tous_parcours(parcours_camionnettes, pharmacies_df, durations_df, distances_df)

if __name__ == "__main__":
    main()