---

# Firebase Integration Server

This project is a backend server that integrates with Firebase for storing data received from ESP32 devices.  
It is built using `Node.js`, `Express`, and the `Firebase Admin SDK` and supports robust handling of internet connection interruptions.

---

## Features

- **Data Handling**: Receives data from ESP32 devices via HTTP POST requests and stores it in Firebase Realtime Database.
- **Irrigation Control Integration**: Supports receiving and storing irrigation system status (`ON`/`OFF`) from ESP32 devices.
- **Default Settings Management**: Automatically sends default settings to the database if no settings exist when the ESP32 connects to the backend.
- **Settings Fetching**: Allows ESP32 devices to fetch updated settings from the database every 15 seconds.
- **Connectivity Monitoring**: Periodically checks for internet connectivity and dynamically reinitializes Firebase when WiFi or internet is recovered.
- **CST Timestamp Integration**: Automatically generates timestamps in the `America/Mexico_City` timezone for accurate data logging.
- **Public Backend Exposure**: Allows the backend to be exposed to the internet using ngrok for testing and temporary public access.

---

## Installation

1. Clone the repository:
   ```bash
   git clone <repository-url>
   cd <repository-folder>
   ```

2. Install dependencies:
   ```bash
   npm install
   ```

3. Install additional required modules:
   ```bash
   npm install node-fetch moment firebase-admin express body-parser cors
   ```

4. Create and add your Firebase service account JSON credentials file as `esp32_project_serviceAccountKey.json` in the root folder.

---

## Backend Changes

### New Endpoints

#### `/getSettings`
- **Method**: GET
- **Description**: Checks if the `settings` JSON package exists in the database.
- **Response**:
  - If settings exist:
    ```json
    {
      "maxLevel": 90,
      "minLevel": 20,
      "hotTemperature": 30,
      "lowHumidity": 15
    }
    ```
  - If settings do not exist:
    ```json
    {
      "error": "No settings found in the database."
    }
    ```

#### `/saveDefaultSettings`
- **Method**: POST
- **Description**: Saves default settings to the database if no settings exist.
- **Request Body Example**:
  ```json
  {
    "defaultSettings": {
      "maxLevel": 90,
      "minLevel": 20,
      "hotTemperature": 30,
      "lowHumidity": 15
    }
  }
  ```
- **Response**:
  - Success:
    ```json
    {
      "message": "Default settings saved successfully!"
    }
    ```
  - Failure:
    ```json
    {
      "error": "Failed to save default settings."
    }
    ```

#### `/updateData`
- **Method**: POST
- **Description**: Stores sensor or actuator data in Firebase.
- **Request Body Example**:
  ```json
  {
    "type": "sensor",
    "lvl": 90,
    "tmp": 25.5,
    "hum": 60,
    "irr": 1
  }
  ```
  - `irr`: Represents the irrigation system status (`1` for `ON`, `0` for `OFF`).

#### `/getLastData`
- **Method**: GET
- **Description**: Retrieves the most recent sensor and actuator data from Firebase.
- **Response Example**:
  ```json
  {
    "sensorData": {
      "lvl": 90,
      "tmp": 25.5,
      "hum": 60
    },
    "actuatorData": {
      "irr": 1,
      "lamp": 1
    }
  }
  ```

#### `/checkConnectivity`
- **Method**: GET
- **Description**: Checks the backend's internet connectivity status.
- **Response Example**:
  ```json
  {
    "status": "connected"
  }
  ```

---

## Making the Local Backend Public with ngrok

To expose the local backend server to the internet:

1. **Install ngrok**:
   Ensure ngrok is installed globally:
   ```bash
   npm install -g ngrok
   ```

2. **Start the local backend server**:
   Run the backend locally:
   ```bash
   node server.js
   ```

3. **Start an ngrok tunnel**:
   Create a public HTTPS URL pointing to your local backend:
   ```bash
   ngrok http 3000
   ```

   Replace `3000` with the port number your backend server is listening on. ngrok will generate a public URL like `https://<random-string>.ngrok-free.app`.

4. **Handle ngrok Browser Warning**:
   To bypass ngrok's browser warning page, update your frontend API requests to include the `ngrok-skip-browser-warning` header:
   ```javascript
   headers: {
     "ngrok-skip-browser-warning": "true"
   }
   ```

5. **Enable Cross-Origin Resource Sharing (CORS)**:
   Configure your backend to allow requests from external origins. Using `cors`:
   ```javascript
   const cors = require("cors");
   app.use(cors({
     origin: "*", // Replace "*" with specific URLs if required for production
     methods: ["GET", "POST"]
   }));
   ```

6. **Update Frontend API URL**:
   Replace the local API URL in your frontend code with the generated ngrok URL:
   ```javascript
   const apiUrl = "https://<your-ngrok-url>/";
   ```

7. **Test Integration**:
   - Open the public ngrok URL directly in your browser to verify it works.
   - Test the hosted frontend using the updated ngrok URL.

---

## Usage

1. Start the server using `PM2` for process management (recommended):
   ```bash
   pm2 start server.js
   ```

2. Access the default root endpoint:
   ```
   GET http://<host>:<port>/
   ```

   - It will respond with a message: `Firebase Server is Running!`

3. Send ESP32 data to the endpoint:
   ```
   POST http://<host>:<port>/updateData
   ```
   - Send JSON payload with sensor or actuator data, including irrigation status.

4. Check for settings existence:
   ```
   GET http://<host>:<port>/getSettings
   ```

5. Save default settings:
   ```
   POST http://<host>:<port>/saveDefaultSettings
   ```

6. Retrieve the most recent data:
   ```
   GET http://<host>:<port>/getLastData
   ```

7. Check connectivity status:
   ```
   GET http://<host>:<port>/checkConnectivity
   ```

---

## Connectivity Monitoring

The server periodically checks internet connectivity every 10 seconds:
- If **connected**, it initializes Firebase if not already active.
- If **disconnected**, it deinitializes Firebase to prevent resource leaks.

---

## Project Structure

- `server.js`: Main server file containing logic for Firebase integration, irrigation control, and connectivity monitoring.
- `esp32_project_serviceAccountKey.json`: Firebase credentials file (you need to add this manually).
- `package.json`: Manages dependencies.

---

## Requirements

- **Node.js**: Version >=14
- **PM2**: For process management (optional)
- Firebase Admin SDK configured with your project's credentials.

---

## Dependencies

- [Express](https://www.npmjs.com/package/express): For server creation.
- [Firebase Admin SDK](https://www.npmjs.com/package/firebase-admin): To interface with Firebase services.
- [Moment-Timezone](https://www.npmjs.com/package/moment-timezone): For handling timezone-specific timestamps.
- [Node-Fetch](https://www.npmjs.com/package/node-fetch): For internet connectivity monitoring.
- [PM2](https://www.npmjs.com/package/pm2): For managing the server in production environments.

---

## Notes
- Firebase functions are not supported. Functions require a paid plan option.
- Ensure Firebase credentials (`esp32_project_serviceAccountKey.json`) are valid.
- Monitor server logs via PM2 to confirm connectivity changes and Firebase reinitialization.
- Make sure your ESP32 devices send valid JSON payloads to the server, including the `irr` field for irrigation status.

---
