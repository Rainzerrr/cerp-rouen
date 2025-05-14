import fs from 'fs';
const API_KEY = "5b3ce3597851110001cf6248717a265e1c504f209faf31ce6733c995"
const STORAGE_CERP = "600 Rue des Madeleines, 77100 Mareuil-lès-Meaux";

async function searchCoords(address, postalCode) {
    const response = await fetch(
      `https://api-adresse.data.gouv.fr/search/?q=${address}&postcode=${postalCode}&limit=1`
    );
    const data = await response.json();
    return data.features[0].geometry.coordinates;
}


// Load the CSV file synchronously
fs.readFile('data/livraison5.csv', 'utf8', async (err, data) => {
    if (err) {
        console.error('Error reading the file:', err);
        return;
    }

    const lines = data.split('\n');
    const pharmacies = [];
    pharmacies.push({
        id: 0,
        name: "CERP",
        address: STORAGE_CERP,
        postalCode: "77100",
        city: "Mareuil-lès-Meaux",
        coordinates: await searchCoords(STORAGE_CERP, "77100")
    });
    for (let i = 1; i < lines.length - 1; i++) {
        const [name, address, postalCode, city] = lines[i].split(',');
        const coordinates = await searchCoords(address, postalCode);
        pharmacies.push({
            id: i,
            name,
            address,
            postalCode,
            city, 
            coordinates
        });
    }   

    var distances = [];

    // Calculation of the distances between each pair of pharmacies
    for (let i = 0; i < pharmacies.length; i++) {
        for (let j = 0; j < pharmacies.length; j++) {
            if (i === j) {
                distances.push({
                    pharmacy1: pharmacies[i],
                    pharmacy2: pharmacies[j],
                    distance: 0,
                    duration: 0
                });
            } else {
                // API Call
                const response = await fetch(
                    `https://api.openrouteservice.org/v2/directions/driving-car?api_key=${API_KEY}&start=${pharmacies[i].coordinates}&end=${pharmacies[j].coordinates}`
                );
                const data = await response.json();
                console.log(data);
                distances.push({
                    pharmacy1: pharmacies[i],
                    pharmacy2: pharmacies[j],
                    distance: data.features[0].properties.summary.distance,
                    duration: data.features[0].properties.summary.duration
                });
            }
        }
    }

    // Creation of the CSV file
    let csv = 'id1,pharmacy1,id2,pharmacy2,distance,duration\n';
    distances.forEach((distance) => {
        csv += `${distance.pharmacy1.id},${distance.pharmacy1.name},${distance.pharmacy2.id},${distance.pharmacy2.name},${distance.distance},${distance.duration}\n`;
    });
    fs.writeFile('data/distances.csv', csv, (err) => {
        if (err) {
            console.error('Error writing the file:', err);
            return;
        }
        console.log('File written successfully');
    });
});