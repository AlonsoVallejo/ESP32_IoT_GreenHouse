# Firebase Integration Server

This project is a backend server that integrates with Firebase for storing data received from ESP32 devices. 
It is built using `Node.js`, `Express`, and the `Firebase Admin SDK` and supports robust handling of internet connection interruptions.

---

## Features

- **Data Handling**: Receives data from ESP32 devices via HTTP POST requests and stores it in Firebase Realtime Database.
- **Connectivity Monitoring**: Periodically checks for internet connectivity and dynamically reinitializes Firebase when WiFi or internet is recovered.
- **CST Timestamp Integration**: Automatically generates timestamps in the `America/Mexico_City` timezone for accurate data logging.

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
   - Send JSON payload with sensor or actuator data.

---

## Endpoint Details

### `/updateData`
- **Method**: POST
- **Description**: Stores sensor or actuator data in Firebase.
- **Request Body Example**:
  ```json
  {
    "type": "sensor",
    "lvl": 90,
    "tmp": 25.5,
    "hum": 60
  }
  ```

---

## Connectivity Monitoring

The server periodically checks internet connectivity every 10 seconds:
- If **connected**, it initializes Firebase if not already active.
- If **disconnected**, it deinitializes Firebase to prevent resource leaks.

---

## Project Structure

- `server.js`: Main server file containing logic for Firebase integration and connectivity monitoring.
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

- Ensure Firebase credentials (`esp32_project_serviceAccountKey.json`) are valid.
- Monitor server logs via PM2 to confirm connectivity changes and Firebase reinitialization.
- Make sure your ESP32 devices send valid JSON payloads to the server.

---