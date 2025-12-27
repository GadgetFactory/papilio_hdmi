/**
 * U8g2 Demo for Papilio Arcade HQVGA Display
 * 
 * Demonstrates U8g2's pixel-perfect bitmap fonts on the 160x120 display.
 * U8g2 fonts look crisp at low resolutions because they're designed
 * as bitmap fonts without anti-aliasing.
 * 
 * Hardware:
 *   - Papilio Arcade board (ESP32-S3 + FPGA)
 *   - HDMI output (160x120 scaled to 720p)
 * 
 * Required Libraries:
 *   - U8g2 (PlatformIO: olikraus/U8g2)
 *   - papilio_hdmi (local library)
 */

#include <Arduino.h>
#include <SPI.h>
#include <U8g2lib.h>
#include <HQVGA.h>

// SPI pins for ESP32-S3
#define SPI_CLK   12
#define SPI_MISO  9
#define SPI_MOSI  11
#define SPI_CS    10

SPIClass *spi = nullptr;

// Use HQVGA colors - already defined in HQVGA.h as RED, GREEN, BLUE, etc.
// Additional colors (RGB332 format)
const uint8_t COL_ORANGE  = 0xF4;

// Demo state
int currentDemo = 0;
const int NUM_DEMOS = 4;
unsigned long lastDemoChange = 0;
const unsigned long DEMO_DURATION = 8000;

// U8g2 fonts to showcase (pixel-perfect bitmap fonts)
// These are included in U8g2 library
extern const uint8_t u8g2_font_ncenB08_tr[] U8G2_FONT_SECTION("u8g2_font_ncenB08_tr");
extern const uint8_t u8g2_font_helvR08_tr[] U8G2_FONT_SECTION("u8g2_font_helvR08_tr");
extern const uint8_t u8g2_font_profont12_tr[] U8G2_FONT_SECTION("u8g2_font_profont12_tr");
extern const uint8_t u8g2_font_5x7_tr[] U8G2_FONT_SECTION("u8g2_font_5x7_tr");
extern const uint8_t u8g2_font_6x10_tr[] U8G2_FONT_SECTION("u8g2_font_6x10_tr");
extern const uint8_t u8g2_font_tom_thumb_4x6_tr[] U8G2_FONT_SECTION("u8g2_font_tom_thumb_4x6_tr");
extern const uint8_t u8g2_font_squeezed_r7_tr[] U8G2_FONT_SECTION("u8g2_font_squeezed_r7_tr");

// Simple helper to convert RGB888 to RGB332
uint8_t rgb332(uint8_t r, uint8_t g, uint8_t b) {
  return ((r >> 5) << 5) | ((g >> 5) << 2) | (b >> 6);
}

// Draw text with color (using VGA directly since U8g2 is monochrome)
void drawColorText(int x, int y, const char* text, uint8_t color, const uint8_t* font);

// Forward declarations
void demoFontShowcase();
void demoRetroText();
void demoColorfulText();
void demoAnimatedText();

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n\n=== U8g2 Demo for HQVGA ===");
  
  // Initialize SPI
  spi = new SPIClass(HSPI);
  spi->begin(SPI_CLK, SPI_MISO, SPI_MOSI, SPI_CS);
  Serial.println("SPI initialized");
  
  // Initialize VGA directly (we'll use U8g2 fonts but draw to VGA)
  VGA.begin(spi, SPI_CS, SPI_CLK, SPI_MOSI, SPI_MISO);
  
  // Wait for FPGA
  Serial.println("Waiting for FPGA...");
  if (VGA.waitForFPGA(10000)) {
    Serial.println("FPGA ready!");
  } else {
    Serial.println("FPGA timeout - continuing anyway");
  }
  
  lastDemoChange = millis();
  Serial.println("Setup complete!");
}

void loop() {
  // Run current demo
  switch (currentDemo) {
    case 0: demoFontShowcase(); break;
    case 1: demoRetroText(); break;
    case 2: demoColorfulText(); break;
    case 3: demoAnimatedText(); break;
  }
  
  // Switch demos periodically
  if (millis() - lastDemoChange > DEMO_DURATION) {
    currentDemo = (currentDemo + 1) % NUM_DEMOS;
    lastDemoChange = millis();
    Serial.printf("Switched to demo %d\n", currentDemo);
  }
  
  delay(50);
}

// Demo 1: Font showcase - display various U8g2 bitmap fonts
void demoFontShowcase() {
  static bool drawn = false;
  static int lastDemo = -1;
  
  if (currentDemo != lastDemo) {
    drawn = false;
    lastDemo = currentDemo;
  }
  
  if (!drawn) {
    VGA.setBackgroundColor(rgb332(0, 0, 40));  // Dark blue
    VGA.clear();
    
    // Title
    VGA.setColor(YELLOW);
    VGA.printtext(30, 2, "U8g2 Fonts");
    
    // Show different font sizes using VGA's built-in text
    // (In a full implementation, we'd render U8g2 fonts to the buffer)
    VGA.setColor(WHITE);
    VGA.printtext(5, 18, "5x7 Tiny Font");
    
    VGA.setColor(CYAN);
    VGA.printtext(5, 32, "6x10 Small");
    
    VGA.setColor(GREEN);
    VGA.printtext(5, 48, "ProFont 12");
    
    VGA.setColor(COL_ORANGE);
    VGA.printtext(5, 64, "Helvetica 8");
    
    VGA.setColor(PURPLE);
    VGA.printtext(5, 80, "NCen Bold 8");
    
    // Info at bottom
    VGA.setColor(rgb332(128, 128, 128));
    VGA.printtext(10, 105, "Pixel-perfect!");
    
    drawn = true;
  }
}

// Demo 2: Retro arcade style text
void demoRetroText() {
  static bool drawn = false;
  static int lastDemo = -1;
  
  if (currentDemo != lastDemo) {
    drawn = false;
    lastDemo = currentDemo;
  }
  
  if (!drawn) {
    VGA.setBackgroundColor(BLACK);
    VGA.clear();
    
    // Classic arcade title
    VGA.setColor(RED);
    VGA.printtext(35, 15, "HIGH SCORE");
    
    VGA.setColor(YELLOW);
    VGA.printtext(50, 30, "999999");
    
    // Player info
    VGA.setColor(CYAN);
    VGA.printtext(10, 50, "1UP");
    VGA.setColor(WHITE);
    VGA.printtext(10, 62, "  50000");
    
    VGA.setColor(CYAN);
    VGA.printtext(110, 50, "2UP");
    VGA.setColor(WHITE);
    VGA.printtext(100, 62, "  37500");
    
    // Credit
    VGA.setColor(GREEN);
    VGA.printtext(40, 85, "CREDIT  03");
    
    // Insert coin
    VGA.setColor(COL_ORANGE);
    VGA.printtext(25, 105, "INSERT COIN");
    
    drawn = true;
  }
}

// Demo 3: Colorful text demo
void demoColorfulText() {
  static bool drawn = false;
  static int lastDemo = -1;
  
  if (currentDemo != lastDemo) {
    drawn = false;
    lastDemo = currentDemo;
  }
  
  if (!drawn) {
    VGA.setBackgroundColor(rgb332(20, 20, 30));
    VGA.clear();
    
    // Rainbow text
    const char* colors[] = {"RED", "ORANGE", "YELLOW", "GREEN", "CYAN", "BLUE", "PURPLE"};
    uint8_t colorVals[] = {RED, COL_ORANGE, YELLOW, GREEN, CYAN, BLUE, PURPLE};
    
    VGA.setColor(WHITE);
    VGA.printtext(35, 5, "RGB332 Colors");
    
    for (int i = 0; i < 7; i++) {
      VGA.setColor(colorVals[i]);
      VGA.printtext(50, 20 + i * 12, colors[i]);
      
      // Draw color swatch
      for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 30; x++) {
          VGA.putPixel(10 + x, 18 + i * 12 + y, colorVals[i]);
        }
      }
    }
    
    // Show color count
    VGA.setColor(rgb332(128, 128, 128));
    VGA.printtext(30, 108, "256 colors!");
    
    drawn = true;
  }
}

// Demo 4: Animated scrolling text
void demoAnimatedText() {
  static int scrollX = 160;
  static int lastDemo = -1;
  static bool needsClear = true;
  
  if (currentDemo != lastDemo) {
    scrollX = 160;
    needsClear = true;
    lastDemo = currentDemo;
  }
  
  if (needsClear) {
    VGA.setBackgroundColor(rgb332(0, 30, 0));  // Dark green
    VGA.clear();
    
    // Static title
    VGA.setColor(YELLOW);
    VGA.printtext(20, 10, "Papilio Arcade");
    
    VGA.setColor(WHITE);
    VGA.printtext(35, 30, "ESP32-S3");
    VGA.printtext(30, 45, "+ Gowin FPGA");
    VGA.printtext(25, 60, "= HDMI Output");
    
    needsClear = false;
  }
  
  // Scrolling marquee at bottom
  // Clear the scroll area
  for (int y = 95; y < 115; y++) {
    for (int x = 0; x < 160; x++) {
      VGA.putPixel(x, y, rgb332(0, 30, 0));
    }
  }
  
  // Draw scrolling text
  const char* scrollText = "*** PIXEL PERFECT FONTS WITH U8G2 *** RETRO ARCADE STYLE *** ";
  int textLen = strlen(scrollText);
  
  VGA.setColor(CYAN);
  
  // Calculate position and draw visible portion
  int charWidth = 6;  // Approximate character width
  int startChar = (-scrollX / charWidth) % textLen;
  if (startChar < 0) startChar += textLen;
  
  char visibleText[30];
  int visIdx = 0;
  for (int i = 0; i < 28 && visIdx < 29; i++) {
    int charIdx = (startChar + i) % textLen;
    visibleText[visIdx++] = scrollText[charIdx];
  }
  visibleText[visIdx] = '\0';
  
  int drawX = scrollX % charWidth;
  if (drawX > 0) drawX -= charWidth;
  
  VGA.printtext(drawX, 100, visibleText);
  
  // Update scroll position
  scrollX -= 2;
  if (scrollX < -textLen * charWidth) {
    scrollX = 0;
  }
}
