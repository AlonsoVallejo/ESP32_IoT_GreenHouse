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

const {onRequest} = require("firebase-functions/v2/https");
const logger = require("firebase-functions/logger");
const functions = require("firebase-functions");
const admin = require("firebase-admin");
const moment = require("moment-timezone");

admin.initializeApp(); // Initializes automatically with Firebase Functions
const db = admin.database();


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

exports.updateData = functions.https.onRequest(async (req, res) => {
  const sensorData = req.body;

  if (!sensorData || Object.keys(sensorData).length === 0) {
      return res.status(400).send({ error: "No data received or invalid JSON payload" });
  }

  const timestampCST = moment().tz("America/Mexico_City").format();
  const preparedData = {
      type: sensorData.type || "unknown",
      lvl: sensorData.lvl !== undefined ? sensorData.lvl : null,
      tmp: sensorData.tmp !== undefined ? sensorData.tmp : null,
      hum: sensorData.hum !== undefined ? sensorData.hum : null,
      // Continue for other fields...
      timestamp: timestampCST,
  };

  Object.keys(preparedData).forEach(key => {
      if (preparedData[key] === null) {
          delete preparedData[key];
      }
  });

  const dbRef = sensorData.type === "actuators" ? "actuatorData" : "sensorData";

  try {
      const newEntryRef = await db.ref(dbRef).push(preparedData);

      // Cleanup old entries if more than 60
      const snapshot = await db.ref(dbRef).orderByKey().once("value");
      const entries = snapshot.val();

      if (entries && Object.keys(entries).length > 60) {
          const keysToDelete = Object.keys(entries).slice(0, Object.keys(entries).length - 60);
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

exports.getLastData = functions.https.onRequest(async (req, res) => {
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
      res.send(filteredData.reverse()); /** Reverse to show the most recent first */
    } else {
      res.status(404).send({ error: "No data found." });
    }
  } catch (error) {
    console.error("Error fetching history data:", error);
    res.status(500).send({ error: "Error fetching history data from Firebase." });
  }
});


exports.getHistoryData = functions.https.onRequest(async (req, res) => {
  const { type } = req.query;

  if (!type || (type !== "sensors" && type !== "actuators")) {
      return res.status(400).send({ error: "Invalid or missing 'type' query parameter. Use 'sensors' or 'actuators'." });
  }

  const dbRef = type === "actuators" ? "actuatorData" : "sensorData";

  try {
      const snapshot = await db.ref(dbRef).orderByKey().limitToLast(60).once("value");
      const data = snapshot.val();

      if (data) {
          const dataList = Object.values(data).reverse();
          res.send(dataList);
      } else {
          res.status(404).send({ error: "No data found." });
      }
  } catch (error) {
      console.error("Error fetching history data:", error);
      res.status(500).send({ error: "Error fetching history data from Firebase." });
  }
});

const PORT = process.env.PORT || 3000;
app.listen(PORT, () => {
  console.log(`Server running on port ${PORT}`);
});
