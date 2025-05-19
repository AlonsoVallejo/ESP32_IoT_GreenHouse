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
 * Endpoint to receive and store data from ESP32 devices
 */
app.post("/updateData", async (req, res) => {
  console.log("Raw request body:", req.body); // Log the raw request body

  const sensorData = req.body;

  /** Validate received data */
  if (!sensorData || Object.keys(sensorData).length === 0) {
    return res.status(400).send({ error: "No data received or invalid JSON payload" });
  }

  /** Generate a timestamp in the 'America/Mexico_City' timezone */
  const timestampCST = moment().tz("America/Mexico_City").format();

  /** Prepare data to be stored in Firebase */
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
    irr: sensorData.irr !== undefined ? sensorData.irr : null,
    timestamp: timestampCST
  };

  /** Remove null values from the prepared data */
  Object.keys(preparedData).forEach((key) => {
    if (preparedData[key] === null) {
      delete preparedData[key];
    }
  });

  const dbRef = sensorData.type === "actuators" ? "actuatorData" : "sensorData"; /** Choose the correct database path */

  if (db) {
    try {
      /** Push new data to Firebase */
      const newEntryRef = await db.ref(dbRef).push(preparedData);

      /** Clean up older entries if there are more than 60 */
      const snapshot = await db.ref(dbRef).orderByKey().once("value");
      const entries = snapshot.val();

      if (entries && Object.keys(entries).length > 60) {
        const keysToDelete = Object.keys(entries).slice(0, Object.keys(entries).length - 60); /** Oldest keys */
        keysToDelete.forEach(async (key) => {
          await db.ref(`${dbRef}/${key}`).remove();
        });
      }

      res.send({ message: "Data stored successfully!" });
    } catch (error) {
      console.error("Error saving data:", error);
      res.status(500).send({ error: "Error saving data to Firebase" });
    }
  } else {
    res.status(500).send({ error: "Firebase is not initialized." });
  }
});

/** 
 * Endpoint to fetch the latest data for sensors or actuators
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
 * Endpoint to save settings
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
 * Endpoint to save settings only if they do not already exist
 */
app.post("/saveDefaultSettings", async (req, res) => {
  const { defaultSettings } = req.body;

  /** Validate input */
  if (!defaultSettings) {
    return res.status(400).send({ error: "Invalid input data" });
  }

  console.log("Received default settings:", defaultSettings);

  if (db) {
    try {
      /** Check if the 'settings' JSON package already exists */
      const snapshot = await db.ref("settings").once("value");
      const existingSettings = snapshot.val();

      if (existingSettings) {
        console.log("Settings already exist in the database. Skipping default settings save.");
        res.send({ message: "Settings already exist. Default settings were not saved." });
      } else {
        /** Save default settings to Firebase under the 'settings' path */
        await db.ref("settings").set(defaultSettings);
        console.log("Default settings saved to Firebase:", defaultSettings);
        res.send({ message: "Default settings saved successfully!" });
      }
    } catch (error) {
      console.error("Error saving default settings to Firebase:", error);
      res.status(500).send({ error: "Error saving default settings to Firebase." });
    }
  } else {
    res.status(500).send({ error: "Firebase is not initialized." });
  }
});

/**
 * Endpoint to save manual settings
 * This endpoint is currently not implemented.
 */
app.post("/saveManualSettings", async (req, res) => {
  const { manualSettings } = req.body; // Expect the "settings" key in the request body

  /** Validate input */
  if (!manualSettings) {
    return res.status(400).send({ error: "Invalid input data" });
  }

  console.log("Received manual settings:", manualSettings);

  if (db) {
    try {
      /** Override the current settings in Firebase under the 'settings' path */
      await db.ref("settings").set(manualSettings);

      console.log("Manual settings saved to Firebase:", manualSettings);
      res.send({ message: "Manual settings saved successfully!" });
    } catch (error) {
      console.error("Error saving manual settings to Firebase:", error);
      res.status(500).send({ error: "Error saving manual settings to Firebase." });
    }
  } else {
    res.status(500).send({ error: "Firebase is not initialized." });
  }
});

/** 
 * Endpoint to fetch current settings from Firebase
 */
app.get("/getSettings", async (req, res) => {
  if (db) {
    try {
      /** Fetch settings from Firebase */
      const snapshot = await db.ref("settings").once("value");
      const settings = snapshot.val();

      if (settings) {
        res.send(settings); /** Send the settings to the frontend */
      } else {
        res.status(404).send({ error: "No settings found in the database." });
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
