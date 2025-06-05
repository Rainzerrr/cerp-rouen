import pandas as pd
from datetime import datetime, timedelta
from reportlab.lib.pagesizes import A4
from reportlab.platypus import SimpleDocTemplate, Table, TableStyle, Paragraph, Spacer, Image
from reportlab.lib.styles import getSampleStyleSheet
from reportlab.lib import colors
from reportlab.lib.units import cm
import os
from staticmap import StaticMap, CircleMarker, Line
from io import BytesIO
from PIL import Image as PILImage, ImageDraw, ImageFont
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont


# Constantes
DUREE_LIVRAISON = 3  # minutes
HEURE_DEPART_MATIN = "09:00"
HEURE_DEPART_APRES_MIDI = "15:00"
CONSOMMATION_CARBURANT = 6.5  # L/100km
PRIX_DIESEL = 1.72  # euros/L
CERP_COLOR_1 = "#50baa1"
CERP_COLOR_2 = "#9ccb5e" 
CERP_COLOR_3 = "#093866" 



def creer_legende_carte():
    """Crée une image de légende pour la carte"""
    legend = PILImage.new('RGBA', (200, 80), (255, 255, 255, 200))
    draw = ImageDraw.Draw(legend)
    
    # Dessiner les cercles de la légende
    draw.ellipse((20, 15, 40, 35), fill=CERP_COLOR_1)
    draw.ellipse((20, 45, 40, 65), fill=CERP_COLOR_2)
    
    # Ajouter le texte explicatif
    font = ImageFont.truetype("fonts/Grotesk.ttf", 14)
    draw.text((50, 17), "CERP Rouen", fill=(0, 0, 0), font=font)
    draw.text((50, 47), "Pharmacies", fill=(0, 0, 0), font=font)
    
    # Sauvegarder dans un buffer
    buffer = BytesIO()
    legend.save(buffer, format="PNG")
    buffer.seek(0)
    
    return buffer

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
    
    # Pour chaque pharmacie du parcours (sans compter le retour à l'entrepôt)
    for i in range(1, len(parcours_indices) - 1):
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
    idx_derniere = parcours_indices[-2]  # Avant-dernier indice (dernière pharmacie)
    idx_entrepot = parcours_indices[-1]  # Dernier indice (retour à l'entrepôt)
    duree_retour = float(durations_df.iloc[idx_derniere, idx_entrepot]) / 60
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
    
    return distance_totale

def creer_carte(parcours_indices, pharmacies_df, nom_fichier):
    """Crée une carte statique du parcours avec légende"""
    # Définir la taille de la carte
    m = StaticMap(800, 500)
    
    # Extraire les coordonnées
    coords = []
    
    for i, idx in enumerate(parcours_indices):
        lat = float(pharmacies_df.iloc[idx]['latitude'])
        lon = float(pharmacies_df.iloc[idx]['longitude'])
        coords.append((lon, lat))
        
        # Numéroter les points sur le parcours
        if idx == 0:
            m.add_marker(CircleMarker((lon, lat), CERP_COLOR_3, 23))
            m.add_marker(CircleMarker((lon, lat), CERP_COLOR_1, 20))
        else:
            m.add_marker(CircleMarker((lon, lat), CERP_COLOR_3, 23))
            m.add_marker(CircleMarker((lon, lat), CERP_COLOR_2, 20))
    
    # Tracer le parcours
    line = Line(coords, CERP_COLOR_3, 5)
    m.add_line(line)
    
    # Générer l'image de la carte
    img_path = f"{nom_fichier}.png"
    image = m.render()
    
    # Ajouter la légende à l'image de la carte
    legend_buffer = creer_legende_carte()
    legend_img = PILImage.open(legend_buffer)
    image.paste(legend_img, (20, image.height - 100), legend_img)
    
    image.save(img_path)
    
    return os.path.abspath(img_path)

def creer_styles_pdf():
    """Crée des styles personnalisés pour le PDF"""
    styles = getSampleStyleSheet()
    
    # Style pour le titre principal (aligné à gauche)
    title_style = styles["Heading1"].clone('title_style')
    title_style.alignment = 0  # 0 = gauche
    title_style.fontSize = 25
    title_style.leading = 22
    title_style.fontName = 'Grotesk'

    # Style pour le texte normal
    body = styles["Normal"].clone('body')
    body.alignment = 0  # 0 = gauche
    body.fontSize = 10
    body.fontName = 'Grotesk'
    
    styles.add(title_style)
    styles.add(body)
    
    return styles

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
    doc = SimpleDocTemplate(nom_pdf, pagesize=A4, leftMargin=1*cm, rightMargin=1*cm, topMargin=1*cm, bottomMargin=1*cm)
    elements = []
    
    # Styles personnalisés pour le PDF
    styles = creer_styles_pdf()

    logo_path = "logo_cerp.png"
    logo = None
    if os.path.exists(logo_path):
        logo = Image(logo_path, width=50, height=50)
    else:
        print(f"Le logo n'a pas été trouvé à l'emplacement: {logo_path}")
    
    # En-tête avec logo à droite et titre à gauche
    header_data = [[Paragraph(f"Camionnette {num_camionnette}", styles["title_style"]), logo]]
    header = Table(header_data, colWidths=[doc.width-1*cm, 1*cm])
    header.setStyle(TableStyle([
        ('VALIGN', (0, 0), (-1, -1), 'MIDDLE'),
        ('ALIGN', (0, 0), (0, 0), 'LEFT'),
        ('ALIGN', (1, 0), (1, 0), 'RIGHT'),
    ]))
    elements.append(header)
    elements.append(Spacer(1, 20))
    
    # Tableau récapitulatif
    data = [["Pharmacie", "Adresse", "Arrivée", "Départ"]]
    
    entrepot = pharmacies_df.iloc[0]
    adresse_entrepot = f"{entrepot['adresse']}, {entrepot['code_postal']} {entrepot['ville']}"
    data.append([
        "Départ entrepôt CERP",
        adresse_entrepot,
        "",
        heures[0]['depart']
    ])
    
    for i in range(1, len(parcours_indices) - 1):
        idx = parcours_indices[i]
        pharmacie = pharmacies_df.iloc[idx]
        adresse = f"{pharmacie['adresse']}, {pharmacie['code_postal']} {pharmacie['ville']}"
        data.append([
            f"{i}. {pharmacie['nom']}",
            adresse,
            heures[i]['arrivee'],
            heures[i]['depart']
        ])
    
    # Ajouter le retour à l'entrepôt
    data.append([
        "Retour entrepôt CERP",
        adresse_entrepot,
        heure_retour,
        ""
    ])
    
    # Créer le tableau
    largeur_tableau = doc.width * 0.98
    table = Table(data, colWidths=[
        largeur_tableau * 0.30,  # Pharmacie
        largeur_tableau * 0.45,  # Adresse  
        largeur_tableau * 0.125,  # Arrivée
        largeur_tableau * 0.125   # Départ
    ])
    table.setStyle(TableStyle([
        ('BACKGROUND', (0, 0), (-1, 0), CERP_COLOR_3),
        ('TEXTCOLOR', (0, 0), (-1, 0), colors.whitesmoke),
        ('ALIGN', (0, 0), (-1, -1), 'LEFT'),
        ('VALIGN', (0, 0), (-1, -1), 'MIDDLE'),
        ('FONTNAME', (0, 0), (-1, 0), 'Grotesk'),
        ('FONTSIZE', (0, 1), (-1, -1), 8),
        ('BOTTOMPADDING', (0, 0), (-1, 0), 5),
        ('BACKGROUND', (0, 1), (-1, -1), colors.whitesmoke),
        ('GRID', (0, 0), (-1, -1), 1, colors.black),
    ]))
    
    elements.append(table)
    elements.append(Spacer(1, 20))
    
    # Informations sur la distance et le carburant
    elements.append(Paragraph(f"Distance totale : {distance_totale:.2f} km", styles["body"]))
    elements.append(Spacer(1, 5))
    elements.append(Paragraph(f"Carburant estimé : {consommation:.2f}L ({CONSOMMATION_CARBURANT} L / 100km)", styles["body"]))
    elements.append(Spacer(1, 5))
    elements.append(Paragraph(f"Coût carburant : {cout_carburant:.2f} euros (diesel : {PRIX_DIESEL} euros / L)", styles["body"]))
    elements.append(Spacer(1, 20))
    
    try:
        carte_path = creer_carte(parcours_indices, pharmacies_df, f"carte_camionnette_{num_camionnette}")
        img = Image(carte_path, width=525, height=300)
        elements.append(img)
        print(f"Une carte a été intégrée au PDF: {carte_path}")
    except Exception as e:
        print(f"Erreur lors de la création de la carte: {e}")
        print("La carte n'a pas pu être générée.")
        elements.append(Paragraph("La carte n'a pas pu être générée.", styles["body"]))
    
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
    
    parcours_camionnettes = [10, 24, 11, 26, 37, -1, 7, 47, 15, 19, 76, -1, 2, 3, 9, 27] # A compléter avec les parcours réels

    parcours_separes = []
    parcours_temp = []

    for point in parcours_camionnettes:
        if point == -1:
            if parcours_temp:
                parcours_separes.append([0] + parcours_temp + [0])  # Ajouter le point de départ et de fin
            parcours_temp = []
        else:
            parcours_temp.append(point)

    if parcours_temp:
        parcours_separes.append([0] + parcours_temp + [0])  # Ajouter le point de départ et de fin pour le dernier parcours

    print(f"Parcours séparés: {parcours_separes}")
    print("Génération des fichiers PDF...")
    pdfmetrics.registerFont(TTFont('Grotesk', 'fonts/Grotesk.ttf'))
    
    # Utilisez parcours_separes au lieu de parcours_camionnettes
    traiter_tous_parcours(parcours_separes, pharmacies_df, durations_df, distances_df)

if __name__ == "__main__":
    main()