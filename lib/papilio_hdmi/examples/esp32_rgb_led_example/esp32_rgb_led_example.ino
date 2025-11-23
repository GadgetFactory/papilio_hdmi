// Minimal example using papilio_hdmi library (HDMI only)
#include <Arduino.h>
#include <HDMIController.h>

// SPI Pin Configuration for ESP32-S3 (adjust for your board)
#define SPI_CLK   12   // SCK
#define SPI_MOSI  11   // MOSI
#define SPI_MISO  9    // MISO
#define SPI_CS    10   // CS

// HDMI Pattern Modes
#define PATTERN_COLOR_BARS  0x00
#define PATTERN_GRID        0x01
#define PATTERN_GRAYSCALE   0x02
#define PATTERN_SOLID       0x03

// Create controller (let it create HSPI instance internally)
HDMIController hdmi(nullptr, SPI_CS, SPI_CLK, SPI_MOSI, SPI_MISO);

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("papilio_hdmi example (HDMI only)");

  hdmi.begin();

  // Init Serial2 for debug from FPGA (optional)
  Serial2.begin(115200, SERIAL_8N1, SPI_MISO, -1);

  // Set initial HDMI pattern
  hdmi.setVideoPattern(PATTERN_COLOR_BARS);
}

void loop() {
  while (Serial2.available()) {
    Serial.write(Serial2.read());
  }

  delay(2000);
  hdmi.setVideoPattern(PATTERN_GRID);
  Serial.println("HDMI: GRID");

  delay(2000);
  hdmi.setVideoPattern(PATTERN_GRAYSCALE);
  Serial.println("HDMI: GRAYSCALE");

  delay(2000);
  hdmi.setVideoPattern(PATTERN_SOLID);
  Serial.println("HDMI: SOLID COLOR");

  delay(2000);
  hdmi.setVideoPattern(PATTERN_COLOR_BARS);
  Serial.println("HDMI: COLOR BARS");
}
