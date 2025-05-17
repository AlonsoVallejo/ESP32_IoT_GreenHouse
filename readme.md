# ESP32 IoT Greenhouse Monitoring System

This project is a **integrated IoT solution for greenhouse monitoring and automation**, built around the ESP32 SoC. The system collects real-time data from multiple sensors (light, temperature, humidity, PIR motion, and water level) and controls actuators (Lamps, relays, irrigation system, electrovales) based on configurable environmental thresholds. 

**Key features**
- **User-friendly WiFi configuration:**  
  Configure WiFi credentials directly from the device using the OLED display and push buttons. The ESP32 scans for available 2.4GHz networks, allows SSID selection, and password entry via a simple UI. Credentials are securely saved to NVS/EEPROM and auto-loaded on boot. Hardcoding credentials is also supported for developers.
- **Settings menu on device:**  
  Adjust system parameters using the onboard UI. Changes are saved locally and synchronized with the backend.
- **Robust multitasking:**  
  Uses FreeRTOS to run sensor reading, actuator control, display updates, and server communication in parallel for responsive and reliable operation.
- **OLED display:**  
  Real-time feedback of sensor readings, actuator states, WiFi status, and settings menus.
- **Backend integration:**  
  Sends sensor and actuator data to a Node.js backend, which stores information in Firebase Realtime Database with CST timestamps. The backend also manages settings and provides RESTful APIs for configuration and monitoring.
- **Automatic reconnection:**  
  If WiFi is lost, the ESP32 will automatically attempt to reconnect using saved credentials, ensuring continuous operation.
- **Manual and remote configuration:**  
  System settings can be updated manually via the device or remotely via the backend server.
- **Front-end dashboard:**  
  A responsive web dashboard (frontend) displays real-time sensor and actuator data, historical trends, and allows remote configuration of system settings. The dashboard is designed for easy access and monitoring from any device.
- **Easy deployment:**  
  Developed with PlatformIO for streamlined firmware development and deployment.

---

## Features

### ESP32 Tasks
- **Sensor Data Collection**:
  - Reads data from light, temperature, humidity, PIR, and water level sensors.
- **Actuator Control**:
  - Controls LEDs, relays, and the irrigation system based on environmental conditions.
- **OLED Display**:
  - Updates the OLED screen with real-time sensor and actuator data.
  - Includes a **Settings Menu** for manual configuration of system parameters.
- **Data Transmission**:
  - Sends sensor and actuator data to the backend server every 15 seconds.

### Settings Menu
- Allows manual configuration of key system parameters:
  - **Max Water Level (%)**
  - **Min Water Level (%)**
  - **Hot Temperature (°C)**
  - **Low Humidity (%)**
- Navigation:
  - Use the **Select** button to cycle through parameters.
  - Use the **Up** and **Down** buttons to adjust values.
  - Use the **ESC** button to save changes and exit the menu.
- Automatically saves updated settings to the backend server when exiting the menu.

### WiFi Settings Menu
- Allows user-friendly WiFi configuration and connection:
  - **Scan for available 2.4GHz WiFi networks** and display them in a scrollable list.
  - **Select a network** using the Up/Down buttons and Select to confirm.
  - **Enter password** for the selected network using Up/Down to change character, Select to move to next character, and ESC to go back or confirm.
  - **Connect to WiFi** and receive feedback ("Connecting...", "Success!", or "Failed!").
  - **Disconnect from current WiFi** if desired.
- Navigation:
  - **Up/Down**: Move through SSID list or change password character.
  - **Select**: Confirm selection or move to next character.
  - **ESC**: Go back, cancel, or confirm password entry.
- Credentials are **saved to NVS/EEPROM** after a successful connection.
- On boot, the ESP32 **automatically tries to connect with saved credentials** before showing the WiFi setup screen.

### Irrigation Control
- Automatically activates the irrigation system based on temperature and humidity thresholds.
- Includes hysteresis to prevent frequent toggling.
- Ensures stable operation by validating sensor data.

### Backend Server
- **Default Settings Management**:
  - Automatically sends default settings to the database if no settings exist when the ESP32 connects to the backend.
- **Settings Fetching**:
  - Allows the ESP32 to fetch updated settings from the database every 15 seconds.
- **Manual Settings Management**:
  - Receives updated settings from the ESP32 and overrides the current settings in the database.
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

1. **Clone this repository:**
   ```bash
   git clone https://github.com/your-repo/ESP32_Project.git
   cd ESP32_Project
   ```

2. **Open the project** in PlatformIO or Visual Studio Code.

3. **Build and upload the firmware** to the ESP32.  
   > **Note:** You can hardcode WiFi credentials in the source code if you wish, but you can also configure WiFi using the user interface with the display and push buttons.  
   On first boot (or if no credentials are saved), the ESP32 will prompt you to configure WiFi using the push buttons and OLED display.

4. **WiFi Setup via OLED and Buttons:**
   - On first boot (or if no credentials are saved), the ESP32 will scan for available 2.4GHz WiFi networks.
   - Use the **Up/Down** buttons to scroll through the list of SSIDs.
   - Press **Select** to choose a network.
   - Enter the password using Up/Down to change the character, Select to move to the next character, and ESC to go back or confirm.
   - After a successful connection, credentials are saved to NVS/EEPROM for future boots.
   - On subsequent boots, the ESP32 will automatically connect using the saved credentials.

5. **Manual WiFi Reconfiguration:**
   - To change WiFi credentials later, use the **WiFi Settings Menu** on the device.

### Backend Server

1. **Navigate to the backend folder:**
   ```bash
   cd backend
   ```

2. **Install dependencies:**
   ```bash
   npm install
   ```

3. **Start the server:**
   ```bash
   node server.js
   ```

4. **(Optional) Use `ngrok` to expose the backend server:**
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
   - Allow manual configuration of system parameters via the **Settings Menu**.
3. The backend server will:
   - Store the data in Firebase.
   - Add a timestamp in CST.
   - Override current settings in the database with updated settings received from the ESP32.

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

### `/updateSettings`
- **Method**: POST
- **Description**: Updates the current settings in the database with new values received from the ESP32.
- **Request Body Example**:
  ```json
  {
    "updatedSettings": {
      "maxLevel": 85,
      "minLevel": 25,
      "hotTemperature": 28,
      "lowHumidity": 20
    }
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