// ESP32 USB Serial Example
// Demonstrates USB CDC-ACM serial port alongside HDMI and RGB LED
// 
// Wishbone Address Map:
// 0x00-0x0F: RGB LED (WS2812B)
// 0x10-0x1F: HDMI Video
// 0x20-0x2F: USB Serial
//
// USB Serial Registers:
// 0x20: TX data (write) / RX data (read)
// 0x21: Status (bit 0=rx_valid, 1=tx_ready, 7=usb_connected)
// 0x22: Control (bit 0=enable)

#include <SPI.h>

// SPI pins for Papilio communication
#define SPI_CS    5   // Chip select
#define SPI_MOSI  23  // Master out, slave in
#define SPI_MISO  19  // Master in, slave out
#define SPI_CLK   18  // Clock

// Wishbone addresses
#define WB_RGB_LED_BASE    0x00
#define WB_HDMI_BASE       0x10
#define WB_USB_BASE        0x20

// USB Serial registers
#define USB_DATA_REG       (WB_USB_BASE + 0x00)
#define USB_STATUS_REG     (WB_USB_BASE + 0x01)
#define USB_CONTROL_REG    (WB_USB_BASE + 0x02)

// USB Status bits
#define USB_RX_VALID       0x01
#define USB_TX_READY       0x02
#define USB_CONNECTED      0x80

SPIClass* spi;

void setup() {
  Serial.begin(115200);
  Serial.println("\nPapilio USB Serial Demo");
  Serial.println("=======================");
  
  // Initialize SPI
  spi = new SPIClass(VSPI);
  spi->begin(SPI_CLK, SPI_MISO, SPI_MOSI, SPI_CS);
  pinMode(SPI_CS, OUTPUT);
  digitalWrite(SPI_CS, HIGH);
  
  delay(100);
  
  // Enable USB serial port
  wishboneWrite8(USB_CONTROL_REG, 0x01);
  
  // Wait for USB enumeration
  Serial.print("Waiting for USB connection...");
  uint32_t timeout = millis() + 5000;
  while (millis() < timeout) {
    uint8_t status = wishboneRead8(USB_STATUS_REG);
    if (status & USB_CONNECTED) {
      Serial.println(" Connected!");
      break;
    }
    delay(100);
    Serial.print(".");
  }
  
  if (!(wishboneRead8(USB_STATUS_REG) & USB_CONNECTED)) {
    Serial.println(" Timeout (no USB host detected)");
    Serial.println("Note: This is normal if USB cable not connected");
  }
  
  Serial.println("\nCommands:");
  Serial.println("  r<color> - Set RGB LED (r=red, g=green, b=blue, w=white)");
  Serial.println("  h<n>     - Set HDMI pattern (0-3)");
  Serial.println("  u<text>  - Send text to USB serial");
  Serial.println("  s        - Check USB status");
}

void loop() {
  // Check for data from USB serial port
  uint8_t status = wishboneRead8(USB_STATUS_REG);
  if (status & USB_RX_VALID) {
    uint8_t data = wishboneRead8(USB_DATA_REG);
    Serial.print("USB RX: 0x");
    Serial.print(data, HEX);
    Serial.print(" ('");
    Serial.print((char)data);
    Serial.println("')");
    
    // Echo back to USB
    if (status & USB_TX_READY) {
      wishboneWrite8(USB_DATA_REG, data);
    }
  }
  
  // Check for commands from ESP32 serial
  if (Serial.available()) {
    char cmd = Serial.read();
    
    switch (cmd) {
      case 'r':  // Red LED
        setRGBLED(255, 0, 0);
        Serial.println("LED: Red");
        break;
        
      case 'g':  // Green LED
        setRGBLED(0, 255, 0);
        Serial.println("LED: Green");
        break;
        
      case 'b':  // Blue LED
        setRGBLED(0, 0, 255);
        Serial.println("LED: Blue");
        break;
        
      case 'w':  // White LED
        setRGBLED(255, 255, 255);
        Serial.println("LED: White");
        break;
        
      case 'h':  // HDMI pattern
        if (Serial.available()) {
          int pattern = Serial.parseInt();
          wishboneWrite8(WB_HDMI_BASE, pattern & 0x03);
          Serial.print("HDMI: Pattern ");
          Serial.println(pattern);
        }
        break;
        
      case 'u':  // USB serial send
        Serial.print("Sending to USB: ");
        while (Serial.available()) {
          char c = Serial.read();
          if (c == '\n' || c == '\r') break;
          
          // Wait for TX ready
          uint32_t timeout = millis() + 1000;
          while (!(wishboneRead8(USB_STATUS_REG) & USB_TX_READY)) {
            if (millis() > timeout) {
              Serial.println("\nUSB TX timeout!");
              return;
            }
            delay(1);
          }
          
          wishboneWrite8(USB_DATA_REG, c);
          Serial.print(c);
        }
        Serial.println();
        break;
        
      case 's':  // Status
        printStatus();
        break;
        
      default:
        if (cmd != '\n' && cmd != '\r') {
          Serial.print("Unknown command: ");
          Serial.println(cmd);
        }
    }
  }
  
  delay(10);
}

void setRGBLED(uint8_t r, uint8_t g, uint8_t b) {
  wishboneWrite8(WB_RGB_LED_BASE + 0, r);
  wishboneWrite8(WB_RGB_LED_BASE + 1, g);
  wishboneWrite8(WB_RGB_LED_BASE + 2, b);
  wishboneWrite8(WB_RGB_LED_BASE + 3, 1);  // Trigger update
}

void printStatus() {
  Serial.println("\n=== System Status ===");
  
  // RGB LED status
  uint8_t r = wishboneRead8(WB_RGB_LED_BASE + 0);
  uint8_t g = wishboneRead8(WB_RGB_LED_BASE + 1);
  uint8_t b = wishboneRead8(WB_RGB_LED_BASE + 2);
  Serial.print("RGB LED: R=");
  Serial.print(r);
  Serial.print(" G=");
  Serial.print(g);
  Serial.print(" B=");
  Serial.println(b);
  
  // HDMI status
  uint8_t pattern = wishboneRead8(WB_HDMI_BASE);
  Serial.print("HDMI Pattern: ");
  Serial.println(pattern);
  
  // USB status
  uint8_t status = wishboneRead8(USB_STATUS_REG);
  Serial.print("USB Status: 0x");
  Serial.print(status, HEX);
  Serial.print(" - ");
  if (status & USB_CONNECTED) Serial.print("CONNECTED ");
  if (status & USB_TX_READY) Serial.print("TX_RDY ");
  if (status & USB_RX_VALID) Serial.print("RX_VAL ");
  Serial.println();
  
  Serial.println("===================\n");
}

// Wishbone write 8-bit
void wishboneWrite8(uint8_t addr, uint8_t data) {
  digitalWrite(SPI_CS, LOW);
  spi->transfer(0x80 | addr);  // Write command
  spi->transfer(data);
  digitalWrite(SPI_CS, HIGH);
  delayMicroseconds(10);
}

// Wishbone read 8-bit
uint8_t wishboneRead8(uint8_t addr) {
  digitalWrite(SPI_CS, LOW);
  spi->transfer(addr & 0x7F);  // Read command
  uint8_t data = spi->transfer(0x00);
  digitalWrite(SPI_CS, HIGH);
  delayMicroseconds(10);
  return data;
}
