/*
  HQVGA_GFX - Adafruit GFX adapter for HQVGA framebuffer display
  
  This class provides Adafruit_GFX compatibility for the HQVGA display,
  giving access to all GFX drawing primitives (lines, circles, rectangles,
  triangles, text, bitmaps, etc.)
  
  Hardware:
  - Papilio Arcade board with ESP32-S3 and FPGA
  - HDMI output (160x120 scaled to 720p)
  
  Usage:
    #include <SPI.h>
    #include <HQVGA_GFX.h>
    
    HQVGA_GFX display;
    
    void setup() {
      SPIClass *spi = new SPIClass(HSPI);
      spi->begin(12, 9, 11, 10);
      display.begin(spi, 10, 12, 11, 9);
      
      display.fillScreen(0);
      display.setTextColor(display.color332(255, 255, 0));  // Yellow
      display.setCursor(10, 10);
      display.print("Hello GFX!");
      display.drawCircle(80, 60, 30, display.color332(255, 0, 0));
    }
*/

#ifndef HQVGA_GFX_h
#define HQVGA_GFX_h

#include <Adafruit_GFX.h>
#include <HQVGA.h>

class HQVGA_GFX : public Adafruit_GFX {
public:
  // Constructor - 160x120 display
  HQVGA_GFX() : Adafruit_GFX(160, 120) {}
  
  // Initialize the display (wraps VGA.begin)
  void begin(SPIClass* spi = nullptr, uint8_t csPin = 10, 
             uint8_t spiClk = 12, uint8_t spiMosi = 11, uint8_t spiMiso = 9,
             uint8_t wishboneBase = 0x00) {
    VGA.begin(spi, csPin, spiClk, spiMosi, spiMiso, wishboneBase);
  }
  
  // Required by Adafruit_GFX - draw a single pixel
  void drawPixel(int16_t x, int16_t y, uint16_t color) override {
    if (x < 0 || x >= width() || y < 0 || y >= height()) return;
    VGA.putPixel(x, y, (uint8_t)color);
  }
  
  // Override fillScreen for better performance
  void fillScreen(uint16_t color) override {
    VGA.setBackgroundColor((uint8_t)color);
    VGA.clear();
  }
  
  // Override drawFastHLine for better performance
  void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) override {
    if (y < 0 || y >= height()) return;
    if (x < 0) { w += x; x = 0; }
    if (x + w > width()) w = width() - x;
    if (w <= 0) return;
    
    for (int16_t i = 0; i < w; i++) {
      VGA.putPixel(x + i, y, (uint8_t)color);
    }
  }
  
  // Override drawFastVLine for better performance
  void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) override {
    if (x < 0 || x >= width()) return;
    if (y < 0) { h += y; y = 0; }
    if (y + h > height()) h = height() - y;
    if (h <= 0) return;
    
    for (int16_t i = 0; i < h; i++) {
      VGA.putPixel(x, y + i, (uint8_t)color);
    }
  }
  
  // Override fillRect for better performance
  void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) override {
    // Clip to screen bounds
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > width()) w = width() - x;
    if (y + h > height()) h = height() - y;
    if (w <= 0 || h <= 0) return;
    
    for (int16_t j = 0; j < h; j++) {
      for (int16_t i = 0; i < w; i++) {
        VGA.putPixel(x + i, y + j, (uint8_t)color);
      }
    }
  }
  
  // Helper: Convert RGB888 to RGB332 color format
  static uint8_t color332(uint8_t r, uint8_t g, uint8_t b) {
    return ((r >> 5) << 5) | ((g >> 5) << 2) | (b >> 6);
  }
  
  // Helper: Get predefined colors (RGB332)
  static const uint8_t BLACK   = 0x00;
  static const uint8_t RED     = 0xE0;
  static const uint8_t GREEN   = 0x1C;
  static const uint8_t BLUE    = 0x03;
  static const uint8_t YELLOW  = 0xFC;
  static const uint8_t CYAN    = 0x1F;
  static const uint8_t MAGENTA = 0xE3;
  static const uint8_t WHITE   = 0xFF;
  
  // Access to underlying VGA object for advanced operations
  VGA_class& getVGA() { return VGA; }
};

#endif
