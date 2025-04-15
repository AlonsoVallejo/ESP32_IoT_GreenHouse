
#ifndef OLED_DISPLAY_H  // Prevent multiple inclusions
#define OLED_DISPLAY_H

#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

class OledDisplay {
private:
    Adafruit_SSD1306 display;
    uint8_t lcd_addr;

public:
    OledDisplay(uint8_t width, uint8_t height, uint8_t lcd_addr);
    void init();
    void clear();
    void setTextProperties(uint8_t textsize, uint8_t color);
    void SetdisplayData(int16_t posX, int16_t posY, const char* data);
    void SetdisplayData(int16_t posX, int16_t posY, uint16_t data);
    void PrintdisplayData();
};

#endif
