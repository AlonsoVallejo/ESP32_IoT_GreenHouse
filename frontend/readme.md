# ESP32 IoT Greenhouse Monitoring System - Frontend

This project is the frontend for the ESP32 IoT Greenhouse Monitoring System. It is designed to display real-time data from sensors and actuators, providing a user-friendly interface for monitoring and controlling the greenhouse environment. The frontend interacts with a backend server to fetch and display data.

---

## Purpose

The purpose of this project is to:
1. Provide a visual interface for monitoring sensor data (e.g., temperature, humidity, water level).
2. Display actuator statuses (e.g., lamp, pump, irrigation system).
3. Host the frontend on a public hosting platform (e.g., Firebase Hosting) for easy access and demonstration.

---

## Features

- **Real-Time Data Display**:
  - Fetches data from the backend server and displays it in grouped cards (e.g., Light, PIR, Lamp; Water, Fault, Pump; Temperature, Humidity).
  - Supports history tracking for the last 60 entries.

- **Dynamic Updates**:
  - Automatically updates the dashboard every 5 seconds with the latest data.

- **Responsive Design**:
  - Optimized for desktop and mobile devices.

- **Future Hosting**:
  - The frontend is designed to be hosted on a public platform like Firebase Hosting for easy access.

---

## Backend Integration

The frontend communicates with a backend server that:
- Receives data from the ESP32 via HTTP POST requests.
- Stores data in a Firebase Realtime Database.
- Provides endpoints for fetching the latest data and historical data:
  - `/getLastData`: Fetches the most recent data for sensors or actuators.
  - `/getHistoryData`: Fetches the last 60 entries for sensors or actuators.

### Backend Requirements
- The backend must be publicly accessible for the frontend to fetch data.
- If the backend is hosted locally, tools like `ngrok` can be used for temporary public access.

---

## Setup Instructions

### Prerequisites
- Node.js installed on your system.
- A backend server running and accessible.

### Steps
1. Clone the repository:
   ```bash
   git clone https://github.com/your-repo/ESP32_Project.git
   cd ESP32_Project/frontend
   ```
2. Install dependencies:
   ```bash
   npm install
   ```
3. Set the backend URL:
   ```javascript
   const apiUrl = "http://<your-backend-url>/getLastData";
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

### Key Updates
1. **Backend Integration**:
   - Added details about the backend endpoints and their purpose.
   - Mentioned the requirement for the backend to be publicly accessible.

2. **Hosting Purpose**:
   - Highlighted the intention to host the frontend on a public platform like Firebase Hosting.

3. **Setup Instructions**:
   - Included steps to set up the frontend and connect it to the backend.

4. **Future Improvements**:
   - Added potential enhancements for the project.