// ESP32-S3 Arduino Example for Controlling FPGA via SPI
// Controls both RGB LED and HDMI color bars via Wishbone SPI interface
// Address Map:
//   0x00-0x0F: RGB LED Controller
//   0x10-0x1F: HDMI Color Bar Controller

#include <SPI.h>

// SPI Pin Configuration for ESP32-S3
#define SPI_CLK   12   // SCK - connects to FPGA esp_clk
#define SPI_MOSI  11   // MOSI - connects to FPGA esp_mosi
#define SPI_MISO  9    // MISO - connects from FPGA esp_miso
#define SPI_CS    10   // CS - connects to FPGA esp_cs_n

// SPI Wishbone Protocol Commands
#define CMD_WRITE 0x01
#define CMD_READ  0x02

// 8-bit Wishbone Register Addresses
// RGB LED (addresses 0x00-0x0F)
#define REG_LED_GREEN  0x00  // LED Green value
#define REG_LED_RED    0x01  // LED Red value
#define REG_LED_BLUE   0x02  // LED Blue value

// HDMI Color Bar (addresses 0x10-0x1F)
#define REG_HDMI_CTRL  0x10  // Control (bit 0 = enable color bars)

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
  delay(1000);
  Serial.println("ESP32-S3 FPGA Control - RGB LED + HDMI Color Bars");
  
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
  
  // Enable HDMI color bars
  wishboneWrite8(REG_HDMI_CTRL, 0x01);
  Serial.println("HDMI color bars enabled");
  
  // Set RGB LED to green
  setLEDColor(COLOR_GREEN);
  Serial.println("LED set to GREEN");
}

void loop() {
  // Read and forward UART debug data from FPGA
  while (Serial2.available()) {
    Serial.write(Serial2.read());
  }
  
  // Example: Cycle through colors every 2 seconds
  delay(2000);
  setLEDColor(COLOR_RED);
  Serial.println("LED set to RED");
  
  delay(2000);
  setLEDColor(COLOR_BLUE);
  Serial.println("LED set to BLUE");
  
  delay(2000);
  setLEDColor(COLOR_GREEN);
  Serial.println("LED set to GREEN");
  
  delay(2000);
  setLEDColor(COLOR_YELLOW);
  Serial.println("LED set to YELLOW");
  
  delay(2000);
  setLEDColor(COLOR_CYAN);
  Serial.println("LED set to CYAN");
  
  delay(2000);
  setLEDColor(COLOR_MAGENTA);
  Serial.println("LED set to MAGENTA");
  
  delay(2000);
  setLEDColor(COLOR_PURPLE);
  Serial.println("LED set to PURPLE");
  
  delay(2000);
  setLEDColor(COLOR_ORANGE);
  Serial.println("LED set to ORANGE");
  
  // Toggle HDMI color bars every 16 seconds
  static unsigned long lastToggle = 0;
  if (millis() - lastToggle > 16000) {
    static bool hdmiEnabled = true;
    hdmiEnabled = !hdmiEnabled;
    wishboneWrite8(REG_HDMI_CTRL, hdmiEnabled ? 0x01 : 0x00);
    Serial.println(hdmiEnabled ? "HDMI enabled" : "HDMI disabled");
    lastToggle = millis();
  }
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
  
  delay(10);  // Allow time for LED update
}

// Write to 8-bit Wishbone register via SPI
// Protocol: [CMD][ADDR][DATA] (3 bytes)
void wishboneWrite8(uint8_t address, uint8_t data) {
  fpgaSPI->beginTransaction(SPISettings(100000, MSBFIRST, SPI_MODE0));
  digitalWrite(SPI_CS, LOW);
  
  fpgaSPI->transfer(CMD_WRITE);  // Command
  fpgaSPI->transfer(address);    // Address (8-bit)
  fpgaSPI->transfer(data);       // Data byte
  
  digitalWrite(SPI_CS, HIGH);
  fpgaSPI->endTransaction();
  delayMicroseconds(10);  // Small delay between transactions
}

// Read from 8-bit Wishbone register via SPI
// Protocol: [CMD][ADDR][DUMMY] → returns data on 3rd byte
uint8_t wishboneRead8(uint8_t address) {
  uint8_t data = 0;
  
  fpgaSPI->beginTransaction(SPISettings(100000, MSBFIRST, SPI_MODE0));
  digitalWrite(SPI_CS, LOW);
  
  fpgaSPI->transfer(CMD_READ);   // Command
  fpgaSPI->transfer(address);    // Address
  data = fpgaSPI->transfer(0x00); // Read data
  
  digitalWrite(SPI_CS, HIGH);
  fpgaSPI->endTransaction();
  
  return data;
}
