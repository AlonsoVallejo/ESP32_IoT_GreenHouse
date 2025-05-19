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
 * Endpoint to fetch the last data entry for sensors or actuators for a specific device
 * Request from frontend.
 * API endpoint: /getLastData
 * Query parameters: chipId (device MAC), type (sensors or actuators)
 */
app.get("/getLastData", async (req, res) => {
  const { chipId, type } = req.query;

  /** Validate query parameters */
  if (!chipId) {
    return res.status(400).send({ error: "Missing 'chipId' query parameter." });
  }
  if (!type || (type !== "sensors" && type !== "actuators")) {
    return res.status(400).send({ error: "Invalid or missing 'type' query parameter. Use 'sensors' or 'actuators'." });
  }

  if (db) {
    try {
      /** Fetch the most recent entry from SensActHistory */
      const snapshot = await db.ref(`devices/${chipId}/SensActHistory`).orderByKey().limitToLast(1).once("value");
      const data = snapshot.val();

      if (data) {
        const lastEntry = Object.values(data)[0];
        if (type === "sensors" && lastEntry.sensorData) {
          res.send({ ...lastEntry.sensorData, timestamp: lastEntry.timestamp });
        } else if (type === "actuators" && lastEntry.actuatorData) {
          res.send({ ...lastEntry.actuatorData, timestamp: lastEntry.timestamp });
        } else {
          res.status(404).send({ error: `No ${type} data found in the last entry.` });
        }
      } else {
        res.status(404).send({ error: "No data found." });
      }
    } catch (error) {
      console.error("Error fetching last data:", error);
      res.status(500).send({ error: "Error fetching last data from Firebase." });
    }
  } else {
    res.status(500).send({ error: "Firebase is not initialized." });
  }
});

/** 
 * Endpoint to fetch historical data (last 60 entries) for sensors or actuators
 * Request from frontend.
 * API endpoint: /getHistoryData
 * Query parameters: chipId (device MAC), type (sensors or actuators)
 */
app.get("/getHistoryData", async (req, res) => {
  const { chipId, type } = req.query;

  /** Validate query parameters */
  if (!chipId) {
    return res.status(400).send({ error: "Missing 'chipId' query parameter." });
  }
  if (!type || (type !== "sensors" && type !== "actuators")) {
    return res.status(400).send({ error: "Invalid or missing 'type' query parameter. Use 'sensors' or 'actuators'." });
  }

  const dbRef = `devices/${chipId}/SensActHistory`;

  if (db) {
    try {
      /** Fetch the last 60 entries */
      const snapshot = await db.ref(dbRef).orderByKey().limitToLast(60).once("value");
      const data = snapshot.val();

      if (data) {
        /** Map to only the requested type data */
        const result = Object.values(data)
          .map(entry => {
            if (type === "sensors" && entry.sensorData) {
              return { ...entry.sensorData, timestamp: entry.timestamp };
            } else if (type === "actuators" && entry.actuatorData) {
              return { ...entry.actuatorData, timestamp: entry.timestamp };
            } else {
              return null;
            }
          })
          .filter(entry => entry !== null)
          .reverse(); /** Most recent first */
        res.send(result);
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
 * Payload format: { "chipId": "XX:XX:XX:XX:XX:XX", "userSettings": {...} }
 */
app.post("/saveSettings", async (req, res) => {
  const { chipId, userSettings } = req.body;

  /** Validate input */
  if (!chipId || !userSettings) {
    return res.status(400).send({ error: "Invalid input data: chipId and userSettings are required." });
  }

  console.log("Received user settings for", chipId, ":", userSettings);

  if (db) {
    try {
      /** Save settings to Firebase under the device's settings path */
      await db.ref(`devices/${chipId}/settings`).set(userSettings);

      console.log("Settings saved to Firebase for", chipId, ":", userSettings);
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
      /** Fetch settings from /devices/{chipId}/settings */
      const snapshot = await db.ref(`devices/${chipId}/settings`).once("value");
      const settings = snapshot.val();

      if (settings !== null && typeof settings === "object") {
        res.send(settings); /** Send the settings object */
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
 * Endpoint to register a new device with alias
 * Request from frontend.
 * API endpoint: /registerDevice
 * Payload format: { "chipId": "unique_chip_id", "alias": "device_alias", "registeredAt": "timestamp" }
 */
app.post("/registerDevice", async (req, res) => {
  const { chipId, alias, registeredAt } = req.body;
  if (!chipId) return res.status(400).send({ error: "Missing chipId" });
  try {
    await db.ref(`RegisteredDevices/${chipId}`).set({
      alias: alias || "",
      registeredAt: registeredAt || new Date().toISOString()
    });
    res.send({ message: "Device registered successfully!" });
  } catch (error) {
    res.status(500).send({ error: "Failed to register device" });
  }
});

/** 
 * Endpoint to get all registered devices
 * Request from frontend.
 * API endpoint: /getRegisteredDevices
 * Returns a list of registered devices with their aliases and registration dates
 */
app.get("/getRegisteredDevices", async (req, res) => {
  try {
    const snapshot = await db.ref("RegisteredDevices").once("value");
    const devices = snapshot.val() || {};
    res.send(devices);
  } catch (error) {
    res.status(500).send({ error: "Failed to fetch registered devices" });
  }
});

/** 
 * Start the server on the specified port
 */
const PORT = process.env.PORT || 3000;
app.listen(PORT, () => {
  console.log(`Server running on port ${PORT}`);
});
