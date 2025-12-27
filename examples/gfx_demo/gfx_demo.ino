/*
  Adafruit GFX Demo for HQVGA
  
  Demonstrates using Adafruit_GFX graphics primitives with the
  HQVGA framebuffer display. This gives you access to:
  - Lines, rectangles, circles, triangles
  - Rounded rectangles, arcs
  - Text with custom fonts
  - Bitmaps and sprites
  
  Hardware:
  - Papilio Arcade board with ESP32-S3 and FPGA
  - HDMI display connected
*/

#include <SPI.h>
#include <HQVGA_GFX.h>

// SPI Pin Configuration
#define SPI_CLK   12
#define SPI_MOSI  11
#define SPI_MISO  9
#define SPI_CS    10

// Create GFX display object
HQVGA_GFX display;

SPIClass *fpgaSPI = NULL;

void setup() {
  Serial.begin(115200);
  Serial.println("Adafruit GFX Demo");
  
  // Initialize SPI
  fpgaSPI = new SPIClass(HSPI);
  fpgaSPI->begin(SPI_CLK, SPI_MISO, SPI_MOSI, SPI_CS);
  
  // Initialize display
  display.begin(fpgaSPI, SPI_CS, SPI_CLK, SPI_MOSI, SPI_MISO);
  
  // Clear screen
  display.fillScreen(HQVGA_GFX::BLACK);
  
  // Seed random number generator
  randomSeed(analogRead(0));
  
  Serial.println("Starting GFX demos...");
}

int currentDemo = 0;
int animFrame = 0;
unsigned long lastDemoChange = 0;
unsigned long lastFrameTime = 0;
const unsigned long DEMO_DURATION = 8000;  // 8 seconds per demo
const unsigned long FRAME_DELAY = 50;      // 50ms per animation frame

void loop() {
  unsigned long now = millis();
  
  // Change demo every 8 seconds
  if (now - lastDemoChange >= DEMO_DURATION) {
    lastDemoChange = now;
    currentDemo = (currentDemo + 1) % 5;
    animFrame = 0;
    
    Serial.print("Switching to demo ");
    Serial.println(currentDemo);
    
    // Clear screen for new demo
    display.fillScreen(HQVGA_GFX::BLACK);
  }
  
  // Update animation frame
  if (now - lastFrameTime >= FRAME_DELAY) {
    lastFrameTime = now;
    
    switch (currentDemo) {
      case 0: demoShapes(); break;        // Static shapes
      case 1: demoText(); break;          // Static text
      case 2: demoExpandingCircles(); break;  // Animated circles
      case 3: demoSpinningLines(); break;     // Animated lines
      case 4: demoExpandingRects(); break;    // Animated rectangles
    }
    
    animFrame++;
  }
}

void demoShapes() {
  // Static demo - only draw once per demo cycle
  if (animFrame > 0) return;
  
  display.fillScreen(HQVGA_GFX::BLACK);
  
  // Draw various shapes
  display.drawRect(10, 10, 40, 30, HQVGA_GFX::RED);
  display.fillRect(60, 10, 40, 30, HQVGA_GFX::GREEN);
  
  display.drawCircle(30, 70, 20, HQVGA_GFX::BLUE);
  display.fillCircle(80, 70, 20, HQVGA_GFX::YELLOW);
  
  display.drawTriangle(120, 10, 150, 40, 110, 40, HQVGA_GFX::CYAN);
  display.fillTriangle(120, 50, 150, 80, 110, 80, HQVGA_GFX::MAGENTA);
  
  display.drawRoundRect(110, 85, 45, 30, 8, HQVGA_GFX::WHITE);
  
  // Title
  display.setTextColor(HQVGA_GFX::WHITE);
  display.setTextSize(1);
  display.setCursor(40, 112);
  display.print("GFX Shapes");
}

void demoText() {
  // Static demo - only draw once per demo cycle
  if (animFrame > 0) return;
  
  display.fillScreen(display.color332(0, 0, 64));  // Dark blue
  
  display.setTextColor(HQVGA_GFX::WHITE);
  display.setTextSize(1);
  display.setCursor(20, 10);
  display.print("Papilio Arcade");
  
  display.setTextColor(HQVGA_GFX::YELLOW);
  display.setTextSize(2);
  display.setCursor(30, 30);
  display.print("GFX!");
  
  display.setTextColor(HQVGA_GFX::CYAN);
  display.setTextSize(1);
  display.setCursor(5, 60);
  display.print("160x120 @ 720p HDMI");
  
  display.setTextColor(HQVGA_GFX::GREEN);
  display.setCursor(15, 80);
  display.print("Adafruit_GFX lib");
  
  display.setTextColor(display.color332(255, 128, 0));  // Orange
  display.setCursor(25, 100);
  display.print("ESP32-S3 + FPGA");
}

void demoExpandingCircles() {
  // Animated expanding circles
  int radius = (animFrame % 60) + 1;
  
  if (radius == 1) {
    display.fillScreen(HQVGA_GFX::BLACK);
  }
  
  // Draw expanding circle
  uint8_t color = display.color332((radius * 4) % 256, (255 - radius * 4) % 256, (radius * 2) % 256);
  display.drawCircle(80, 60, radius, color);
  
  // Add some random sparkles
  if (animFrame % 5 == 0) {
    int x = random(160);
    int y = random(120);
    display.drawPixel(x, y, HQVGA_GFX::WHITE);
  }
}

void demoSpinningLines() {
  // Animated spinning line - draw one line at a time, erase previous
  static int lastAngle = -1;
  
  if (animFrame == 0) {
    display.fillScreen(HQVGA_GFX::BLACK);
    lastAngle = -1;
    
    // Draw static label
    display.setTextColor(HQVGA_GFX::WHITE);
    display.setTextSize(1);
    display.setCursor(50, 112);
    display.print("Spinning");
  }
  
  int angle = (animFrame * 6) % 360;  // Current angle
  
  // Erase previous line (draw in black)
  if (lastAngle >= 0) {
    float radOld = lastAngle * 3.14159 / 180.0;
    int xOld = 80 + cos(radOld) * 50;
    int yOld = 60 + sin(radOld) * 40;
    display.drawLine(80, 60, xOld, yOld, HQVGA_GFX::BLACK);
  }
  
  // Draw new line
  float rad = angle * 3.14159 / 180.0;
  int x = 80 + cos(rad) * 50;
  int y = 60 + sin(rad) * 40;
  uint8_t color = display.color332((angle * 2) % 256, (angle + 85) % 256, (255 - angle) % 256);
  display.drawLine(80, 60, x, y, color);
  
  // Leave a trail - draw fading lines
  for (int i = 1; i <= 8; i++) {
    int trailAngle = (angle - i * 6 + 360) % 360;
    float radTrail = trailAngle * 3.14159 / 180.0;
    int xTrail = 80 + cos(radTrail) * 50;
    int yTrail = 60 + sin(radTrail) * 40;
    uint8_t fade = 255 - i * 30;
    uint8_t trailColor = display.color332(fade / 4, fade / 4, fade / 8);
    display.drawLine(80, 60, xTrail, yTrail, trailColor);
  }
  
  // Center dot
  display.fillCircle(80, 60, 4, HQVGA_GFX::WHITE);
  
  lastAngle = angle;
}

void demoExpandingRects() {
  // Animated expanding rectangles - draw incrementally
  static bool centerDrawn = false;
  
  if (animFrame == 0) {
    display.fillScreen(HQVGA_GFX::BLACK);
    centerDrawn = false;
  }
  
  // Draw one new rectangle every few frames
  int rectIndex = (animFrame / 8) % 10;  // Which rectangle to draw
  int phase = animFrame % 8;  // Phase within each rectangle
  
  if (phase == 0 && rectIndex < 8) {
    // Draw a new rectangle
    int offset = rectIndex * 8;
    uint8_t r = (255 - rectIndex * 25) % 256;
    uint8_t g = (rectIndex * 30) % 256;
    uint8_t b = (128 + rectIndex * 15) % 256;
    uint8_t color = display.color332(r, g, b);
    display.drawRect(offset, offset, 160 - offset * 2, 120 - offset * 2, color);
  }
  
  // Draw center box once after a few rectangles are visible
  if (!centerDrawn && rectIndex >= 3) {
    display.fillRoundRect(50, 40, 60, 40, 8, HQVGA_GFX::CYAN);
    display.setTextColor(HQVGA_GFX::BLACK);
    display.setTextSize(1);
    display.setCursor(58, 55);
    display.print("HQVGA");
    centerDrawn = true;
  }
  
  // Reset and start over when we've drawn all rectangles
  if (rectIndex == 9 && phase == 0) {
    display.fillScreen(HQVGA_GFX::BLACK);
    centerDrawn = false;
  }
}
