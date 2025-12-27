/*
  VGALiquidCrystal Demo
  
  Demonstrates the VGALiquidCrystal library which provides
  a LiquidCrystal-compatible API for the HQVGA display.
  
  This allows existing LCD code to work with minimal changes
  on the Papilio Arcade's HDMI output.
  
  Hardware:
  - Papilio Arcade board with ESP32-S3 and FPGA
  - HDMI display connected
*/

#include <SPI.h>
#include <HQVGA.h>
#include <VGALiquidCrystal.h>

// SPI Pin Configuration
#define SPI_CLK   12
#define SPI_MOSI  11
#define SPI_MISO  9
#define SPI_CS    10

// Color definitions (RGB332 format)
#define BLUE_BG       0x03    // Blue background for screen
#define SILVER        0xDB    // Silver/gray for border (R=6, G=6, B=3)
#define DARK_GREEN    0x08    // Dark green for LCD cells (green=2)

// Create VGALiquidCrystal object (pin parameters ignored, for API compatibility)
VGALiquidCrystal lcd;

SPIClass *fpgaSPI = NULL;

void setup() {
  Serial.begin(115200);
  Serial.println("VGALiquidCrystal Demo");
  
  // Initialize SPI
  fpgaSPI = new SPIClass(HSPI);
  fpgaSPI->begin(SPI_CLK, SPI_MISO, SPI_MOSI, SPI_CS);
  
  // Initialize VGA
  VGA.begin(fpgaSPI, SPI_CS, SPI_CLK, SPI_MOSI, SPI_MISO);
  
  // Fill entire screen with blue background (clears any leftover pixels)
  VGA.setBackgroundColor(BLUE_BG);
  VGA.clear();
  
  // Set LCD position and colors BEFORE begin() to avoid drawing at default position
  // Center the LCD on screen
  // LCD size: 16 cols × 2 rows = 96×18 pixels (6×9 per character cell)
  // Screen: 160×120, Center: (160-96)/2=32, (120-18)/2=51
  lcd.setPosition(32, 51);
  
  // Set colors: dark green cells with black text (classic LCD look)
  lcd.setTextColor(YELLOW);            // Yellow text (visible on dark green)
  lcd.setBackgroundColor(DARK_GREEN);  // Dark green LCD cells
  
  // Initialize LCD (16 columns, 2 rows)
  lcd.begin(16, 2);
  
  // Draw silver border around the LCD (3 pixel thick)
  lcd.drawBorder(SILVER, 3);;
  
  // Print message (just like a real LCD!)
  lcd.setCursor(0, 0);
  lcd.print("PAPILIO ARCADE");
  lcd.setCursor(0, 1);
  lcd.print("VGA LCD Demo!");
  
  Serial.println("LCD initialized, scrolling in 3 seconds...");
  delay(3000);
}

void loop() {
  // Scroll display left (just like a real LCD!)
  lcd.scrollDisplayLeft();
  delay(300);
}
