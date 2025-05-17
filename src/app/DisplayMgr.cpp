#include "DisplayMgr.h"
#include <Arduino.h>

enum wifiSettings_Type {
    WIFI_SETTIGNS_MENU,
    WIFI_SETTIGNS_LIST_NETWORKS,
    WIFI_SETTIGNS_SET_PASSWORD,
    WIFI_SETTIGNS_CONNECT_FEEDBACK,
    WIFI_SETTIGNS_DISCONNECT,
};

#define HELD_BUTTON_TIME  (3000) // Time in ms to consider a button as held
#define MIN_PASSWORD_LENGTH (8) // Minimum password length

/* Displays footer with labels for next screen and settings.
 * @param oledDisplay Pointer to the OledDisplay object.
 * @param ltbottom Label for the left bottom corner.
 * @param midbottom Label for the middle bottom corner.
 * @param rtbottom Label for the right bottom corner.
 */
void displayFooter(OledDisplay* oledDisplay, const char* ltbottom, const char* midbottom, const char* rtbottom) {
    const uint16_t displayWidth = SCREEN_WIDTH;
    const uint16_t y = 50;

    uint16_t ltWidth = oledDisplay->getStringWidth(ltbottom);
    uint16_t midWidth = oledDisplay->getStringWidth(midbottom);
    uint16_t rtWidth = oledDisplay->getStringWidth(rtbottom);

    uint16_t ltX = 0;
    uint16_t midX = (displayWidth - midWidth) / 2;
    uint16_t rtX = displayWidth - rtWidth;

    oledDisplay->SetdisplayData(ltX, y, ltbottom);
    oledDisplay->SetdisplayData(midX, y, midbottom);
    oledDisplay->SetdisplayData(rtX, y, rtbottom);
}

/* Displays light sensor, PIR presence, and lamp state.
 * @param data Pointer to the SystemData structure containing sensor and actuator objects.
 */
void displayLightAndPresence(SystemData* data) {
    data->oledDisplay->SetdisplayData(0, 0, "Light Sensor: ");
    data->oledDisplay->SetdisplayData(80, 0, data->sensorMgr->getLightSensorValue() ? "Dark" : "Light");

    data->oledDisplay->SetdisplayData(0, 10, "Presence: ");
    data->oledDisplay->SetdisplayData(80, 10, data->PirPresenceDetected ? "YES" : "NO");

    data->oledDisplay->SetdisplayData(0, 20, "Lamp: ");
    data->oledDisplay->SetdisplayData(80, 20, data->actuatorMgr->getLamp()->getOutstate() ? "ON" : "OFF");

    displayFooter(data->oledDisplay, "Next", " ", "");
}

/* Displays water level and pump state.
 * @param data Pointer to the SystemData structure containing sensor and actuator objects.
 */
void displayWaterLevelAndPump(SystemData* data) {
    uint16_t levelValue = data->sensorMgr->getLevelSensorValue();

    data->oledDisplay->SetdisplayData(0, 0, "Water Level: ");
    if (levelValue >= SENSOR_LVL_OPENCKT_V) {
        data->oledDisplay->SetdisplayData(80, 0, "OPEN");
    } else if (levelValue <= SENSOR_LVL_STG_V + SENSOR_LVL_THRESHOLD_V) {
        data->oledDisplay->SetdisplayData(80, 0, "SHORT");
    } else if (levelValue >= SENSOR_LVL_OPENCKT_V - SENSOR_LVL_THRESHOLD_V) {
        data->oledDisplay->SetdisplayData(80, 0, "100%");
    } else {
        data->oledDisplay->SetdisplayData(80, 0, data->levelPercentage);
        data->oledDisplay->SetdisplayData(105, 0, "%");
    }

    data->oledDisplay->SetdisplayData(0, 10, "Pump: ");
    data->oledDisplay->SetdisplayData(80, 10, data->actuatorMgr->getPump()->getOutstate() ? "ON" : "OFF");

    displayFooter(data->oledDisplay, "Next", " " , "Settings");
}

/* Displays temperature, humidity, and irrigator state.
 * @param data Pointer to the SystemData structure containing sensor and actuator objects.
 */
void displayTemperatureAndHumidity(SystemData* data) {

    data->oledDisplay->SetdisplayData(0, 0, "Temperature: ");
    data->oledDisplay->SetdisplayData(80, 0, data->sensorMgr->getTemperature());
    data->oledDisplay->SetdisplayData(105, 0, "C");

    data->oledDisplay->SetdisplayData(0, 10, "Humidity: ");
    data->oledDisplay->SetdisplayData(80, 10, data->sensorMgr->getHumidity());
    data->oledDisplay->SetdisplayData(105, 10, "%");

    data->oledDisplay->SetdisplayData(0, 20, "Irrigator: ");
    data->oledDisplay->SetdisplayData(80, 20, data->actuatorMgr->getIrrigator()->getOutstate() ? "ON" : "OFF");

    displayFooter(data->oledDisplay, "Next", " ", "Settings");
}

/* Displays WiFi status and connection information.
 * @param data Pointer to the SystemData structure containing sensor and actuator objects.
 */
void displayWiFiStatus(SystemData* data) {

    data->oledDisplay->SetdisplayData(0, 0, "WiFi: ");
    data->oledDisplay->SetdisplayData(0, 10, data->wifiManager->getSSID());
    data->oledDisplay->SetdisplayData(0, 20, "Status: ");
    data->oledDisplay->SetdisplayData(45, 20, data->wifiManager->IsWiFiConnected() ? "Connected" : "Disconnected");
    data->oledDisplay->SetdisplayData(0, 30, "");
    displayFooter(data->oledDisplay, "Next", " ", "Settings");
}

/* Displays the current selector state.
 * @param data Pointer to the SystemData structure containing sensor and actuator objects.
 * @param currentSettingMenu The current setting being displayed.
 * @param currentValue The current value of the setting.
 */
void displayLevelSettings(SystemData* data, uint8_t currentValue) {
    const char* settings[] = {
        "Max Level (%)",
        "Min Level (%)",
    };
    data->oledDisplay->SetdisplayData(0, 0, "System settings: ");
    data->oledDisplay->SetdisplayData(0, 10, settings[data->currentSettingMenu]);
    data->oledDisplay->SetdisplayData(0, 20, "Value:");
    data->oledDisplay->SetdisplayData(50, 20, currentValue);

    displayFooter(data->oledDisplay, "Param", "^ v", "save");
}

/* Displays the current selector state.
 * @param data Pointer to the SystemData structure containing sensor and actuator objects.
 * @param currentSettingMenu The current setting being displayed.
 * @param currentValue The current value of the setting.
 */
void displayTempHumSettings(SystemData* data, uint8_t currentValue) {
    const char* settings[] = {
        "Hot Temp (C)",
        "Low Humidity (%)",
    };

    data->oledDisplay->SetdisplayData(0, 0, "System settings: ");
    data->oledDisplay->SetdisplayData(0, 10, settings[data->currentSettingMenu]);
    data->oledDisplay->SetdisplayData(0, 20, "Value:");
    data->oledDisplay->SetdisplayData(50, 20, currentValue);

    displayFooter(data->oledDisplay, "Param", "^ v", "save");
}

/* Screen to allow the user select the option in wifi settins menu.
 * The user can choose to scan for networks or disconnect from the current network.
 * Use up/down to move, select to choose, esc to exit.
 * @param data Pointer to the SystemData structure containing sensor and actuator objects.
 * @return WIFI_SETTIGNS_LIST_NETWORKS: if the user selects to scan for networks.
 *         WIFI_SETTIGNS_DISCONNECT: if the user selects to disconnect from the current network.
 *         WIFI_SETTIGNS_MENU: if the user selects to exit.
 */
wifiSettings_Type WifiSettinsMenu(SystemData* data) {
    /* Static variable to keep track of menu selection */
    static int menuIdx = 0;
    const int menuCount = 2;
    const char* menuOptions[menuCount] = {
        "Scan WiFi Networks",
        "Disconnect WiFi"
    };

    /* Read button states */
    bool selectPressed = !data->sensorMgr->getButtonSelectorValue();
    bool escPressed = !data->sensorMgr->getButtonEscValue();
    bool upPressed = !data->sensorMgr->getButtonUpValue();
    bool downPressed = !data->sensorMgr->getButtonDownValue();

    static uint32_t lastButtonTime = 0;
    uint32_t now = millis();

    /* Display menu options */
    data->oledDisplay->SetdisplayData(0, 0, "WiFi Settings:");
    for (int i = 0; i < menuCount; ++i) {
        String line = (i == menuIdx ? ">" : " ") + String(menuOptions[i]);
        data->oledDisplay->SetdisplayData(0, 10 + i * 10, line.c_str());
    }

    /* Footer for navigation hints */
    displayFooter(data->oledDisplay, "Select", "^ v", "Esc");

    /* Debounce buttons */
    if (now - lastButtonTime > 300) {
        if (upPressed) {
            menuIdx = (menuIdx - 1 + menuCount) % menuCount;
            lastButtonTime = now;
        } else if (downPressed) {
            menuIdx = (menuIdx + 1) % menuCount;
            lastButtonTime = now;
        } else if (selectPressed) {
            lastButtonTime = now;
            if (menuIdx == 0) {
                data->currentDisplayDataSelec = SCREEN_WIFI_SETT_SUB_MENU;
                return WIFI_SETTIGNS_LIST_NETWORKS;
            } else if (menuIdx == 1) {
                /* Disconnect from the current network */
                return WIFI_SETTIGNS_DISCONNECT;
            }
        } else if (escPressed) {
            lastButtonTime = now;
            return WIFI_SETTIGNS_MENU; /* Reset state machine  */
        } else {
            /* No button pressed, keep in the wifi setting main screen */
            data->currentDisplayDataSelec = SCREEN_WIFI_SETT_MENU;
        }
    }

    return WIFI_SETTIGNS_MENU;
}

/* Start scanning for available WiFi networks and display them. The user can select one to connect or do nothing(esc).
 * @param data Pointer to the SystemData structure containing sensor and actuator objects.
 * @param selected_ssid[OUT] The SSID of the selected network.
 * @return WIFI_SETTIGNS_SET_PASSWORD: if the user selects a network to connect to, otherwise WIFI_SETTIGNS_MENU.
 */
wifiSettings_Type WifiSettinsListNetworks(SystemData* data, String& selected_ssid) {
    /* Static variables to keep scan results and selection */
    static std::vector<String> ssidList;
    static int selectedIdx = 0;
    static bool scanned = false;
    static uint32_t lastButtonTime = 0;
    uint32_t now = millis();

    data->currentDisplayDataSelec = SCREEN_WIFI_SETT_SUB_MENU;

    /* Read button states */
    bool selectPressed = !data->sensorMgr->getButtonSelectorValue();
    bool escPressed = !data->sensorMgr->getButtonEscValue();
    bool upPressed = !data->sensorMgr->getButtonUpValue();
    bool downPressed = !data->sensorMgr->getButtonDownValue();

    /* Scan only once when entering this state */
    if (!scanned) {
        data->oledDisplay->SetdisplayData(0, 0, "Scanning Networks..");
        data->oledDisplay->PrintdisplayData();
        ssidList = data->wifiManager->scanNetworks();
        selectedIdx = 0;
        scanned = true;
    }
    
    /* Footer for navigation hints */
    displayFooter(data->oledDisplay, "Select", "^ v", "Esc");

    /* Debounce buttons */
    if (now - lastButtonTime > 300) {
        if (upPressed && !ssidList.empty()) {
            selectedIdx = (selectedIdx - 1 + ssidList.size()) % ssidList.size();
            lastButtonTime = now;
        } else if (downPressed && !ssidList.empty()) {
            selectedIdx = (selectedIdx + 1) % ssidList.size();
            lastButtonTime = now;
        } else if (selectPressed && !ssidList.empty()) {
            selected_ssid = ssidList[selectedIdx];
            scanned = false; /* Reset for next entry */
            lastButtonTime = now;
            return WIFI_SETTIGNS_SET_PASSWORD;
        } else if (escPressed) {
            scanned = false; /* Reset for next entry */
            lastButtonTime = now;
            data->currentDisplayDataSelec = SCREEN_WIFI_SETT_MENU; /* Return to main wifi settins */
            return WIFI_SETTIGNS_MENU;
        }
    }

    /* Display scanned SSIDs */
    data->oledDisplay->SetdisplayData(0, 0, "Select WiFi:");
    if (ssidList.empty()) {
        data->oledDisplay->SetdisplayData(0, 10, "No networks found");
    } else {
        /* Show up to 3 SSIDs, centered on selectedIdx */
        int start = selectedIdx - 1;
        if (start < 0) start = 0;
        int end = start + 3;
        if (end > (int)ssidList.size()) {
            end = ssidList.size();
            start = end - 3;
            if (start < 0) start = 0;
        }
        for (int i = start, line = 0; i < end; ++i, ++line) {
            String lineStr = (i == selectedIdx ? ">" : " ") + ssidList[i];
            data->oledDisplay->SetdisplayData(0, 10 + line * 10, lineStr.c_str());
        }
    }

    return WIFI_SETTIGNS_LIST_NETWORKS;
}

/* Allow the user to set the password for the selected SSID using Navigation buttons.
 * The password is shown in the display (not masked).
 * Use up/down to change character, select to move to next, esc to go back.
 * @param data Pointer to the SystemData structure containing sensor and actuator objects.
 * @param selected_ssid[IN] The SSID of the selected network.
 * @param password[OUT] The entered password for the selected network.
 * @return WIFI_SETTIGNS_CONNECT_FEEDBACK: if the user selects to connect to the network with the entered password.
 *         WIFI_SETTIGNS_LIST_NETWORKS: if the user selects to go back to the list of networks.
 *         WIFI_SETTIGNS_MENU: if the user selects to go back to the main menu.
 */
wifiSettings_Type WifiSettinsSetPassword(SystemData* data, String selected_ssid, String& password) {
    /* Static variables to keep password entry state */
    static char passwordBuffer[33] = {0}; /* Max 32 chars + null terminator */
    static uint8_t passwordLength = 0;
    static uint8_t cursorPosition = 0;
    static bool isInitialized = false;
    static String lastSsid = "";
    static uint32_t lastButtonTime = 0;
    uint32_t currentMillis = millis();

    data->currentDisplayDataSelec = SCREEN_WIFI_SETT_SUB_MENU;

    /* Character set for password entry */
    static const char characterSet[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()-_=+[]{};:,.<>/?";
    static const uint8_t characterSetLength = sizeof(characterSet) - 1;
    
    /* Initialize on first entry or new SSID */
    if (!isInitialized || lastSsid != selected_ssid) {
        memset(passwordBuffer, 0, sizeof(passwordBuffer));
        passwordLength = 0;
        cursorPosition = 0;
        isInitialized = true;
        lastSsid = selected_ssid;
    }

    /* Read button states */
    bool selectButtonPressed = !data->sensorMgr->getButtonSelectorValue();
    bool escButtonPressed = !data->sensorMgr->getButtonEscValue();
    bool upButtonPressed = !data->sensorMgr->getButtonUpValue();
    bool downButtonPressed = !data->sensorMgr->getButtonDownValue();

    /* Debounce buttons and handle fast scroll */
    static uint32_t upButtonPressStart = 0;
    static uint32_t downButtonPressStart = 0;
    bool upButtonHeld = false;
    bool downButtonHeld = false;

    /* Detect long press for up button */
    if (upButtonPressed) {
        if (upButtonPressStart == 0) upButtonPressStart = currentMillis;
        if (currentMillis - upButtonPressStart > HELD_BUTTON_TIME) upButtonHeld = true;
    } else {
        upButtonPressStart = 0;
    }

    /* Detect long press for down button */
    if (downButtonPressed) {
        if (downButtonPressStart == 0) downButtonPressStart = currentMillis;
        if (currentMillis - downButtonPressStart > HELD_BUTTON_TIME) downButtonHeld = true;
    } else {
        downButtonPressStart = 0;
    }

    /* Debounce buttons */
    if (currentMillis - lastButtonTime > 200) {
        if (upButtonPressed) {
            int step = upButtonHeld ? 5 : 1;
            if (passwordLength == 0) {
                passwordBuffer[0] = characterSet[0];
                passwordLength = 1;
            } else {
                char* foundChar = strchr(characterSet, passwordBuffer[cursorPosition]);
                int charIndex = foundChar ? (foundChar - characterSet) : 0;
                charIndex = (charIndex + step) % characterSetLength;
                passwordBuffer[cursorPosition] = characterSet[charIndex];
            }
            lastButtonTime = currentMillis;
        } else if (downButtonPressed) {
            int step = downButtonHeld ? 5 : 1;
            if (passwordLength == 0) {
                passwordBuffer[0] = characterSet[0];
                passwordLength = 1;
            } else {
                char* foundChar = strchr(characterSet, passwordBuffer[cursorPosition]);
                int charIndex = foundChar ? (foundChar - characterSet) : 0;
                charIndex = (charIndex - step + characterSetLength) % characterSetLength;
                passwordBuffer[cursorPosition] = characterSet[charIndex];
            }
            lastButtonTime = currentMillis;
        } else if (selectButtonPressed) {
            if (passwordLength < 32) {
                if (cursorPosition == passwordLength) {
                    /* Only add a new character if we're at the end and it's not set */
                    if (passwordBuffer[cursorPosition] == '\0') {
                        passwordBuffer[cursorPosition] = characterSet[0];
                    }
                    passwordLength++;
                }
                /* Move to next character, but do not go past passwordLength */
                if (cursorPosition < passwordLength) {
                    cursorPosition++;
                }
                if (cursorPosition > passwordLength) cursorPosition = passwordLength;
            }
            lastButtonTime = currentMillis;
        } else if (escButtonPressed) {
            if (cursorPosition > 0) {
                /* Move cursor back or delete char */
                cursorPosition--;
            } else {
                /* If password is long enough, accept it and continue */
                if (passwordLength >= MIN_PASSWORD_LENGTH) {
                    password = String(passwordBuffer);
                    isInitialized = false;
                    LogSerial("Connecting to WiFi with pass: ", true);
                    LogSerialn(password, true);
                    return WIFI_SETTIGNS_CONNECT_FEEDBACK;
                } else {
                    /* Go back to network list */
                    isInitialized = false;
                    LogSerialn("Password too short, go back to network list", true);
                    return WIFI_SETTIGNS_LIST_NETWORKS;
                }
            }
            lastButtonTime = currentMillis;
        }
    }

    /* Display UI */
    data->oledDisplay->SetdisplayData(0, 0, "SSID:");
    data->oledDisplay->SetdisplayData(40, 0, selected_ssid.c_str());
    data->oledDisplay->SetdisplayData(0, 10, "Password:");
    data->oledDisplay->SetdisplayData(0, 20, passwordBuffer);

    /* Show cursor position (underline or ^) */
    char cursorLine[33] = {0};
    for (uint8_t i = 0; i < cursorPosition; ++i) cursorLine[i] = ' ';
    cursorLine[cursorPosition] = '^';
    data->oledDisplay->SetdisplayData(0, 30, cursorLine);

    displayFooter(data->oledDisplay, "Next", "^ v", "Esc|Set");

    return WIFI_SETTIGNS_SET_PASSWORD;
}

/* Show "Connecting..." and then "Success!" or "Failed!" with reason.
 * @param data Pointer to the SystemData structure containing sensor and actuator objects.
 * @param selected_ssid The SSID of the selected network.
 * @param password The entered password for the selected network.
 * @return WIFI_SETTIGNS_SET_PASSWORD: if the user selects to set the password again.
 */
wifiSettings_Type WifiSettinsConnectFeedack(SystemData* data, String selected_ssid, String password) {
    data->currentDisplayDataSelec = SCREEN_WIFI_SETT_SUB_MENU;

    /* Read button states */
    bool escPressed = !data->sensorMgr->getButtonEscValue();

    /* Display connecting message */
    data->oledDisplay->SetdisplayData(0, 0, "Connecting to WiFi...");
    data->oledDisplay->PrintdisplayData();

    /* Attempt to connect to the selected network */
    if (data->wifiManager->connectToNetwork(selected_ssid, password)) {
        data->oledDisplay->SetdisplayData(0, 0, "Connection Success!");
        data->oledDisplay->PrintdisplayData();
    } else {
        data->oledDisplay->SetdisplayData(0, 0, "Failed!");
        data->oledDisplay->SetdisplayData(0, 10, "Check password");
        data->oledDisplay->PrintdisplayData();
    }

    displayFooter(data->oledDisplay, "Next", "^ v", "Esc|Set");

    if (escPressed) {
        return WIFI_SETTIGNS_SET_PASSWORD;
    }
    
    vTaskDelay(pdMS_TO_TICKS(1000));

    return WIFI_SETTIGNS_MENU;
}

/* Disconnect from the current WiFi network.
 * @param data Pointer to the SystemData structure containing sensor and actuator objects.
 * @return WIFI_SETTIGNS_MENU: if the user selects to go back to the main menu.
 */
wifiSettings_Type WifiSettinsDisconnect(SystemData* data) {
    data->currentDisplayDataSelec = SCREEN_WIFI_SETT_SUB_MENU;

    /* Read button states */
    bool escPressed = !data->sensorMgr->getButtonEscValue();

    /* Disconnect from the current network */
    data->oledDisplay->SetdisplayData(0, 0, "WiFi Disconnecting...");
    data->oledDisplay->PrintdisplayData();
    data->wifiManager->disconnectWiFi();
    data->oledDisplay->SetdisplayData(0, 0, "WiFi Disconnected!");
    data->oledDisplay->PrintdisplayData();

    displayFooter(data->oledDisplay, "Next", "^ v", "Esc");

    if (escPressed) {
        return WIFI_SETTIGNS_MENU;
    }

    vTaskDelay(pdMS_TO_TICKS(1000));

    return WIFI_SETTIGNS_MENU;
}

/* Displays the WiFi settings screen with a list of available SSIDs.
 * State machine: menu -> scan/list -> password -> connect feedback -> disconnect.
 */
void displayWiFiSettings(SystemData* data) {
    /* Static variables for state machine */
    static wifiSettings_Type wifiSettings = WIFI_SETTIGNS_MENU;
    static String selected_ssid = "";
    static String password = "";

    switch(wifiSettings) {
        case WIFI_SETTIGNS_MENU:
            wifiSettings = WifiSettinsMenu(data);
            break;
        case WIFI_SETTIGNS_LIST_NETWORKS:
            wifiSettings = WifiSettinsListNetworks(data, selected_ssid);
            break;
        case WIFI_SETTIGNS_SET_PASSWORD:
            wifiSettings = WifiSettinsSetPassword(data, selected_ssid, password);
            break;
        case WIFI_SETTIGNS_CONNECT_FEEDBACK:
            wifiSettings = WifiSettinsConnectFeedack(data, selected_ssid, password);
            break;
        case WIFI_SETTIGNS_DISCONNECT:
            wifiSettings = WifiSettinsDisconnect(data);
            break;
        default:
            wifiSettings = WIFI_SETTIGNS_MENU;
            break;
    }
}