# ESP32 IoT Greenhouse Monitoring System

Developed an IoT solution for greenhouse monitoring using an ESP32 microcontroller. The system collects real-time data from sensors (light, temperature, humidity, PIR, water level) and controls actuators (LEDs, relays, irrigation system) based on environmental conditions. Data is displayed on an OLED screen and sent to a backend server built with Node.js, which integrates with Firebase Realtime Database for storage and timestamping. The project leverages FreeRTOS for multitasking, PlatformIO for firmware development, and RESTful APIs for communication. Designed for efficient monitoring and automation of greenhouse environments.

---

## Features

### ESP32 Tasks
- **Sensor Data Collection**:
  - Reads data from light, temperature, humidity, PIR, and water level sensors.
- **Actuator Control**:
  - Controls LEDs, relays, and the irrigation system based on environmental conditions.
- **OLED Display**:
  - Updates the OLED screen with real-time sensor and actuator data.
- **Data Transmission**:
  - Sends sensor and actuator data to the backend server every 15 seconds.

### Irrigation Control
- Automatically activates the irrigation system based on temperature and humidity thresholds.
- Includes hysteresis to prevent frequent toggling.
- Ensures stable operation by validating sensor data.

### Backend Server
- **Default Settings Management**:
  - Automatically sends default settings to the database if no settings exist when the ESP32 connects to the backend.
- **Settings Fetching**:
  - Allows the ESP32 to fetch updated settings from the database every 15 seconds.
- **Data Storage**:
  - Receives data from the ESP32 via HTTP POST requests and stores it in Firebase Realtime Database.
- **Timestamping**:
  - Generates timestamps in Central Standard Time (CST).

---

## Requirements

### Hardware
- ESP32 microcontroller
- Sensors:
  - Light sensor
  - Temperature and humidity sensor
  - PIR motion sensor
  - Water level sensor
- Actuators:
  - LED
  - Relays
  - Irrigation system (e.g., water pump or solenoid valve)
- OLED display

### Software
- [PlatformIO](https://platformio.org/) (for ESP32 development)
- Node.js (for the backend server)
- Firebase Realtime Database

---

## Setup Instructions

### ESP32 Firmware

1. Clone this repository:
   ```bash
   git clone https://github.com/your-repo/ESP32_Project.git
   cd ESP32_Project
   ```

2. Open the project in PlatformIO or Visual Studio Code.

3. Configure WiFi credentials:
   ```cpp
   const char* ssid = "YOUR_WIFI_SSID";
   const char* password = "YOUR_WIFI_PASSWORD";
   ```

4. Build and upload the firmware to the ESP32.

### Backend Server

1. Navigate to the backend folder:
   ```bash
   cd backend
   ```

2. Install dependencies:
   ```bash
   npm install
   ```

3. Start the server:
   ```bash
   node server.js
   ```

4. Use `ngrok` to expose the backend server if needed:
   ```bash
   ngrok http 3000
   ```

---

## Usage

1. Power on the ESP32 and ensure it connects to WiFi.
2. The ESP32 will:
   - Check if the `settings` JSON package exists in the database.
   - Send default settings to the backend if no settings exist.
   - Fetch updated settings every 15 seconds.
   - Read sensor data and update the OLED display.
   - Send sensor and actuator data to the backend server every 15 seconds.
3. The backend server will:
   - Store the data in Firebase.
   - Add a timestamp in CST.

---

## Backend Endpoints

### `/getSettings`
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

### `/saveDefaultSettings`
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

### `/updateData`
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

---

## Common Issues

### ESP32 Resets Frequently
- Ensure sufficient stack size for tasks.
- Check for blocking code in FreeRTOS tasks.

### Cannot Connect to WiFi
- Verify WiFi credentials in `main.cpp`.
- Ensure the ESP32 is within range of the router.

### Cannot POST to `/updateData`
- Ensure the backend server is running.
- Test the endpoint using curl or Postman.

### Firebase Errors
- Verify Firebase credentials in `serviceAccountKey.json`.
- Check Firebase rules for read/write access.

---

## Notes
- Ensure the backend server is publicly accessible if the frontend is hosted on Firebase.
- Use `ngrok` for temporary public access to the backend during testing.
- Monitor server logs for connectivity and data storage issues.

---