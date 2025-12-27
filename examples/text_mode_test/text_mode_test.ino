// Simple Text Mode Test
// Tests text mode rendering by writing characters directly via Wishbone
// This bypasses the MCP server to verify the FPGA text pipeline

#include <Arduino.h>
#include <SPI.h>
#include "WishboneSPI.h"

// ESP32-S3 SPI pins for FPGA connection
#define SPI_MOSI 11
#define SPI_MISO 13
#define SPI_SCK  12
#define SPI_CS   10

// Text mode registers (matching HDMIController.h)
#define VIDEO_MODE_REG  0x0000
#define TEXT_CURSOR_X   0x0021
#define TEXT_CURSOR_Y   0x0022
#define TEXT_COLOR      0x0023
#define TEXT_CHAR       0x0024
#define TEXT_CLEAR      0x0025

// Video modes (matching FPGA implementation)
#define MODE_TEST_PATTERN 0  // Color bars, grid, grayscale
#define MODE_TEXT         1  // 80x26 text mode
#define MODE_FRAMEBUFFER  2  // 160x120 pixel mode

// CGA Colors
#define COLOR_BLACK       0
#define COLOR_BLUE        1
#define COLOR_GREEN       2
#define COLOR_CYAN        3
#define COLOR_RED         4
#define COLOR_MAGENTA     5
#define COLOR_BROWN       6
#define COLOR_LIGHT_GRAY  7
#define COLOR_DARK_GRAY   8
#define COLOR_LIGHT_BLUE  9
#define COLOR_LIGHT_GREEN 10
#define COLOR_LIGHT_CYAN  11
#define COLOR_LIGHT_RED   12
#define COLOR_LIGHT_MAGENTA 13
#define COLOR_YELLOW      14
#define COLOR_WHITE       15

SPIClass fpgaSPI(HSPI);

void setCursor(uint8_t x, uint8_t y) {
    wishboneWrite16(TEXT_CURSOR_X, x & 0x7F);
    wishboneWrite16(TEXT_CURSOR_Y, y & 0x1F);
}

void setColor(uint8_t fg, uint8_t bg) {
    uint8_t attr = ((bg & 0x0F) << 4) | (fg & 0x0F);
    wishboneWrite16(TEXT_COLOR, attr);
}

void writeChar(char c) {
    wishboneWrite16(TEXT_CHAR, c);
}

void writeString(const char* str) {
    while (*str) {
        writeChar(*str++);
    }
}

void clearScreen() {
    wishboneWrite16(TEXT_CLEAR, 0x01);
    delay(10);  // Give FPGA time to clear
}

void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 3000);
    
    Serial.println();
    Serial.println("=== Text Mode Test ===");
    Serial.println("Testing FPGA text rendering pipeline");
    Serial.println();
    
    // Initialize SPI
    fpgaSPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, SPI_CS);
    wishboneInit(&fpgaSPI, SPI_CS);
    
    // Set text mode
    Serial.println("Setting video mode to TEXT (mode 3)...");
    wishboneWrite16(VIDEO_MODE_REG, MODE_TEXT);
    delay(100);
    
    // Clear screen
    Serial.println("Clearing screen...");
    clearScreen();
    delay(100);
    
    // Draw a colorful border
    setColor(COLOR_LIGHT_BLUE, COLOR_BLUE);
    for (int x = 0; x < 80; x++) {
        setCursor(x, 0);
        writeChar(205);  // Double horizontal line
        setCursor(x, 24);
        writeChar(205);
    }
    for (int y = 1; y < 24; y++) {
        setCursor(0, y);
        writeChar(186);  // Double vertical line
        setCursor(79, y);
        writeChar(186);
    }
    // Corners
    setCursor(0, 0);
    writeChar(201);   // Top-left
    setCursor(79, 0);
    writeChar(187);   // Top-right
    setCursor(0, 24);
    writeChar(200);   // Bottom-left
    setCursor(79, 24);
    writeChar(188);   // Bottom-right
    
    // Title banner
    setColor(COLOR_YELLOW, COLOR_BLUE);
    setCursor(25, 2);
    writeString("PAPILIO ARCADE - TEXT MODE");
    
    setColor(COLOR_WHITE, COLOR_BLACK);
    setCursor(28, 4);
    writeString("80x26 Character Display");
    
    // Color palette demo
    setColor(COLOR_LIGHT_GREEN, COLOR_BLACK);
    setCursor(5, 7);
    writeString("CGA Color Palette:");
    
    const char* colorNames[] = {
        "Black", "Blue", "Green", "Cyan", 
        "Red", "Magenta", "Brown", "LtGray",
        "DkGray", "LtBlue", "LtGreen", "LtCyan",
        "LtRed", "LtMag", "Yellow", "White"
    };
    
    for (int i = 0; i < 16; i++) {
        int row = 9 + (i / 4);
        int col = 5 + (i % 4) * 18;
        setColor(i, COLOR_BLACK);
        setCursor(col, row);
        writeChar(219);  // Solid block
        writeChar(219);
        writeChar(' ');
        writeString(colorNames[i]);
    }
    
    // Feature list
    setColor(COLOR_LIGHT_CYAN, COLOR_BLACK);
    setCursor(5, 15);
    writeString("Features:");
    
    setColor(COLOR_WHITE, COLOR_BLACK);
    setCursor(7, 17);
    writeChar(175);  // Arrow
    writeString(" 720p HDMI Output");
    setCursor(7, 18);
    writeChar(175);
    writeString(" ESP32-S3 + Gowin FPGA");
    setCursor(7, 19);
    writeChar(175);
    writeString(" Wishbone Bus over SPI");
    setCursor(7, 20);
    writeChar(175);
    writeString(" Real-time Text Rendering");
    
    // Credits
    setColor(COLOR_LIGHT_MAGENTA, COLOR_BLACK);
    setCursor(50, 17);
    writeString("Gadget Factory");
    setColor(COLOR_DARK_GRAY, COLOR_BLACK);
    setCursor(50, 19);
    writeString("github.com/GadgetFactory");
    
    // Footer
    setColor(COLOR_YELLOW, COLOR_RED);
    setCursor(30, 22);
    writeString(" Press RESET to restart ");
    
    Serial.println("Demo complete!");
}

void loop() {
    // Blink cursor indicator
    static unsigned long lastBlink = 0;
    static bool state = false;
    
    if (millis() - lastBlink > 500) {
        lastBlink = millis();
        state = !state;
        
        setCursor(78, 1);
        setColor(state ? COLOR_WHITE : COLOR_BLUE, COLOR_BLUE);
        writeChar(state ? '*' : ' ');
    }
}
