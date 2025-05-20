
#ifndef OLED_DISPLAY_H  // Prevent multiple inclusions
#define OLED_DISPLAY_H

#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

class OledDisplay {
private:
    Adafruit_SSD1306 display;
    uint8_t lcd_addr;

public:
    OledDisplay(uint8_t width, uint8_t height, uint8_t lcd_addr);
    void init();
    void clearAllDisplay();
    void setTextProperties(uint8_t textsize, uint8_t color);
    void SetdisplayData(int16_t posX, int16_t posY, const char* data);
    void SetdisplayData(int16_t posX, int16_t posY, uint16_t data);
    void SetdisplayData(int16_t posX, int16_t posY, uint8_t data);
    void SetdisplayData(int16_t posX, int16_t posY, double data);
    uint16_t getStringWidth(const char* str);
    void DrawLine(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void DrawIcon(int16_t x, int16_t y, const uint8_t* icon, uint8_t w, uint8_t h);
    void PrintdisplayData();
};

#endif
