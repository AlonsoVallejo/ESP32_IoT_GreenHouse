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
#define MIN_PASSWORD_LENGTH (4) // Minimum password length

static const unsigned char PROGMEM Sun_Icon[] = {0x01,0x00,0x21,0x08,0x10,0x10,0x03,0x80,0x8c,0x62,0x48,0x24,0x10,0x10,0x10,0x10,0x10,0x10,0x48,0x24,0x8c,0x62,0x03,0x80,0x10,0x10,0x21,0x08,0x01,0x00,0x00,0x00};
static const unsigned char PROGMEM Moon_Icon[] = {0x04,0x00,0x1c,0x0e,0x38,0x02,0x78,0x04,0x71,0xee,0xf0,0x40,0xf0,0x80,0xf1,0xe0,0xf8,0x00,0xf8,0x06,0x7e,0x1c,0x7f,0xfc,0x3f,0xf8,0x1f,0xf0,0x07,0xc0,0x00,0x00};
static const unsigned char PROGMEM Presence_Icon[] = {0x07,0x00,0x08,0x80,0x10,0x40,0x10,0x40,0x10,0x40,0x08,0x80,0x07,0x00,0x00,0x00,0x0f,0x80,0x30,0x60,0x40,0x10,0x40,0x10,0x80,0x08,0x80,0x08,0x80,0x08,0xff,0xf8};
static const unsigned char PROGMEM Lamp_On_Icon[] = {0x02,0x00,0x07,0x00,0x05,0x00,0x08,0x80,0x08,0x80,0x08,0x80,0x30,0x60,0x40,0x10,0x80,0x08,0x7f,0xf0,0x08,0x80,0xc7,0x18,0x00,0x00,0x10,0x40,0x22,0x20,0x02,0x00};
static const unsigned char PROGMEM Lamp_Off_Icon[] = {0x02,0x00,0x07,0x00,0x05,0x00,0x08,0x80,0x08,0x80,0x08,0x80,0x30,0x60,0x40,0x10,0x80,0x08,0x7f,0xf0,0x08,0x80,0x07,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

static const unsigned char PROGMEM Lvl_Icon[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x20,0x00,0x00,0x70,0x00,0x30,0x24,0x0c,0x30,0x04,0x0c,0x30,0x10,0x0c,0x33,0xe0,0x0c,0x37,0xf8,0x2c,0x37,0xff,0xec,0x37,0xff,0xec,0x37,0xf3,0xec,0x37,0xff,0xec,0x37,0xff,0xec,0x33,0xff,0xcc,0x18,0x00,0x18,0x1f,0xff,0xf8,0x07,0xff,0xe0,0x00,0x00,0x00,0x00,0x00,0x00};
static const unsigned char PROGMEM Water_well_icon[] = {0x1f,0xff,0xf8,0x20,0x00,0x04,0x20,0x00,0x04,0x40,0x00,0x02,0x40,0x00,0x02,0xc0,0x00,0x03,0x7f,0xff,0xfe,0x08,0x08,0x10,0x08,0x08,0x10,0x08,0x08,0x10,0x09,0xff,0x90,0x08,0x81,0x10,0x08,0x81,0x10,0x08,0x81,0x10,0x08,0x81,0x10,0x08,0x42,0x10,0x08,0x42,0x10,0x08,0x7e,0x10,0x08,0x00,0x10,0x7f,0xff,0xfc,0x41,0x08,0x84,0x41,0x08,0x84,0x41,0x08,0x84,0x41,0x08,0x84};
static const unsigned char PROGMEM Pump_Icon[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x06,0x00,0x00,0x1f,0xfe,0x00,0x00,0x40,0x86,0x00,0x00,0x80,0x46,0x00,0x01,0x00,0x26,0x00,0x02,0x00,0x1e,0x00,0x02,0x12,0x16,0x00,0x02,0x22,0x10,0x00,0x02,0x20,0x10,0x00,0x1a,0x12,0x10,0x00,0x1e,0x08,0x00,0x00,0x19,0x00,0x20,0x00,0x18,0x80,0x40,0x00,0x18,0xc0,0x80,0x00,0x1f,0xfe,0x00,0x00,0x18,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

static const unsigned char PROGMEM Temperature_Icon[] = {0x1c,0x00,0x22,0x02,0x2b,0x05,0x2a,0x02,0x2b,0x38,0x2a,0x60,0x2b,0x40,0x2a,0x40,0x2a,0x60,0x49,0x38,0x9c,0x80,0xae,0x80,0xbe,0x80,0x9c,0x80,0x41,0x00,0x3e,0x00};
static const unsigned char PROGMEM Humidity_Icon[] = {0x04,0x00,0x04,0x00,0x0c,0x00,0x0e,0x00,0x1e,0x00,0x1f,0x00,0x3f,0x80,0x3f,0x80,0x7e,0xc0,0x7f,0x40,0xff,0x60,0xff,0xe0,0x7f,0xc0,0x7f,0xc0,0x3f,0x80,0x0f,0x00};
static const unsigned char PROGMEM Irrigator_Off_Icon[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x07,0x00,0x00,0x08,0xa0,0x00,0x11,0x10,0x00,0x12,0x08,0x00,0x14,0x04,0x00,0x08,0x02,0x00,0x20,0x01,0x00,0x40,0x02,0x08,0x20,0x04,0x28,0x10,0x08,0x08,0x08,0x00,0x08,0x04,0x1f,0xe8,0x02,0x20,0x18,0x01,0x40,0x00,0x00,0x80,0x00,0x00,0x00,0x00};
static const unsigned char PROGMEM Irrigator_On_Icon[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x07,0x00,0x00,0x00,0x08,0xa0,0x00,0x00,0x11,0x10,0x00,0x00,0x12,0x08,0x00,0x00,0x14,0x04,0x00,0x00,0x08,0x02,0x00,0x00,0x20,0x01,0x00,0x00,0x40,0x02,0x08,0x00,0x20,0x04,0x28,0x00,0x10,0x08,0x08,0x60,0x08,0x00,0x08,0x00,0x04,0x1f,0xe8,0x90,0x02,0x20,0x18,0x50,0x01,0x40,0x01,0x50,0x00,0x80,0x01,0x50,0x00,0x00,0x01,0x50,0x00,0x00,0x01,0x50,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

static const unsigned char PROGMEM WiFi_Connected_Icon[] = {0x01,0xf0,0x00,0x07,0xfc,0x00,0x1e,0x0f,0x00,0x39,0xf3,0x80,0x77,0xfd,0xc0,0xef,0x1e,0xe0,0x5c,0xe7,0x40,0x3b,0xfb,0x80,0x17,0x1d,0x00,0x0e,0xee,0x00,0x05,0xf4,0x00,0x03,0xb8,0x00,0x01,0x50,0x00,0x00,0xe0,0x00,0x00,0x40,0x00,0x00,0x00,0x00};
static const unsigned char PROGMEM WiFi_Not_Connected_Icon[] = {0x21,0xf0,0x00,0x16,0x0c,0x00,0x08,0x03,0x00,0x25,0xf0,0x80,0x42,0x0c,0x40,0x89,0x02,0x20,0x10,0xa1,0x00,0x23,0x58,0x80,0x04,0x24,0x00,0x08,0x52,0x00,0x01,0xa8,0x00,0x02,0x04,0x00,0x00,0x42,0x00,0x00,0xa1,0x00,0x00,0x40,0x80,0x00,0x00,0x00};
/*
 * Displays header with the current screen name.
 * @param oledDisplay Pointer to the OledDisplay object.
 * @param header The header text to display.
*/
void displayHeader(OledDisplay* oledDisplay, const char* header) {
    oledDisplay->SetdisplayData(0, 0, header);
    oledDisplay->DrawLine(0, 8, SCREEN_WIDTH, 0, SSD1306_WHITE);
}

/* Displays footer with labels for next screen and settings.
 * @param oledDisplay Pointer to the OledDisplay object.
 * @param ltbottom Label for the left bottom corner.
 * @param midbottom Label for the middle bottom corner.
 * @param rtbottom Label for the right bottom corner.
 */
void displayFooter(OledDisplay* oledDisplay, const char* ltbottom, const char* midbottom, const char* rtbottom) {
    const uint16_t displayWidth = SCREEN_WIDTH;
    const uint16_t y = 55;

    uint16_t ltWidth = oledDisplay->getStringWidth(ltbottom);
    uint16_t midWidth = oledDisplay->getStringWidth(midbottom);
    uint16_t rtWidth = oledDisplay->getStringWidth(rtbottom);

    uint16_t ltX = 0;
    uint16_t midX = (displayWidth - midWidth) / 2;
    uint16_t rtX = displayWidth - rtWidth;

    oledDisplay->DrawLine(0, y - 2, displayWidth, 0, SSD1306_WHITE);

    oledDisplay->SetdisplayData(ltX, y, ltbottom);
    oledDisplay->SetdisplayData(midX, y, midbottom);
    oledDisplay->SetdisplayData(rtX, y, rtbottom);
}

/* Displays light sensor, PIR presence, and lamp state.
 * @param data Pointer to the SystemData structure containing sensor and actuator objects.
 */
void displayLightAndPresence(SystemData* data) {
    bool lightState = data->sensorMgr->getLightSensorValue();
    uint8_t LampState =  data->actuatorMgr->getLamp()->getOutstate();
    bool PirPresenceDetected = data->PirPresenceDetected;

    displayHeader(data->oledDisplay, "Lamp Info");

    data->oledDisplay->SetdisplayData(1, 14, "Ambient");
    data->oledDisplay->DrawIcon(13, 23, lightState ? Moon_Icon : Sun_Icon, 15, 16);
    data->oledDisplay->SetdisplayData(9, 41, lightState ? "Dark" : "Light");

    data->oledDisplay->SetdisplayData(53, 14, "Motion");
    data->oledDisplay->DrawIcon(62, 23, Presence_Icon, 15, 16);
    data->oledDisplay->SetdisplayData(PirPresenceDetected ? 60 : 63, 41, PirPresenceDetected ? "YES" : "NO");

    data->oledDisplay->SetdisplayData(100, 14, "Lamp");
    data->oledDisplay->DrawIcon(104, 23, LampState ? Lamp_On_Icon : Lamp_Off_Icon, 13, 16);
    data->oledDisplay->SetdisplayData(LampState ? 106 : 102, 41, LampState ? "ON" : "OFF");

    displayFooter(data->oledDisplay, "Next", " ", "");
}

/* Displays water level and pump state.
 * @param data Pointer to the SystemData structure containing sensor and actuator objects.
 */
void displayWaterLevelAndPump(SystemData* data) {
    uint16_t levelValue = data->sensorMgr->getLevelSensorValue();
    bool wellState = data->sensorMgr->getWellSensorValue();

    displayHeader(data->oledDisplay, "Cistern info");

    data->oledDisplay->SetdisplayData(7, 11, "Well");
    data->oledDisplay->DrawIcon(6, 20, Water_well_icon, 24, 24);
    data->oledDisplay->SetdisplayData(wellState ? 4 : 8, 45, wellState ? "EMPTY" : "FULL");


    data->oledDisplay->SetdisplayData(43, 11, "Cistern");
    data->oledDisplay->DrawIcon(54, 21, Lvl_Icon, 22, 19);
    data->oledDisplay->SetdisplayData(53, 43, data->levelPercentage);
    data->oledDisplay->SetdisplayData(73, 43, "%");

    data->oledDisplay->SetdisplayData(97, 11, "Pump");
    data->oledDisplay->DrawIcon(93, 21, Pump_Icon, 26, 23);
    data->oledDisplay->SetdisplayData(97, 44, data->actuatorMgr->getPump()->getOutstate() ? "ON" : "OFF");

    displayFooter(data->oledDisplay, "Next", " " , "Settings");
}

/* Displays temperature, humidity, and irrigator state.
 * @param data Pointer to the SystemData structure containing sensor and actuator objects.
 */
void displayTemperatureAndHumidity(SystemData* data) {
    uint8_t irr_state = data->actuatorMgr->getIrrigator()->getOutstate();
    
    displayHeader(data->oledDisplay, "Irrigator Info");

    data->oledDisplay->SetdisplayData(5, 14, "Temp");
    data->oledDisplay->DrawIcon(9, 24, Temperature_Icon, 16, 16);
    data->oledDisplay->SetdisplayData(4, 43, data->sensorMgr->getTemperature());
    data->oledDisplay->SetdisplayData(28, 43, "C");

    data->oledDisplay->SetdisplayData(51, 14, "Hum");
    data->oledDisplay->DrawIcon(54, 23, Humidity_Icon, 16, 16);
    data->oledDisplay->SetdisplayData(46, 43, data->sensorMgr->getHumidity());
    data->oledDisplay->SetdisplayData(71, 43, "%");

    data->oledDisplay->SetdisplayData(90, 14, "Irrgtr");
    data->oledDisplay->DrawIcon(93, 21, irr_state ? Irrigator_On_Icon : Irrigator_Off_Icon, irr_state ? 31 : 22, irr_state ? 21 : 18);
    data->oledDisplay->SetdisplayData(100, 43, data->actuatorMgr->getIrrigator()->getOutstate() ? "ON" : "OFF");

    displayFooter(data->oledDisplay, "Next", " ", "Settings");
}

/* Displays WiFi status and connection information.
 * @param data Pointer to the SystemData structure containing sensor and actuator objects.
 */
void displayWiFiStatus(SystemData* data) {
    bool wifi_state = data->wifiManager->IsWiFiConnected();

    displayHeader(data->oledDisplay, "Network Info");
    
    data->oledDisplay->DrawIcon(52, 13, wifi_state ? WiFi_Connected_Icon : WiFi_Not_Connected_Icon, 19, 16);
    data->oledDisplay->SetdisplayData(wifi_state ? 36 : 26, 32, wifi_state ? "Connected" : "Not Connected");
    data->oledDisplay->SetdisplayData(7, 42, wifi_state ? data->wifiManager->getSSID() : "");

    displayFooter(data->oledDisplay, "Next", " ", "Settings");
}

/* Displays device information.
 * @param data Pointer to the SystemData structure containing sensor and actuator objects.
 */
void displayDeviceInfo(SystemData* data) {
    char chipIdStr[18];
    uint64_t chipId = ESP.getEfuseMac();

    displayHeader(data->oledDisplay, "Device Info");

    data->oledDisplay->SetdisplayData(0, 12, "SW Ver: ");
    data->oledDisplay->SetdisplayData(45, 12, DEV_SW_VERSION);

    data->oledDisplay->SetdisplayData(0, 22, "Dev: ");
    data->oledDisplay->SetdisplayData(30, 22, ESP.getChipModel());

    data->oledDisplay->SetdisplayData(0, 32, "DevID: ");
    snprintf(chipIdStr, sizeof(chipIdStr), "%02X:%02X:%02X:%02X:%02X:%02X",
        (uint8_t)(chipId >> 40),
        (uint8_t)(chipId >> 32),
        (uint8_t)(chipId >> 24),
        (uint8_t)(chipId >> 16),
        (uint8_t)(chipId >> 8),
        (uint8_t)chipId);
    data->oledDisplay->SetdisplayData(0, 40, chipIdStr);

    displayFooter(data->oledDisplay, "Next", " ", "");
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

    displayHeader(data->oledDisplay, "Pump settings");
    data->oledDisplay->SetdisplayData(0, 10, settings[data->currentSettingMenu]);
    data->oledDisplay->SetdisplayData(0, 20, "Set Value:");
    data->oledDisplay->SetdisplayData(65, 20, currentValue);

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

    displayHeader(data->oledDisplay, "Irrigator settings");
    data->oledDisplay->SetdisplayData(0, 10, settings[data->currentSettingMenu]);
    data->oledDisplay->SetdisplayData(0, 20, "Set Value:");
    data->oledDisplay->SetdisplayData(65, 20, currentValue);

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
    displayHeader(data->oledDisplay, "WiFi Settings");
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
    displayHeader(data->oledDisplay, "Select WiFi:");
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
                int trimmedLength = passwordLength;
                while (trimmedLength > 0 && passwordBuffer[trimmedLength - 1] == ' ') {
                    /* Trim trailing spaces before accepting password */
                    trimmedLength--;
                }
                passwordBuffer[trimmedLength] = '\0';
                if (trimmedLength >= MIN_PASSWORD_LENGTH) {
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