---

# Firebase Integration Server

This backend receives data from ESP32 devices and stores it in Firebase Realtime Database using a **per-device structure** based on the ESP32 chip ID (MAC address).

---

## Features

- **Data Handling**: Receives data from ESP32 devices via HTTP POST requests and stores it in Firebase Realtime Database.
- **Per-Device Structure**: Organizes data under unique paths for each device based on its chip ID (MAC address).
- **Irrigation Control Integration**: Supports receiving and storing irrigation system status (`ON`/`OFF`) from ESP32 devices.
- **Default Settings Management**: Automatically sends default settings to the database if no settings exist when the ESP32 connects to the backend.
- **Settings Fetching**: Allows ESP32 devices to fetch updated settings from the database every 15 seconds.
- **Connectivity Monitoring**: Periodically checks for internet connectivity and dynamically reinitializes Firebase when WiFi or internet is recovered.
- **CST Timestamp Integration**: Automatically generates timestamps in the `America/Mexico_City` timezone for accurate data logging.
- **Public Backend Exposure**: Allows the backend to be exposed to the internet using ngrok for testing and temporary public access.

---

## Data Structure

All data is now organized under the path:

```
devices/
  XX:XX:XX:XX:XX:XX/
    settings: { ... }                # Unique settings for each device
    SensActHistory/
      <auto_generated_key>/
        sensorData: { ... }
        actuatorData: { ... }
        timestamp: ...
```

- **settings**: Only one JSON object per device, overwritten on update.
- **SensActHistory**: Contains timestamped sensor and actuator data entries.

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

## API Endpoints

### `/updateSettings` (POST)
- **Description**: Store or update settings for a device.
- **Payload Example**:
  ```json
  {
    "XX:XX:XX:XX:XX:XX": {
      "settings": {
        "maxLevel": 90,
        "minLevel": 20,
        "hotTemperature": 30,
        "lowHumidity": 15
      }
    }
  }
  ```
- **Result**: Stored at `/devices/XX:XX:XX:XX:XX:XX/settings`.

---

### `/updateSensActHistory` (POST)
- **Description**: Store a new sensor/actuator data entry for a device.
- **Payload Example**:
  ```json
  {
    "XX:XX:XX:XX:XX:XX": {
      "sensorData": {
        "lvl": 90,
        "tmp": 25.5,
        "hum": 60,
        "ldr": 1,
        "pir": 0
      },
      "actuatorData": {
        "lmp": 1,
        "pmp": 0,
        "flt": 0,
        "irr": 1
      }
    }
  }
  ```
- **Result**: Appended under `/devices/XX:XX:XX:XX:XX:XX/SensActHistory/`.

---

### `/getSettings` (GET)
- **Description**: Fetch settings for a device.
- **Query**: `chipId=XX:XX:XX:XX:XX:XX`
- **Response**:
  - If exists:
    ```json
    {
      "maxLevel": 90,
      "minLevel": 20,
      "hotTemperature": 30,
      "lowHumidity": 15
    }
    ```
  - If not exists:
    ```json
    {
      "error": "No settings found in the database for this device."
    }
    ```

---

### `/getLastData` (GET)
- **Description**: Fetch the latest sensor or actuator data for a device.
- **Query**: `chipId=XX:XX:XX:XX:XX:XX&type=sensors` or `type=actuators`
- **Response Example**:
  ```json
  {
    "lvl": 90,
    "tmp": 25.5,
    "hum": 60,
    "ldr": 1,
    "pir": 0,
    "timestamp": "2024-06-01T12:34:56-06:00"
  }
  ```
  or
  ```json
  {
    "lmp": 1,
    "pmp": 0,
    "flt": 0,
    "irr": 1,
    "timestamp": "2024-06-01T12:34:56-06:00"
  }
  ```

---

### `/getHistoryData` (GET)
- **Description**: Fetch the last 60 entries of a specific sensor or actuator key for a device.
- **Query**: `chipId=XX:XX:XX:XX:XX:XX&type=sensors&key=lvl`
- **Response**: Array of objects with the requested key and timestamp.

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
   POST http://<host>:<port>/updateSensActHistory
   ```
   - Send JSON payload with sensor or actuator data, including irrigation status.

4. Check for settings existence:
   ```
   GET http://<host>:<port>/getSettings?chipId=XX:XX:XX:XX:XX:XX
   ```

5. Save or update settings:
   ```
   POST http://<host>:<port>/updateSettings
   ```

6. Retrieve the most recent data:
   ```
   GET http://<host>:<port>/getLastData?chipId=XX:XX:XX:XX:XX:XX&type=sensors
   ```

7. Retrieve history data:
   ```
   GET http://<host>:<port>/getHistoryData?chipId=XX:XX:XX:XX:XX:XX&type=sensors&key=lvl
   ```

8. Check connectivity status:
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

## Example Firebase Structure

```
devices/
  AA:BB:CC:DD:EE:FF/
    settings: {
      "maxLevel": 90,
      "minLevel": 20,
      "hotTemperature": 30,
      "lowHumidity": 15
    }
    SensActHistory/
      -Nabc123xyz/
        sensorData: { ... }
        actuatorData: { ... }
        timestamp: ...
      -Nabc456uvw/
        sensorData: { ... }
        actuatorData: { ... }
        timestamp: ...
```

---

## Notes

- All endpoints now require the device chip ID (MAC address) as the key or query parameter.
- The backend and frontend must use the same chip ID format (`XX:XX:XX:XX:XX:XX`).
- Settings are unique per device and not stored globally.
- Firebase functions are not supported. Functions require a paid plan option.
- Ensure Firebase credentials (`esp32_project_serviceAccountKey.json`) are valid.
- Monitor server logs via PM2 to confirm connectivity changes and Firebase reinitialization.
- Make sure your ESP32 devices send valid JSON payloads to the server, including the `irr` field for irrigation status.

---
