/*
  Papilio Arcade HDMI LCD Demo
  
  Demonstrates LCD-style text display on HDMI output using the HDMILiquidCrystal class.
  This example mimics the classic Arduino LiquidCrystal library but outputs to HDMI.
  
  Features:
  - 16x2 LCD emulation on 80x26 HDMI text display
  - Scrolling text demo
  - Compatible with LiquidCrystal API
  
  Hardware:
  - Papilio Arcade board with ESP32-S3 and FPGA
  - HDMI output connected to monitor
  
  Created 2025
  by Jack Gassett
  http://www.gadgetfactory.net
  
  This example code is in the public domain.
*/

#include <HDMIController.h>
#include <HDMILiquidCrystal.h>

// Create HDMI controller (uses default SPI pins)
HDMIController hdmi;

// Create LCD interface with 16 columns and 2 rows
HDMILiquidCrystal lcd(&hdmi, 16, 2);

void setup() {
  Serial.begin(115200);
  Serial.println("Papilio HDMI LCD Demo");
  
  // Initialize HDMI controller
  hdmi.begin();
  delay(100);
  
  // Initialize LCD display
  lcd.begin(16, 2);
  
  // Set colors (optional - defaults to white on black)
  lcd.setColor(HDMI_COLOR_WHITE, HDMI_COLOR_BLUE);
  
  // Clear the LCD screen
  lcd.clear();
  
  // Display initial message
  lcd.setCursor(0, 0);
  lcd.print("PAPILIO LCD DEMO");
  lcd.setCursor(0, 1);
  lcd.print("HDMI TEXT MODE!");
  
  delay(3000);
}

void loop() {
  // Scroll the display left
  lcd.scrollDisplayLeft();
  delay(150);
}
