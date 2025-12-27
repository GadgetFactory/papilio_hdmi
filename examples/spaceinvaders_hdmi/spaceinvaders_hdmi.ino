/*
  HDMI LiquidCrystal Library - Space Invaders Demo
 
  Demonstrates the use a 16x2 LCD display on HDMI.  The HDMILiquidCrystal
  library works with all LCD displays that are compatible with the Hitachi HD44780 driver.
  
  This sketch shows a space-invader animation using custom characters.
  
  Original Demo by JO3RI
  Modified for HDMI by Jack Gassett
  
  Hardware:
  - Papilio Arcade board with ESP32-S3 and FPGA
  - HDMI output
*/

#include <SPI.h>
#include <HDMIController.h>
#include <HDMILiquidCrystal.h>

// SPI Pin Configuration for ESP32-S3
#define SPI_CLK   12
#define SPI_MOSI  11
#define SPI_MISO  9
#define SPI_CS    10

// Binary notation helpers
#define B00000 0
#define B00001 1
#define B00010 2
#define B00011 3
#define B00100 4
#define B00101 5
#define B00110 6
#define B00111 7
#define B01000 8
#define B01001 9
#define B01010 10
#define B01011 11
#define B01100 12
#define B01101 13
#define B01110 14
#define B01111 15
#define B10000 16
#define B10001 17
#define B10010 18
#define B10011 19
#define B10100 20
#define B10101 21
#define B10110 22
#define B10111 23
#define B11000 24
#define B11001 25
#define B11010 26
#define B11011 27
#define B11100 28
#define B11101 29
#define B11110 30
#define B11111 31

SPIClass *fpgaSPI = NULL;
HDMIController *hdmi = NULL;
HDMILiquidCrystal *lcd = NULL;

// Space invader animation speed:
int animDelay = 200;

// Custom character definitions for space invader
byte charEmpty[8] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000
};

// Space invader - frame 1 parts
byte invader1_part1[8] = {
  B00001,
  B00000,
  B00001,
  B00011,
  B00111,
  B01111,
  B01011,
  B01011
};

byte invader1_part2[8] = {
  B11110,
  B00000,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B01110
};

byte invader1_part3[8] = {
  B01111,
  B00000,
  B11110,
  B11100,
  B11000,
  B10000,
  B00000,
  B00000
};

// Space invader - frame 2 parts
byte invader2_part1[8] = {
  B00001,
  B00000,
  B00001,
  B00011,
  B00111,
  B01111,
  B01011,
  B11011
};

byte invader2_part2[8] = {
  B11110,
  B00000,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B10110
};

byte invader2_part3[8] = {
  B01111,
  B00000,
  B11110,
  B11100,
  B11000,
  B10000,
  B00000,
  B00000
};

void setup() {
  Serial.begin(115200);
  
  Serial.println("Papilio HDMI Space Invaders Demo");
  
  // Initialize SPI
  fpgaSPI = new SPIClass(HSPI);
  fpgaSPI->begin(SPI_CLK, SPI_MISO, SPI_MOSI, SPI_CS);
  pinMode(SPI_CS, OUTPUT);
  digitalWrite(SPI_CS, HIGH);
  
  delay(500);
  
  // Initialize HDMI controller
  hdmi = new HDMIController(fpgaSPI, SPI_CS, SPI_CLK, SPI_MOSI, SPI_MISO);
  hdmi->begin();
  delay(100);
  
  // Create LCD interface with 16 columns and 2 rows
  lcd = new HDMILiquidCrystal(hdmi, 16, 2);
  lcd->begin(16, 2);
  
  // Set colors (green text on black background for retro look)
  lcd->setColor(HDMI_COLOR_LIGHT_GREEN, HDMI_COLOR_BLACK);
  
  // Clear the LCD screen
  lcd->clear();
  
  // Display title
  lcd->setCursor(0, 0);
  lcd->print("SPACE INVADERS!");
  lcd->setCursor(0, 1);
  lcd->print("  HDMI DEMO  ");
  
  Serial.println("LCD initialized");
  
  delay(2000);
  lcd->clear();
}

void loop() {
  // Animate space invader moving across screen
  for (int pos = 0; pos < 14; pos++) {
    lcd->clear();
    
    // Frame 1 - legs down
    lcd->createChar(0, invader1_part1);
    lcd->createChar(1, invader1_part2);
    lcd->createChar(2, invader1_part3);
    
    lcd->setCursor(pos, 0);
    lcd->write((uint8_t)0);
    lcd->write((uint8_t)1);
    lcd->write((uint8_t)2);
    
    delay(animDelay);
    
    // Frame 2 - legs up
    lcd->createChar(0, invader2_part1);
    lcd->createChar(1, invader2_part2);
    lcd->createChar(2, invader2_part3);
    
    lcd->setCursor(pos, 0);
    lcd->write((uint8_t)0);
    lcd->write((uint8_t)1);
    lcd->write((uint8_t)2);
    
    delay(animDelay);
  }
  
  // Show "GAME OVER" message
  lcd->clear();
  lcd->setCursor(3, 0);
  lcd->print("GAME OVER!");
  delay(2000);
  
  lcd->clear();
  lcd->setCursor(2, 0);
  lcd->print("PLAY AGAIN?");
  delay(2000);
}
