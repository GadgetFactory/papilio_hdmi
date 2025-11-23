// Papilio HDMI Text Mode Example
// Demonstrates text output to HDMI display using the papilio_hdmi library
// This example shows how to use the text mode features similar to the classic VGA text mode

#include <Arduino.h>
#include <HDMIController.h>

// SPI Pin Configuration for ESP32-S3 (adjust for your board)
#define SPI_CLK   12   // SCK
#define SPI_MOSI  11   // MOSI
#define SPI_MISO  9    // MISO
#define SPI_CS    10   // CS

// Create HDMI controller instance
HDMIController hdmi(nullptr, SPI_CS, SPI_CLK, SPI_MOSI, SPI_MISO);

void setup() {
  Serial.begin(115200);
  Serial.println("Papilio HDMI Text Mode Example");
  
  // Initialize HDMI controller
  hdmi.begin();
  delay(100);
  
  // Enable text mode (pattern mode 3)
  hdmi.enableTextMode();
  delay(100);
  
  // Clear the screen
  hdmi.clearScreen();
  
  // Set text color to bright green on black
  hdmi.setTextColor(COLOR_LIGHT_GREEN, COLOR_BLACK);
  
  // Display header
  hdmi.setCursor(25, 2);
  hdmi.println("==============================");
  hdmi.setCursor(25, 3);
  hdmi.println("  Papilio Arcade HDMI Demo");
  hdmi.setCursor(25, 4);
  hdmi.println("==============================");
  
  // Display some information with different colors
  delay(500);
  
  hdmi.setCursor(5, 7);
  hdmi.setTextColor(COLOR_YELLOW, COLOR_BLACK);
  hdmi.print("Text Mode Features:");
  
  hdmi.setCursor(7, 9);
  hdmi.setTextColor(COLOR_WHITE, COLOR_BLACK);
  hdmi.println("* 80x30 character display");
  
  hdmi.setCursor(7, 10);
  hdmi.println("* 16 foreground colors");
  
  hdmi.setCursor(7, 11);
  hdmi.println("* 16 background colors");
  
  hdmi.setCursor(7, 12);
  hdmi.println("* 8x8 VGA font");
  
  hdmi.setCursor(7, 13);
  hdmi.println("* Hardware cursor");
  
  // Show color palette
  delay(500);
  
  hdmi.setCursor(5, 16);
  hdmi.setTextColor(COLOR_YELLOW, COLOR_BLACK);
  hdmi.print("Color Palette:");
  
  // Display all 16 colors
  const char* colorNames[] = {
    "Black  ", "Blue   ", "Green  ", "Cyan   ",
    "Red    ", "Magenta", "Brown  ", "Lt Gray",
    "Dk Gray", "Lt Blue", "Lt Gren", "Lt Cyan",
    "Lt Red ", "Lt Mgnt", "Yellow ", "White  "
  };
  
  for (int i = 0; i < 16; i++) {
    int row = 18 + (i / 4);
    int col = 7 + ((i % 4) * 18);
    hdmi.setCursor(col, row);
    hdmi.setTextColor(i, COLOR_BLACK);
    hdmi.print(colorNames[i]);
  }
  
  // Reset to white on black
  hdmi.setTextColor(COLOR_WHITE, COLOR_BLACK);
  
  // Show live counter at bottom
  hdmi.setCursor(5, 26);
  hdmi.setTextColor(COLOR_LIGHT_CYAN, COLOR_BLACK);
  hdmi.print("Counter: ");
  
  Serial.println("Text mode initialized - counter starting...");
}

uint32_t counter = 0;

void loop() {
  // Update counter every second
  hdmi.setCursor(14, 26);
  hdmi.setTextColor(COLOR_LIGHT_CYAN, COLOR_BLACK);
  
  char buffer[20];
  sprintf(buffer, "%08lu", counter);
  hdmi.print(buffer);
  
  // Also print to serial for debugging
  Serial.print("Counter: ");
  Serial.println(counter);
  
  counter++;
  delay(1000);
  
  // Every 10 seconds, show a different color
  if (counter % 10 == 0) {
    uint8_t color = (counter / 10) % 16;
    hdmi.setCursor(5, 28);
    hdmi.setTextColor(color, COLOR_BLACK);
    hdmi.print("Color changing demo - current color index: ");
    char colorBuf[4];
    sprintf(colorBuf, "%02d", color);
    hdmi.print(colorBuf);
    hdmi.print("  ");
  }
}
