const fs = require('fs').promises;

async function main() {
    try {
        // 1. Lire le fichier livraison85.csv
        const deliveryContent = await fs.readFile('livraison85.csv', 'utf8');
        const selectedPharmacies = parseDeliveryFile(deliveryContent);
        
        // 2. Traiter les fichiers de distances et durées
        const distanceContent = await fs.readFile('distances_pharmacies.csv', 'utf8');
        const filteredDistances = filterMatrix(distanceContent, selectedPharmacies);
        await fs.writeFile('distances_filtrees.csv', filteredDistances);
        
        const durationContent = await fs.readFile('durations_pharmacies.csv', 'utf8');
        const filteredDurations = filterMatrix(durationContent, selectedPharmacies);
        await fs.writeFile('durees_filtrees.csv', filteredDurations);
        
        console.log('Traitement terminé avec succès !');
        console.log(`Pharmacies traitées: ${selectedPharmacies.length}`);
    } catch (error) {
        console.error('Erreur:', error);
    }
}

function parseDeliveryFile(content) {
    const pharmacies = new Set();
    const lines = content.trim().split('\n');
    
    for (const line of lines) {
        // Supposer que l'ID est dans la première colonne
        const pharmacyId = line.split(',')[0].trim();
        if (pharmacyId) pharmacies.add(pharmacyId);
    }
    
    return Array.from(pharmacies);
}

function filterMatrix(matrixContent, selectedPharmacies) {
    const lines = matrixContent.trim().split('\n');
    const headers = lines[0].split(',');
    
    // 1. Identifier les indices des pharmacies sélectionnées
    const selectedIndices = [];
    for (let i = 1; i < headers.length; i++) {
        if (selectedPharmacies.includes(headers[i].trim())) {
            selectedIndices.push(i);
        }
    }
    
    // 2. Filtrer les lignes et colonnes
    const filteredLines = [];
    
    // En-tête
    const filteredHeader = [headers[0]];
    selectedIndices.forEach(idx => filteredHeader.push(headers[idx]));
    filteredLines.push(filteredHeader.join(','));
    
    // Données
    for (let i = 1; i < lines.length; i++) {
        const cells = lines[i].split(',');
        const rowId = cells[0].trim();
        
        if (selectedPharmacies.includes(rowId)) {
            const filteredRow = [rowId];
            selectedIndices.forEach(idx => filteredRow.push(cells[idx].trim()));
            filteredLines.push(filteredRow.join(','));
        }
    }
    
    return filteredLines.join('\n');
}

// Exécution du programme
main();