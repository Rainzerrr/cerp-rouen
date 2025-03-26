import fs from 'fs';
const API_KEY = "5b3ce3597851110001cf6248717a265e1c504f209faf31ce6733c995"

async function searchCity(city, postalCode) {
    const response = await fetch(
      `https://api-adresse.data.gouv.fr/search/?q=${city}&postcode=${postalCode}&limit=1`
    );
    const data = await response.json();
    return data.features[0].geometry.coordinates;
}


// Load the CSV file synchronously
fs.readFile('data/livraison10.csv', 'utf8', async (err, data) => {
    if (err) {
        console.error('Error reading the file:', err);
        return;
    }

    const lines = data.split('\n');
    const pharmacies = [];
    for (let i = 0; i < lines.length - 1; i++) {
        const [name, address, postalCode, city] = lines[i].split(',');
        const coordinates = await searchCity(city, postalCode);
        pharmacies.push({
            name,
            address,
            postalCode,
            city, 
            coordinates
        });
    }   

    var distances = [];

    // Calculation of the distances between each pair of pharmacies
    console.log(pharmacies.length);
    for (let i = 0; i < pharmacies.length; i++) {
        for (let j = i + 1; j < pharmacies.length; j++) {
            // Appel API
            const response = await fetch(
                `https://api.openrouteservice.org/v2/directions/driving-car?api_key=${API_KEY}&start=${pharmacies[i].coordinates}&end=${pharmacies[j].coordinates}`
            );
            const data = await response.json();
            distances.push({
                pharmacy1: pharmacies[i],
                pharmacy2: pharmacies[j],
                distance: data.features[0].properties.summary.distance,
                duration: data.features[0].properties.summary.duration
            });
            setTimeout(() => {}, 1000);
        }
    }

    // Creation of the CSV file
    let csv = 'pharmacy1,pharmacy2,distance,duration\n';
    distances.forEach((distance) => {
        csv += `${distance.pharmacy1.name},${distance.pharmacy2.name},${distance.distance},${distance.duration}\n`;
    });
    fs.writeFile('data/distances.csv', csv, (err) => {
        if (err) {
            console.error('Error writing the file:', err);
            return;
        }
        console.log('File written successfully');
    });
});