// ESP32-S3 Arduino Example for Controlling FPGA RGB LED and HDMI via SPI
// Controls RGB LED and HDMI test patterns via Wishbone SPI interface

#include <SPI.h>

// SPI Pin Configuration for ESP32-S3
// Adjust these pins based on your actual hardware connections
#define SPI_CLK   12   // SCK - connects to FPGA esp_clk
#define SPI_MOSI  11   // MOSI - connects to FPGA esp_mosi
#define SPI_MISO  9   // MISO - connects from FPGA esp_miso
#define SPI_CS    10   // CS - connects to FPGA esp_cs_n

// SPI Wishbone Protocol Commands
#define CMD_WRITE 0x01
#define CMD_READ  0x02

// 8-bit Wishbone Register Addresses - RGB LED (Slave 0: 0x00-0x0F)
#define REG_LED_GREEN  0x0000  // LED Green value
#define REG_LED_RED    0x0001  // LED Red value
#define REG_LED_BLUE   0x0002  // LED Blue value
#define REG_LED_CTRL   0x0003  // Control (bit 0 = trigger)

// 8-bit Wishbone Register Addresses - HDMI Video (Slave 1: 0x10-0x1F)
#define REG_VIDEO_PATTERN  0x0010  // Pattern mode (0=bars, 1=grid, 2=gray, 3=solid)
#define REG_VIDEO_STATUS   0x0011  // Status/version

// HDMI Pattern Modes
#define PATTERN_COLOR_BARS  0x00
#define PATTERN_GRID        0x01
#define PATTERN_GRAYSCALE   0x02
#define PATTERN_SOLID       0x03

// Common colors in GRB format for WS2812B (dimmed to 10% brightness)
#define COLOR_OFF     0x000000
#define COLOR_RED     0x001900  // G=0,   R=25, B=0
#define COLOR_GREEN   0x190000  // G=25, R=0,   B=0
#define COLOR_BLUE    0x000019  // G=0,   R=0,   B=25
#define COLOR_YELLOW  0x191900  // G=25, R=25, B=0
#define COLOR_CYAN    0x190019  // G=25, R=0,   B=25
#define COLOR_MAGENTA 0x001919  // G=0,   R=25, B=25
#define COLOR_WHITE   0x191919  // G=25, R=25, B=25
#define COLOR_ORANGE  0x0C1900  // G=12, R=25, B=0
#define COLOR_PURPLE  0x000C0C  // G=0,   R=12, B=12

SPIClass * fpgaSPI = NULL;

void setup() {
  Serial.begin(115200);
  delay(10000);
  Serial.println("ESP32-S3 FPGA RGB LED + HDMI Control");
  
  // Initialize SPI for FPGA communication
  fpgaSPI = new SPIClass(HSPI);
  fpgaSPI->begin(SPI_CLK, SPI_MISO, SPI_MOSI, SPI_CS);
  
  
  // Configure CS pin
  pinMode(SPI_CS, OUTPUT);
  digitalWrite(SPI_CS, HIGH);  // CS idle high
  
  // Initialize Serial2 for UART debug from FPGA (on MISO pin)
  Serial2.begin(115200, SERIAL_8N1, SPI_MISO, -1);  // RX on MISO pin, no TX
  
  Serial.println("SPI initialized");
  Serial.println("UART debug initialized on MISO pin");
  delay(100);
  
  // Set RGB LED to green
  setLEDColor(COLOR_GREEN);
  Serial.println("LED set to GREEN");
  
  // Set HDMI to color bars
  setVideoPattern(PATTERN_COLOR_BARS);
  Serial.println("HDMI set to COLOR BARS");
}

void loop() {
  // Read and forward UART debug data from FPGA
  while (Serial2.available()) {
    Serial.write(Serial2.read());
  }
  
  // Example: Cycle through colors and patterns every 2 seconds
  delay(2000);
  setLEDColor(COLOR_RED);
  setVideoPattern(PATTERN_GRID);
  Serial.println("LED: RED, HDMI: GRID");
  
  delay(2000);
  setLEDColor(COLOR_BLUE);
  setVideoPattern(PATTERN_GRAYSCALE);
  Serial.println("LED: BLUE, HDMI: GRAYSCALE");
  
  delay(2000);
  setLEDColor(COLOR_GREEN);
  setVideoPattern(PATTERN_SOLID);
  Serial.println("LED: GREEN, HDMI: SOLID COLOR");
  
  delay(2000);
  setLEDColor(COLOR_YELLOW);
  setVideoPattern(PATTERN_COLOR_BARS);
  Serial.println("LED: YELLOW, HDMI: COLOR BARS");
  
  delay(2000);
  setLEDColor(COLOR_CYAN);
  Serial.println("LED: CYAN");
  
  delay(2000);
  setLEDColor(COLOR_MAGENTA);
  Serial.println("LED: MAGENTA");
  
  delay(2000);
  setLEDColor(COLOR_PURPLE);
  Serial.println("LED: PURPLE");
  
  delay(2000);
  setLEDColor(COLOR_ORANGE);
  Serial.println("LED: ORANGE");
}

// Set RGB LED color
void setLEDColor(uint32_t color) {
  // Extract color components (GRB format)
  uint8_t g = (color >> 16) & 0xFF;
  uint8_t r = (color >> 8) & 0xFF;
  uint8_t b = color & 0xFF;
  
  // Write to 8-bit Wishbone registers
  wishboneWrite8(REG_LED_GREEN, g);
  wishboneWrite8(REG_LED_RED, r);
  wishboneWrite8(REG_LED_BLUE, b);
  
  delay(100);  // Allow time for LED update
}

// Write to 8-bit Wishbone register via SPI
// Protocol: [CMD][ADDR][DATA] (3 bytes)
void wishboneWrite8(uint16_t address, uint8_t data) {
  fpgaSPI->beginTransaction(SPISettings(100000, MSBFIRST, SPI_MODE0));  // Try MODE0
  digitalWrite(SPI_CS, LOW);
  
  fpgaSPI->transfer(CMD_WRITE);      // Command
  fpgaSPI->transfer(address & 0xFF); // Address (8-bit)
  fpgaSPI->transfer(data);           // Data byte
  
  digitalWrite(SPI_CS, HIGH);
  fpgaSPI->endTransaction();
  delay(1);  // Small delay between transactions
}

// Read from 8-bit Wishbone register via SPI
// Protocol: [CMD][ADDR_L][ADDR_H] (3 bytes)
uint8_t wishboneRead8(uint16_t address) {
  uint8_t data = 0;
  
  fpgaSPI->beginTransaction(SPISettings(100000, MSBFIRST, SPI_MODE1));
  digitalWrite(SPI_CS, LOW);
  
  fpgaSPI->transfer(CMD_READ);            // Command
  fpgaSPI->transfer(address & 0xFF);      // Address low byte
  fpgaSPI->transfer((address >> 8) & 0xFF); // Address high byte
  
  digitalWrite(SPI_CS, HIGH);
  fpgaSPI->endTransaction();
  
  return data;
}

// Write to Wishbone register via SPI
// Protocol: [CMD][ADDRESS][DATA]
//   CMD: 1 byte (0x01 for write)
//   ADDRESS: 4 bytes (MSB first)
//   DATA: 4 bytes (MSB first)
void wishboneWrite(uint32_t address, uint32_t data) {
  fpgaSPI->beginTransaction(SPISettings(100000, MSBFIRST, SPI_MODE1));  // Try MODE1
  digitalWrite(SPI_CS, LOW);
  
  // Send command byte
  fpgaSPI->transfer(CMD_WRITE);
  
  // Send address (32 bits, MSB first)
  fpgaSPI->transfer((address >> 24) & 0xFF);
  fpgaSPI->transfer((address >> 16) & 0xFF);
  fpgaSPI->transfer((address >> 8) & 0xFF);
  fpgaSPI->transfer(address & 0xFF);
  
  // Send data (32 bits, MSB first)
  fpgaSPI->transfer((data >> 24) & 0xFF);
  fpgaSPI->transfer((data >> 16) & 0xFF);
  fpgaSPI->transfer((data >> 8) & 0xFF);
  fpgaSPI->transfer(data & 0xFF);
  
  digitalWrite(SPI_CS, HIGH);
  fpgaSPI->endTransaction();
}

// Read from Wishbone register via SPI
// Protocol: [CMD][ADDRESS][DATA_READ]
//   CMD: 1 byte (0x02 for read)
//   ADDRESS: 4 bytes (MSB first)
//   DATA_READ: 4 bytes received (MSB first)
uint32_t wishboneRead(uint32_t address) {
  uint32_t data = 0;
  
  fpgaSPI->beginTransaction(SPISettings(100000, MSBFIRST, SPI_MODE1));  // Try MODE1
  digitalWrite(SPI_CS, LOW);
  
  // Send command byte
  fpgaSPI->transfer(CMD_READ);
  
  // Send address (32 bits, MSB first)
  fpgaSPI->transfer((address >> 24) & 0xFF);
  fpgaSPI->transfer((address >> 16) & 0xFF);
  fpgaSPI->transfer((address >> 8) & 0xFF);
  fpgaSPI->transfer(address & 0xFF);
  
  // Receive data (32 bits, MSB first)
  data |= ((uint32_t)fpgaSPI->transfer(0x00) << 24);
  data |= ((uint32_t)fpgaSPI->transfer(0x00) << 16);
  data |= ((uint32_t)fpgaSPI->transfer(0x00) << 8);
  data |= (uint32_t)fpgaSPI->transfer(0x00);
  
  digitalWrite(SPI_CS, HIGH);
  fpgaSPI->endTransaction();
  
  return data;
}

// Check if LED controller is busy
bool isLEDBusy() {
  uint8_t status = wishboneRead8(REG_LED_CTRL);
  return (status & 0x01) != 0;  // Bit 0 is BUSY flag
}

// Set custom RGB color (values 0-255)
void setLEDColorRGB(uint8_t red, uint8_t green, uint8_t blue) {
  // Convert RGB to GRB format for WS2812B
  uint32_t color = ((uint32_t)green << 16) | ((uint32_t)red << 8) | blue;
  setLEDColor(color);
}

// Set HDMI video pattern mode
void setVideoPattern(uint8_t pattern) {
  wishboneWrite8(REG_VIDEO_PATTERN, pattern);
  Serial.print("HDMI pattern changed to: ");
  Serial.println(pattern);
}

// Get current HDMI video pattern
uint8_t getVideoPattern() {
  return wishboneRead8(REG_VIDEO_PATTERN);
}

// Get HDMI controller status/version
uint8_t getVideoStatus() {
  return wishboneRead8(REG_VIDEO_STATUS);
}
