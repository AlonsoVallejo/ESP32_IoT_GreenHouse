#include <OledDisplay.h>
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
    display.begin(SSD1306_SWITCHCAPVCC, lcd_addr); // Initialize display with I2C address
    display.display();  // Refresh display after initialization
    delay(500); // Short delay for stabilization
}

/**
 * @brief Clears all content from the display.
 */
void OledDisplay::clearAllDisplay() {
    display.clearDisplay();  // Clears the buffer
    display.display();       // Refresh the display
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
    display.setTextColor(SSD1306_WHITE, SSD1306_BLACK); // White text on black background
    display.print(data);  // Print the string to the display buffer
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
    display.setTextColor(SSD1306_WHITE, SSD1306_BLACK); // White text on black background
    display.print(data);  // Print the numerical value to the display buffer
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
    display.setTextColor(SSD1306_WHITE, SSD1306_BLACK); // White text on black background
    display.print(data);  // Print the numerical value to the display buffer
}

/**
 * @brief Sends the buffered display data to the OLED screen.
 */
void OledDisplay::PrintdisplayData() {
    display.display(); // Refresh the display with new data
}
