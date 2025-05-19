---

# ESP32 IoT Greenhouse Monitoring System - Frontend

This project is the frontend for the ESP32 IoT Greenhouse Monitoring System. It is designed to display real-time data from sensors and actuators, providing a user-friendly interface for monitoring and controlling the greenhouse environment. The frontend interacts with a backend server to fetch and display data.

---

## Purpose

The purpose of this project is to:
1. Provide a visual interface for monitoring sensor data (e.g., temperature, humidity, water level).
2. Display actuator statuses (e.g., lamp, pump, irrigation system).
3. Allow users to register, select, and remove devices by Chip ID and alias.
4. Host the frontend on a public hosting platform (e.g., Firebase Hosting) for easy access and demonstration.

---

## Features

- **Device Registration & Selection**:
  - Users can register new devices by Chip ID and assign an alias.
  - Devices can be selected from a dropdown to view their data.
  - Devices can be removed from the registered list.

- **Real-Time Data Display**:
  - Fetches data from the backend server and displays it in grouped cards (e.g., Light, PIR, Lamp; Water, Fault, Pump; Temperature, Humidity).
  - Supports history tracking for the last 60 entries (only the last 60 are kept in the database).

- **Dynamic Updates**:
  - Automatically updates the dashboard every 5 seconds with the latest data.

- **Responsive Design**:
  - Optimized for desktop and mobile devices.

- **Settings Management**:
  - Fetches and displays device settings.
  - Allows users to update settings for the selected device.

- **Friendly Error Handling**:
  - If a device has no data, the dashboard and settings display a clear message to the user.

---

## Backend Integration

The frontend communicates with a backend server that:
- Receives data from the ESP32 via HTTP POST requests.
- Stores data in a Firebase Realtime Database.
- Provides endpoints for fetching the latest data, historical data, settings, and device registration/removal:
  - `/getLastData`: Fetches the most recent data for sensors or actuators.
  - `/getHistoryData`: Fetches the last 60 entries for sensors or actuators.
  - `/getSettings`: Fetches the current settings from the backend.
  - `/saveSettings`: Saves user settings to the backend.
  - `/registerDevice`: Registers a new device with alias.
  - `/getRegisteredDevices`: Gets all registered devices.
  - `/removeDevice`: Removes a registered device.

### Backend Requirements
- The backend must be publicly accessible for the frontend to fetch data.
- If the backend is hosted locally, tools like `ngrok` can be used for temporary public access.

---

## Using ngrok for Backend Accessibility

When the backend is hosted locally and the frontend is deployed on Firebase, use `ngrok` to expose the local backend to the internet. 
This ensures compatibility with the hosted frontend.

### Steps to Use ngrok:
1. **Install ngrok**:
   ```bash
   npm install -g ngrok
   ```

2. **Run the local backend server**:
   Start your backend server. For example:
   ```bash
   node server.js
   ```

3. **Create an ngrok tunnel**:
   Run the following command to create a public URL pointing to your backend:
   ```bash
   ngrok http 3000
   ```
   Replace `3000` with your backend's port number. ngrok will provide a public URL like `https://<random-string>.ngrok-free.app`.

4. **Update the frontend API URL**:
   Replace the local API URL in your frontend code with the generated ngrok public URL:
   ```javascript
   const apiUrl = "https://<your-ngrok-url>/";
   ```

5. **Bypass ngrok Browser Warning**:
   To prevent the ngrok browser warning page from interrupting requests, include the following header in your fetch requests:
   ```javascript
   headers: {
     "ngrok-skip-browser-warning": "true"
   }
   ```

6. **Test End-to-End Integration**:
   - Deploy the updated frontend to Firebase Hosting:
     ```bash
     firebase deploy
     ```
   - Open the Firebase-hosted frontend (e.g., `https://<your-firebase-app>.web.app`).
   - Ensure the frontend successfully fetches data from the ngrok-exposed backend.

---

## Setup Instructions

### Prerequisites
- Node.js installed on your system.
- A backend server running locally.
- Firebase CLI installed globally.

### Steps
1. Clone the repository:
   ```bash
   git clone https://github.com/your-repo/ESP32_Project.git
   cd ESP32_Project/frontend
   ```

2. Install dependencies (if any):
   ```bash
   npm install
   ```

3. Set the backend URL:
   ```javascript
   const apiUrl = "https://<your-ngrok-url>/";
   ```

4. Initialize Firebase Hosting:
   ```bash
   firebase init hosting
   ```

5. Deploy the frontend:
   ```bash
   firebase deploy
   ```

---

## Key Considerations When Using ngrok
1. **ngrok URL Changes**:
   - Free-tier ngrok URLs are temporary and will change if the tunnel restarts.
   - Each time you restart ngrok, update the frontend `apiUrl` with the new URL and redeploy to Firebase.

2. **CORS Configuration**:
   - Ensure the backend is configured to allow requests from the Firebase-hosted frontend. Use the correct `origin` in the `cors` middleware.

3. **ngrok Header**:
   - Use the `ngrok-skip-browser-warning` header in the frontend requests to bypass the browser warning.

4. **Deployment Workflow**:
   - Always test the end-to-end setup after updating the `apiUrl` in the frontend and redeploying to Firebase Hosting.

5. **Temporary Solution**:
   - Using ngrok is a temporary fix until Firebase Functions are deployed. A paid Firebase plan is required for this.

---

## Features Recap

- **Device Registration & Selection**: Register, select, and remove devices by Chip ID and alias.
- **Real-Time Dashboard**: Provides up-to-date sensor and actuator information.
- **Settings Management**: Fetch and update device settings.
- **Friendly Error Handling**: Clear messages when a device has no data.
- **History Limiting**: Only the last 60 entries per device are kept in the database.
- **Cross-Origin Compatibility**: Designed to work seamlessly with Firebase Hosting and ngrok.
- **Responsive Design**: Optimized for both desktop and mobile devices.

---
