const express = require("express");
const admin = require("firebase-admin");
const cors = require("cors");
const bodyParser = require("body-parser");
const moment = require("moment-timezone"); // Import moment-timezone

const app = express();
app.use(cors());
app.use(bodyParser.json());

// Load Firebase credentials from serviceAccountKey.json
const serviceAccount = require("./esp32_project_serviceAccountKey.json");

admin.initializeApp({
  credential: admin.credential.cert(serviceAccount),
  databaseURL: "https://esp32-project-ef103-default-rtdb.firebaseio.com/"
});

const db = admin.database();

// Root Endpoint (For Testing)
app.get("/", (req, res) => {
  res.send("Firebase Server is Running!");
});

// Endpoint to Receive and Store Data from ESP32
app.post("/updateData", (req, res) => {
  const sensorData = req.body;

  // Basic validation to ensure at least one field is sent
  if (Object.keys(sensorData).length === 0) {
    return res.status(400).send({ error: "No data received" });
  }

  // Generate timestamp in CST
  const timestampCST = moment().tz("America/Mexico_City").format(); // CST timezone

  // Push data to Firebase with a timestamp
  db.ref("sensorActuactorData").push({
    ...sensorData,
    timestamp: timestampCST, // Use CST timestamp
  }, (error) => {
    if (error) {
      return res.status(500).send({ error: "Error saving data to Firebase" });
    }
    res.send({ message: "Data stored successfully!" });
  });
});

// Start Express server
const PORT = process.env.PORT || 3000;
app.listen(PORT, () => {
  console.log(`Server running on port ${PORT}`);
});
