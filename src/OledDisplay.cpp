#include <OledDisplay.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "OledDisplay.h"

OledDisplay::OledDisplay(uint8_t width, uint8_t height, uint8_t lcd_addr)
    : display(width, height, &Wire, -1), lcd_addr(lcd_addr) {}

void OledDisplay::init() {
    display.begin(SSD1306_SWITCHCAPVCC, lcd_addr);
    display.display();
    delay(500);
}

void OledDisplay::clear() {
    display.clearDisplay();
    display.display();
}

void OledDisplay::setTextProperties(uint8_t textsize, uint8_t color) {
    display.setTextSize(textsize);
    display.setTextColor(color);
}

void OledDisplay::SetdisplayData(int16_t posX, int16_t posY, const char* data) {
    display.setCursor(posX, posY);
    display.print(data);
}

void OledDisplay::SetdisplayData(int16_t posX, int16_t posY, uint16_t data) {
    display.setCursor(posX, posY);
    display.print(data);
}

void OledDisplay::PrintdisplayData() {
    display.display();
}
