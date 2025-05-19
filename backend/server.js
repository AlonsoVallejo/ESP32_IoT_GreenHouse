const express = require("express");
const admin = require("firebase-admin");
const cors = require("cors");
const bodyParser = require("body-parser");
const moment = require("moment-timezone");
const fetch = require("node-fetch");

const app = express();

/** 
 * Middleware to parse JSON requests
 */
app.use(bodyParser.json());

/** Uncomment this line for local testing with the backend, comment it back before deploying */
app.use(cors());

/** 
 * Load Firebase credentials for initializing Firebase Admin SDK
 */
let serviceAccount = require("./esp32_project_serviceAccountKey.json");

/** 
 * Function to initialize Firebase Admin SDK
 */
function initializeFirebase() {
  if (admin.apps.length) {
    console.log("Firebase already initialized, skipping reinitialization.");
    return admin.database();
  }
  
  console.log("Initializing Firebase...");
  admin.initializeApp({
    credential: admin.credential.cert(serviceAccount),
    databaseURL: "https://esp32-project-ef103-default-rtdb.firebaseio.com/" /** Firebase Realtime Database URL */
  });
  console.log("Firebase initialized successfully.");
  return admin.database();
}

/** 
 * Function to deinitialize Firebase Admin SDK to free resources
 */
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

/** 
 * Initialize Firebase when the server starts
 */
let db = initializeFirebase();

/** 
 * Function to check internet connectivity by pinging Google
 */
async function checkConnectivity() {
  try {
    const response = await fetch("https://www.google.com", { method: "HEAD", timeout: 5000 });
    return response.ok; /** Return true if online */
  } catch (error) {
    return false; /** Return false if offline */
  }
}

/** 
 * Periodically check internet connectivity and manage Firebase initialization
 */
setInterval(async () => {
  const isConnected = await checkConnectivity();

  if (!isConnected) {
    console.warn("Internet connection lost. Attempting to deinitialize Firebase...");
    deinitializeFirebase(); /** Clean up Firebase resources */
    db = null; /** Reset database reference */
  } else {
    if (!admin.apps.length) { /** Reinitialize Firebase if it is not active */
      console.log("Internet connection restored. Reinitializing Firebase...");
      db = initializeFirebase(); 
    } 
  }
}, 10000); /** Check every 10 seconds */

/** 
 * Root endpoint to confirm the server is running
 */
app.get("/", (req, res) => {
  res.send("Backend Server is Running!");
});

/** 
 * Endpoint to receive and store sensor/actuator history data from ESP32 devices.
 * Request from ESP32 devices.
 * API endpoint: /updateSensActHistory
 * Payload format: { "chipId": { "sensorData": {...}, "actuatorData": {...} } }
 * 
 */
app.post("/updateSensActHistory", async (req, res) => {
  const body = req.body;
  const chipId = Object.keys(body)[0];
  const data = body[chipId];

  if (!data) {
    return res.status(400).send({ error: "Invalid payload" });
  }

  data.timestamp = moment().tz("America/Mexico_City").format();

  if (db) {
    try {
      /** Store under /devices/{chipId}/SensActHistory/ */
      await db.ref(`devices/${chipId}/SensActHistory`).push({
        sensorData: data.sensorData,
        actuatorData: data.actuatorData,
        timestamp: data.timestamp
      });
      res.send({ message: "Sensor/Actuator history stored successfully!" });
    } catch (error) {
      console.error("Error saving history:", error);
      res.status(500).send({ error: "Error saving history to Firebase" });
    }
  } else {
    res.status(500).send({ error: "Firebase is not initialized." });
  }
});

/** 
 * Endpoint to receive and store settings data from ESP32 devices.
 * Request from ESP32 devices.
 * API endpoint: /updateSettings
 * Payload format: { "chipId": { "settings": {...} } }
 */
app.post("/updateSettings", async (req, res) => {
  const body = req.body;
  const chipId = Object.keys(body)[0];
  const data = body[chipId];

  if (!data || !data.settings) {
    return res.status(400).send({ error: "Invalid payload" });
  }

  if (db) {
    try {
      /** Store under /devices/{chipId}/settings */
      await db.ref(`devices/${chipId}/settings`).set(data.settings);
      res.send({ message: "Settings stored successfully!" });
    } catch (error) {
      console.error("Error saving settings:", error);
      res.status(500).send({ error: "Error saving settings to Firebase" });
    }
  } else {
    res.status(500).send({ error: "Firebase is not initialized." });
  }
});

/** 
 * Endpoint to fetch the last data entry for sensors or actuators
 * Request from frontend.
 * API endpoint: /getLastData
 * Query parameters: type (sensors or actuators)
 */
app.get("/getLastData", async (req, res) => {
  console.log("Received request:", req.query); /** Log incoming request details */
  const { type } = req.query;

  /** Validate query parameter */
  if (!type || (type !== "sensors" && type !== "actuators")) {
    return res.status(400).send({ error: "Invalid or missing 'type' query parameter. Use 'sensors' or 'actuators'." });
  }

  const dbRef = type === "actuators" ? "actuatorData" : "sensorData";

  if (db) {
    try {
      /** Fetch the most recent entry */
      const snapshot = await db.ref(dbRef).orderByKey().limitToLast(1).once("value");
      const data = snapshot.val();

      if (data) {
        const lastEntry = Object.values(data)[0]; /** Get the most recent entry */
        res.send(lastEntry);
      } else {
        res.status(404).send({ error: "No data found." });
      }
    } catch (error) {
      console.error("Error fetching data:", error);
      res.status(500).send({ error: "Error fetching data from Firebase." });
    }
  } else {
    res.status(500).send({ error: "Firebase is not initialized." });
  }
});

/** 
 * Endpoint to fetch historical data (last 60 entries) for sensors or actuators
 * Request from frontend.
 * API endpoint: /getHistoryData
 * Query parameters: type (sensors or actuators), key (specific key to filter data)
 */
app.get("/getHistoryData", async (req, res) => {
  const { type, key } = req.query;

  /** Validate query parameters */
  if (!type || (type !== "sensors" && type !== "actuators")) {
    return res.status(400).send({ error: "Invalid or missing 'type' query parameter. Use 'sensors' or 'actuators'." });
  }
  if (!key) {
    return res.status(400).send({ error: "Missing 'key' query parameter." });
  }

  const dbRef = type === "actuators" ? "actuatorData" : "sensorData";

  if (db) {
    try {
      /** Fetch the last 60 entries */
      const snapshot = await db.ref(dbRef).orderByKey().limitToLast(60).once("value");
      const data = snapshot.val();

      if (data) {
        /** Filter the data to include only entries with the specified key */
        const filteredData = Object.values(data).filter((entry) => entry[key] !== undefined);

        /** Send the raw data to the frontend */
        res.send(filteredData.reverse()); /** Reverse to show the most recent first */
      } else {
        res.status(404).send({ error: "No data found." });
      }
    } catch (error) {
      console.error("Error fetching history data:", error);
      res.status(500).send({ error: "Error fetching history data from Firebase." });
    }
  } else {
    res.status(500).send({ error: "Firebase is not initialized." });
  }
});

/** 
 * Endpoint to save settings data from the frontend to Firebase
 * Request from frontend.
 * API endpoint: /saveSettings
 * Payload format: { "userSettings": {...} }
 */
app.post("/saveSettings", async (req, res) => {
  const { userSettings } = req.body;

  /** Validate input */
  if (!userSettings) {
    return res.status(400).send({ error: "Invalid input data" });
  }

  console.log("Received user settings:", userSettings);

  if (db) {
    try {
      /** Save settings to Firebase under the 'settings' path */
      await db.ref("settings").set(userSettings);

      console.log("Settings saved to Firebase:", userSettings);
      res.send({ message: "Settings saved successfully!" });
    } catch (error) {
      console.error("Error saving settings to Firebase:", error);
      res.status(500).send({ error: "Error saving settings to Firebase." });
    }
  } else {
    res.status(500).send({ error: "Firebase is not initialized." });
  }
});

/** 
 * Endpoint to fetch current settings for a specific device from Firebase.
 * Request from frontend and ESP32 device.
 * API endpoint: /getSettings
 * Query parameters: chipId (unique identifier for the device)
 */
app.get("/getSettings", async (req, res) => {
  const chipId = req.query.chipId;
  if (!chipId) {
    return res.status(400).send({ error: "Missing 'chipId' query parameter." });
  }

  if (db) {
    try {
      /** Fetch settings from Firebase for this device */
      const snapshot = await db.ref(`devices/${chipId}/settings`).once("value");
      const settings = snapshot.val();

      if (settings) {
        res.send(settings); /** Send the settings to the frontend */
      } else {
        res.status(404).send({ error: "No settings found in the database for this device." });
      }
    } catch (error) {
      console.error("Error fetching settings from Firebase:", error);
      res.status(500).send({ error: "Error fetching settings from Firebase." });
    }
  } else {
    res.status(500).send({ error: "Firebase is not initialized." });
  }
});

/** 
 * Start the server on the specified port
 */
const PORT = process.env.PORT || 3000;
app.listen(PORT, () => {
  console.log(`Server running on port ${PORT}`);
});
