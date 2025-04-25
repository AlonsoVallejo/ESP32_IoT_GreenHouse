const express = require("express");
const admin = require("firebase-admin");
const cors = require("cors");
const bodyParser = require("body-parser");
const moment = require("moment-timezone");
const fetch = require("node-fetch");

const app = express();
app.use(cors());
app.use(bodyParser.json());

// Firebase credentials
let serviceAccount = require("./esp32_project_serviceAccountKey.json");

// Function to initialize Firebase
function initializeFirebase() {
  if (admin.apps.length) {
    console.log("Firebase already initialized, skipping reinitialization.");
    return admin.database();
  }
  
  console.log("Initializing Firebase...");
  admin.initializeApp({
    credential: admin.credential.cert(serviceAccount),
    databaseURL: "https://esp32-project-ef103-default-rtdb.firebaseio.com/",
  });
  console.log("Firebase initialized successfully.");
  return admin.database();
}

// Function to deinitialize Firebase
function deinitializeFirebase() {
  if (admin.apps.length) {
    console.log("Deinitializing Firebase...");
    admin.app().delete()
      .then(() => console.log("Firebase deinitialized."))
      .catch((error) => console.error("Error during Firebase deinitialization:", error));
  } else {
    console.log("Firebase is not initialized, skipping deinitialization.");
  }
}

let db = initializeFirebase(); // Initialize Firebase at startup

// Function to monitor internet connectivity
async function checkConnectivity() {
  try {
    const response = await fetch("https://www.google.com", { method: "HEAD", timeout: 5000 });
    return response.ok; // Online if status is OK
  } catch (error) {
    return false; // Offline if any error occurs
  }
}

// Monitor connectivity and manage Firebase initialization
setInterval(async () => {
  const isConnected = await checkConnectivity();

  if (!isConnected) {
    console.warn("Internet connection lost. Attempting to deinitialize Firebase...");
    deinitializeFirebase(); // Clean up Firebase resources
    db = null; // Reset database reference
  } else {
    if (!admin.apps.length) { // Only reinitialize if Firebase is not active
      console.log("Internet connection restored. Reinitializing Firebase...");
      db = initializeFirebase(); // Reinitialize Firebase
    } 
  }
}, 10000); // Check every 10 seconds

// Existing endpoints remain unchanged
app.get("/", (req, res) => {
  res.send("Firebase Server is Running!");
});

app.post("/updateData", (req, res) => {
  const sensorData = req.body;

  if (Object.keys(sensorData).length === 0) {
    return res.status(400).send({ error: "No data received" });
  }

  const timestampCST = moment().tz("America/Mexico_City").format();

  const preparedData = {
    type: sensorData.type || "unknown",
    lvl: sensorData.lvl !== undefined ? sensorData.lvl : null,
    tmp: sensorData.tmp !== undefined ? sensorData.tmp : null,
    hum: sensorData.hum !== undefined ? sensorData.hum : null,
    ldr: sensorData.ldr !== undefined ? sensorData.ldr : null,
    pir: sensorData.pir !== undefined ? sensorData.pir : null,
    lmp: sensorData.lmp !== undefined ? sensorData.lmp : null,
    pmp: sensorData.pmp !== undefined ? sensorData.pmp : null,
    flt: sensorData.flt !== undefined ? sensorData.flt : null,
    timestamp: timestampCST,
  };

  Object.keys(preparedData).forEach((key) => {
    if (preparedData[key] === null) {
      delete preparedData[key];
    }
  });

  const dbRef = sensorData.type === "actuators" ? "actuatorData" : "sensorData";
  if (db) {
    db.ref(dbRef).push(preparedData, (error) => {
      if (error) {
        return res.status(500).send({ error: "Error saving data to Firebase" });
      }
      res.send({ message: "Data stored successfully!" });
    });
  } else {
    res.status(500).send({ error: "Firebase is not initialized." });
  }
});

const PORT = process.env.PORT || 3000;
app.listen(PORT, () => {
  console.log(`Server running on port ${PORT}`);
});
