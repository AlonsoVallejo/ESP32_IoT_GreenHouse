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
- **Device Registration & Alias**: Devices can be registered with an alias and managed from the frontend.
- **Device Removal**: Devices can be removed from the registered list via the frontend.
- **History Limiting**: Only the last 60 sensor/actuator data entries per device are kept in the database; older entries are deleted automatically.

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
RegisteredDevices/
  XX:XX:XX:XX:XX:XX/
    alias: "My Greenhouse"
    registeredAt: "2024-06-01T12:34:56Z"
```

- **settings**: Only one JSON object per device, overwritten on update.
- **SensActHistory**: Contains up to 60 timestamped sensor and actuator data entries.
- **RegisteredDevices**: List of registered devices with alias and registration date.

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
- **Description**: Store a new sensor/actuator data entry for a device. **Keeps only the last 60 entries per device.**
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
- **Result**: Appended under `/devices/XX:XX:XX:XX:XX:XX/SensActHistory/`. Oldest entries are deleted if more than 60 exist.

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

### `/registerDevice` (POST)
- **Description**: Register a new device with an alias.
- **Payload Example**:
  ```json
  {
    "chipId": "XX:XX:XX:XX:XX:XX",
    "alias": "My Greenhouse",
    "registeredAt": "2024-06-01T12:34:56Z"
  }
  ```
- **Result**: Device is added to `/RegisteredDevices`.

---

### `/getRegisteredDevices` (GET)
- **Description**: Get all registered devices with their aliases and registration dates.

---

### `/removeDevice` (POST)
- **Description**: Remove a registered device from `/RegisteredDevices`.
- **Payload Example**:
  ```json
  {
    "chipId": "XX:XX:XX:XX:XX:XX"
  }
  ```

---

## Making the Local Backend Public with ngrok

When using a local backend with a hosted frontend, use ngrok to expose your backend:

1. **Install ngrok**:
   ```bash
   npm install -g ngrok
   ```

2. **Start the local backend server**:
   ```bash
   node server.js
   ```

3. **Start an ngrok tunnel**:
   ```bash
   ngrok http 3000
   ```

4. **Update Frontend API URL**:
   Replace the local API URL in your frontend code with the generated ngrok URL:
   ```javascript
   const apiUrl = "https://<your-ngrok-url>/";
   ```

5. **Bypass ngrok Browser Warning**:
   Add the `ngrok-skip-browser-warning` header in your frontend fetch requests.

---

## Usage

- Start the server using `node server.js` or `pm2 start server.js`.
- Use the endpoints as described above for device data, settings, registration, and removal.

---

## Notes

- Only the last 60 sensor/actuator entries per device are kept in the database.
- Devices must be registered before data can be managed in the frontend.
- Device removal only affects the registration list, not the device's data in `/devices/{chipId}`.
- Ensure your ESP32 devices send valid JSON payloads to the server.

---
