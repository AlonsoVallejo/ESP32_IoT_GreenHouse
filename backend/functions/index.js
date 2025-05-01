/**
 * Import function triggers from their respective submodules:
 *
 * const {onCall} = require("firebase-functions/v2/https");
 * const {onDocumentWritten} = require("firebase-functions/v2/firestore");
 *
 * See a full list of supported triggers at https://firebase.google.com/docs/functions
 */

// Create and deploy your first functions
// https://firebase.google.com/docs/functions/get-started

// exports.helloWorld = onRequest((request, response) => {
//   logger.info("Hello logs!", {structuredData: true});
//   response.send("Hello from Firebase!");
// });

const { onRequest } = require("firebase-functions/v2/https");
const admin = require("firebase-admin");
const moment = require("moment-timezone");

admin.initializeApp(); // Initializes automatically with Firebase Functions
const db = admin.database();

/**
 * Endpoint to receive and store data from ESP32 devices
 */
exports.updateData = onRequest(async (req, res) => {
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
    timestamp: timestampCST,
  };

  /** Remove null values from the prepared data */
  Object.keys(preparedData).forEach((key) => {
    if (preparedData[key] === null) {
      delete preparedData[key];
    }
  });

  const dbRef = sensorData.type === "actuators" ? "actuatorData" : "sensorData";

  try {
    /** Push new data to Firebase */
    const newEntryRef = await db.ref(dbRef).push(preparedData);

    /** Clean up older entries if there are more than 60 */
    const snapshot = await db.ref(dbRef).orderByKey().once("value");
    const entries = snapshot.val();

    if (entries && Object.keys(entries).length > 60) {
      const keysToDelete = Object.keys(entries).slice(0, Object.keys(entries).length - 60); // Oldest keys
      keysToDelete.forEach(async (key) => {
        await db.ref(`${dbRef}/${key}`).remove();
      });
    }

    res.send({ message: "Data stored successfully!" });
  } catch (error) {
    console.error("Error saving data:", error);
    res.status(500).send({ error: "Error saving data to Firebase" });
  }
});

/**
 * Endpoint to fetch the latest data for sensors or actuators
 */
exports.getLastData = onRequest(async (req, res) => {
  console.log("Received request:", req.query); // Log incoming request details
  const { type } = req.query;

  /** Validate query parameter */
  if (!type || (type !== "sensors" && type !== "actuators")) {
    return res.status(400).send({ error: "Invalid or missing 'type' query parameter. Use 'sensors' or 'actuators'." });
  }

  const dbRef = type === "actuators" ? "actuatorData" : "sensorData";

  try {
    /** Fetch the most recent entry */
    const snapshot = await db.ref(dbRef).orderByKey().limitToLast(1).once("value");
    const data = snapshot.val();

    if (data) {
      const lastEntry = Object.values(data)[0]; // Get the most recent entry
      res.send(lastEntry);
    } else {
      res.status(404).send({ error: "No data found." });
    }
  } catch (error) {
    console.error("Error fetching data:", error);
    res.status(500).send({ error: "Error fetching data from Firebase." });
  }
});

/**
 * Endpoint to fetch historical data (last 60 entries) for sensors or actuators
 */
exports.getHistoryData = onRequest(async (req, res) => {
  const { type, key } = req.query;

  /** Validate query parameters */
  if (!type || (type !== "sensors" && type !== "actuators")) {
    return res.status(400).send({ error: "Invalid or missing 'type' query parameter. Use 'sensors' or 'actuators'." });
  }
  if (!key) {
    return res.status(400).send({ error: "Missing 'key' query parameter." });
  }

  const dbRef = type === "actuators" ? "actuatorData" : "sensorData";

  try {
    /** Fetch the last 60 entries */
    const snapshot = await db.ref(dbRef).orderByKey().limitToLast(60).once("value");
    const data = snapshot.val();

    if (data) {
      /** Filter the data to include only entries with the specified key */
      const filteredData = Object.values(data).filter((entry) => entry[key] !== undefined);

      /** Send the raw data to the frontend */
      res.send(filteredData.reverse()); // Reverse to show the most recent first
    } else {
      res.status(404).send({ error: "No data found." });
    }
  } catch (error) {
    console.error("Error fetching history data:", error);
    res.status(500).send({ error: "Error fetching history data from Firebase." });
  }
});

/**
 * Endpoint to save settings
 */
exports.saveSettings = onRequest(async (req, res) => {
  const { userSettings } = req.body;

  /** Validate input */
  if (!userSettings) {
    return res.status(400).send({ error: "Invalid input data" });
  }

  console.log("Received user settings:", userSettings);

  try {
    /** Save settings to Firebase under the 'settings' path */
    await db.ref("settings").set(userSettings);

    console.log("Settings saved to Firebase:", userSettings);
    res.send({ message: "Settings saved successfully!" });
  } catch (error) {
    console.error("Error saving settings to Firebase:", error);
    res.status(500).send({ error: "Error saving settings to Firebase." });
  }
});

/**
 * Endpoint to fetch current settings from Firebase
 */
exports.getSettings = onRequest(async (req, res) => {
  try {
    /** Fetch settings from Firebase */
    const snapshot = await db.ref("settings").once("value");
    const settings = snapshot.val();

    if (settings) {
      res.send(settings); // Send the settings to the frontend
    } else {
      res.status(404).send({ error: "No settings found in the database." });
    }
  } catch (error) {
    console.error("Error fetching settings from Firebase:", error);
    res.status(500).send({ error: "Error fetching settings from Firebase." });
  }
});
