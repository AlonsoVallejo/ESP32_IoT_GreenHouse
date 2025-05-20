#include <OledDisplay_classes.h>
#include <Adafruit_GFX.h>      
#include <Adafruit_SSD1306.h> 
#include <Wire.h>              

/**
 * @brief Constructor for the OledDisplay class.
 * @param width Width of the OLED display.
 * @param height Height of the OLED display.
 * @param lcd_addr I2C address of the OLED display.
 */
OledDisplay::OledDisplay(uint8_t width, uint8_t height, uint8_t lcd_addr)
    : display(width, height, &Wire, -1), lcd_addr(lcd_addr) {}

/**
 * @brief Initializes the OLED display.
 *        Sets up the display and provides a startup delay for stability.
 */
void OledDisplay::init() {
    display.begin(SSD1306_SWITCHCAPVCC, lcd_addr); /* Initialize display with I2C address */
    display.display();  /* Refresh display after initialization */
    vTaskDelay(pdMS_TO_TICKS(500)); /* Short delay for stabilization */
}

/**
 * @brief Clears all content from the display.
 */
void OledDisplay::clearAllDisplay() {
    display.clearDisplay(); 
}

/**
 * @brief Sets text properties such as size and color.
 * @param textsize Size of the text.
 * @param color Text color.
 */
void OledDisplay::setTextProperties(uint8_t textsize, uint8_t color) {
    display.setTextSize(textsize);
    display.setTextColor(color);
}

/**
 * @brief Displays a text string at a specified position.
 * @param posX X-coordinate of the text.
 * @param posY Y-coordinate of the text.
 * @param data String to display.
 */
void OledDisplay::SetdisplayData(int16_t posX, int16_t posY, const char* data) {
    /* Clear the section where the text will be updated */
    display.fillRect(posX, posY, display.width() - posX, 8, SSD1306_BLACK);
    display.setCursor(posX, posY);
    display.setTextColor(SSD1306_WHITE, SSD1306_BLACK); /* White text on black background */
    display.print(data);  /* Print the string to the display buffer */
}

/**
 * @brief Displays numerical data at a specified position.
 * @param posX X-coordinate of the data.
 * @param posY Y-coordinate of the data.
 * @param data Numerical value to display.
 */
void OledDisplay::SetdisplayData(int16_t posX, int16_t posY, uint16_t data) {
    /* Clear the section where the text will be updated */
    display.fillRect(posX, posY, display.width() - posX, 8, SSD1306_BLACK);
    display.setCursor(posX, posY);
    display.setTextColor(SSD1306_WHITE, SSD1306_BLACK); /* White text on black background */
    display.print(data);  /* Print the numerical value to the display buffer */
}

/**
 * @brief Displays numerical data at a specified position.
 * @param posX X-coordinate of the data.
 * @param posY Y-coordinate of the data.
 * @param data Numerical value to display.
 */
void OledDisplay::SetdisplayData(int16_t posX, int16_t posY, uint8_t data) {
    /* Clear the section where the text will be updated */
    display.fillRect(posX, posY, display.width() - posX, 8, SSD1306_BLACK);
    display.setCursor(posX, posY);
    display.setTextColor(SSD1306_WHITE, SSD1306_BLACK); /* White text on black background */
    display.print(data);  /* Print the numerical value to the display buffer */
}

/**
 * @brief Displays numerical data at a specified position.
 * @param posX X-coordinate of the data.
 * @param posY Y-coordinate of the data.
 * @param data Numerical value to display.
 */
void OledDisplay::SetdisplayData(int16_t posX, int16_t posY, double data) {
    /* Clear the section where the text will be updated */
    display.fillRect(posX, posY, display.width() - posX, 8, SSD1306_BLACK);
    display.setCursor(posX, posY);
    display.setTextColor(SSD1306_WHITE, SSD1306_BLACK); /* White text on black background */
    display.print(data);  /* Print the numerical value to the display buffer */
}

/**
 * @brief Draws a line on the display.
 * @param x X-coordinate of the line.
 * @param y Y-coordinate of the line.
 * @param w Width of the line.
 * @param h Height of the line.
 * @param color Color of the line.
 */
void OledDisplay::DrawLine(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    display.drawLine(x, y, x + w, y + h, color); /* Draw a line on the display */
}

/**
 * @brief Draws an icon on the display.
 * @param x X-coordinate of the icon.
 * @param y Y-coordinate of the icon.
 * @param icon Pointer to the icon data.
 * @param w Width of the icon.
 * @param h Height of the icon.
 */
void OledDisplay::DrawIcon(int16_t x, int16_t y, const uint8_t* icon, uint8_t w, uint8_t h) {
    display.drawBitmap(x, y, icon, w, h, SSD1306_WHITE); /* Draw an icon on the display */
}

/**
 * @brief Sends the buffered display data to the OLED screen.
 */
void OledDisplay::PrintdisplayData() {
    display.display(); /* Refresh the display with new data */
}

/**
 * @brief Returns the pixel width of a string using the current font and text size.
 * @param str The string to measure.
 * @return The width in pixels.
 */
uint16_t OledDisplay::getStringWidth(const char* str) {
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(str, 0, 0, &x1, &y1, &w, &h);
    return w;
}
