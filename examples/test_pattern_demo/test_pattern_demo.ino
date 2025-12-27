/**
 * @file test_pattern_demo.ino
 * @brief Demonstrates all FPGA test pattern modes
 * 
 * This example cycles through the three available test patterns:
 * - Pattern 0: Color Bars (SMPTE-style display calibration)
 * - Pattern 1: Grid (32-pixel spacing for alignment)
 * - Pattern 2: Grayscale Gradient (black to white)
 * 
 * Hardware: Papilio Arcade board with HDMI output
 * Display: 720p HDMI output from Gowin FPGA
 * 
 * Test patterns are generated entirely in the FPGA at full 720p resolution
 * and are useful for:
 * - Display calibration and color accuracy testing
 * - Verifying HDMI signal integrity
 * - Checking display alignment and geometry
 * - Testing grayscale linearity
 */

#include <SPI.h>
#include <WishboneSPI.h>
#include <PapilioMCP.h>

// SPI Pin Configuration for ESP32-S3
#define SPI_CLK   12
#define SPI_MOSI  11
#define SPI_MISO  9
#define SPI_CS    10

// FPGA Register Addresses
#define VIDEO_MODE_REG      0x0000  // Video mode: 0=Test, 1=Text, 2=Framebuffer
#define TEST_PATTERN_REG    0x0010  // Test pattern: 0=Bars, 1=Grid, 2=Grayscale

// Video modes
#define MODE_TEST_PATTERN   0
#define MODE_TEXT           1
#define MODE_FRAMEBUFFER    2

// Test patterns
#define PATTERN_COLOR_BARS  0
#define PATTERN_GRID        1
#define PATTERN_GRAYSCALE   2

// Pattern names for display
const char* patternNames[] = {
    "Color Bars",
    "Grid Pattern",
    "Grayscale Gradient"
};

const char* patternDescriptions[] = {
    "SMPTE-style color bars for display calibration",
    "Red grid lines every 32 pixels for alignment",
    "Horizontal gradient from black to white"
};

// SPI instance
SPIClass* fpgaSPI = NULL;

// Demo state
int currentPattern = 0;
const int NUM_PATTERNS = 3;
unsigned long lastSwitch = 0;
const unsigned long PATTERN_DURATION = 4000;  // 4 seconds per pattern

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n=== Test Pattern Demo ===");
    Serial.println("Cycles through all FPGA test patterns");
    Serial.println();
    
    // Initialize MCP debug interface
    PapilioMCP.begin();
    
    // Initialize SPI
    fpgaSPI = new SPIClass(HSPI);
    fpgaSPI->begin(SPI_CLK, SPI_MISO, SPI_MOSI, SPI_CS);
    pinMode(SPI_CS, OUTPUT);
    digitalWrite(SPI_CS, HIGH);
    
    // Initialize Wishbone SPI (waits for FPGA)
    Serial.println("Waiting for FPGA...");
    wishboneInit(fpgaSPI, SPI_CS);
    
    // Set to test pattern video mode
    Serial.println("Setting video mode to Test Pattern...");
    wishboneWrite16(VIDEO_MODE_REG, MODE_TEST_PATTERN);
    delay(100);
    
    // Start with color bars
    setTestPattern(PATTERN_COLOR_BARS);
    
    Serial.println("\nTest patterns cycle automatically every 4 seconds");
    Serial.println("Press button (GPIO0) to manually cycle patterns\n");
}

void loop() {
    // Process MCP debug commands
    PapilioMCP.update();
    
    if (PapilioMCP.isPaused()) {
        delay(10);
        return;
    }
    
    // Auto-cycle patterns
    unsigned long now = millis();
    if (now - lastSwitch >= PATTERN_DURATION) {
        nextPattern();
        lastSwitch = now;
    }
    
    // Check for button press (active low)
    static bool lastButtonState = true;
    bool buttonState = digitalRead(0);
    if (lastButtonState && !buttonState) {
        // Button pressed - manual cycle
        nextPattern();
        lastSwitch = millis();  // Reset auto-cycle timer
    }
    lastButtonState = buttonState;
    
    // Breakpoint for debugging
    PapilioMCP.breakpoint("main_loop");
    
    delay(50);
}

void setTestPattern(int pattern) {
    pattern = pattern % NUM_PATTERNS;
    currentPattern = pattern;
    
    // Ensure we're in test pattern video mode
    wishboneWrite16(VIDEO_MODE_REG, MODE_TEST_PATTERN);
    delay(10);
    
    // Set the test pattern
    wishboneWrite16(TEST_PATTERN_REG, pattern);
    
    Serial.print("Pattern ");
    Serial.print(pattern);
    Serial.print(": ");
    Serial.println(patternNames[pattern]);
    Serial.print("  -> ");
    Serial.println(patternDescriptions[pattern]);
    Serial.println();
}

void nextPattern() {
    currentPattern = (currentPattern + 1) % NUM_PATTERNS;
    setTestPattern(currentPattern);
}

void previousPattern() {
    currentPattern = (currentPattern + NUM_PATTERNS - 1) % NUM_PATTERNS;
    setTestPattern(currentPattern);
}

// Get current pattern info
int getCurrentPattern() {
    return currentPattern;
}

const char* getCurrentPatternName() {
    return patternNames[currentPattern];
}

const char* getCurrentPatternDescription() {
    return patternDescriptions[currentPattern];
}
