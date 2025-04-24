# ESP32 IoT Greenhouse Monitoring System

Developed an IoT solution for greenhouse monitoring using an ESP32 microcontroller. The system collects real-time data from sensors (light, temperature, humidity, PIR, water level) and controls actuators (LEDs, relays) based on environmental conditions. Data is displayed on an OLED screen and sent to a backend server built with Node.js, which integrates with Firebase Realtime Database for storage and timestamping. The project leverages FreeRTOS for multitasking, PlatformIO for firmware development, and RESTful APIs for communication. Designed for efficient monitoring and automation of greenhouse environments.

## Features

- **ESP32 Tasks**:
  - Read sensor data (light, temperature, humidity, PIR, water level).
  - Control actuators (LED, relays).
  - Update an OLED display with real-time data.
  - Send sensor and actuator data to a backend server.

- **Backend Server**:
  - Receives data from the ESP32 via HTTP POST requests.
  - Stores data in a Firebase Realtime Database.
  - Generates timestamps in Central Standard Time (CST).

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
- OLED display

### Software
- [PlatformIO](https://platformio.org/) (for ESP32 development)
- Node.js (for the backend server)
- Firebase Realtime Database

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

### Backend Server

1. Install dependencies:
   ```bash
   npm install
   ```

2. Start the server:
   ```bash
   node server.js
   ```
Usage
1. Power on the ESP32 and ensure it connects to WiFi.
2. The ESP32 will:
        Read sensor data.
        Update the OLED display.
        Send data to the backend server every 5 seconds.
3. The backend server will:
        Store the data in Firebase.
        Add a timestamp in CST.

Common Issues
ESP32 Resets Frequently:
Ensure sufficient stack size for tasks.
Check for blocking code in FreeRTOS tasks.
Cannot Connect to WiFi:

Verify WiFi credentials in main.cpp.
Ensure the ESP32 is within range of the router.
Cannot POST to /updateData:

Ensure the backend server is running.
Test the endpoint using curl or Postman.
Firebase Errors:

Verify Firebase credentials in serviceAccountKey.json.
Check Firebase rules for read/write access.