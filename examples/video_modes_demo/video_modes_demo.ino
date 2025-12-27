/**
 * @file video_modes_demo.ino
 * @brief Demonstrates all FPGA video output modes
 * 
 * This example cycles through the three available video modes:
 * - Mode 0: Test Pattern (color bars for display calibration)
 * - Mode 1: Text Mode (80x26 character display with CGA colors)
 * - Mode 2: Framebuffer Mode (160x120 pixel graphics)
 * 
 * Hardware: Papilio Arcade board with HDMI output
 * Display: 720p HDMI output from Gowin FPGA
 */

#include <HQVGA.h>
#include <PapilioMCP.h>

// Demo state
int currentMode = 0;
const int NUM_MODES = 3;
unsigned long lastSwitch = 0;
const unsigned long MODE_DURATION = 5000;  // 5 seconds per mode

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n=== Video Modes Demo ===");
    Serial.println("Cycles through all FPGA video output modes");
    Serial.println();
    
    // Initialize MCP debug interface
    PapilioMCP.begin();
    
    // Initialize VGA
    VGA.begin();
    VGA.waitForFPGA();
    
    Serial.println("Starting with Test Pattern mode...");
    showTestPattern();
}

void loop() {
    // Process MCP debug commands
    PapilioMCP.update();
    
    if (PapilioMCP.isPaused()) {
        delay(10);
        return;
    }
    
    unsigned long now = millis();
    
    if (now - lastSwitch >= MODE_DURATION) {
        lastSwitch = now;
        currentMode = (currentMode + 1) % NUM_MODES;
        
        switch (currentMode) {
            case 0: showTestPattern(); break;
            case 1: showTextMode(); break;
            case 2: showFramebuffer(); break;
        }
    }
    
    // Update animations based on current mode
    if (currentMode == 2) {
        updateFramebufferAnimation();
    }
    
    delay(16);
}

// ============================================================================
// Mode 0: Test Pattern
// ============================================================================
void showTestPattern() {
    Serial.println("Mode 0: Test Pattern (Color Bars)");
    VGA.setVideoMode(0);
    
    // The FPGA generates the test pattern automatically
    // No additional drawing needed
    
    PapilioMCP.breakpoint("test_pattern");
}

// ============================================================================
// Mode 1: Text Mode (80x26)
// ============================================================================
void showTextMode() {
    Serial.println("Mode 1: Text Mode (80x26)");
    VGA.setVideoMode(1);
    
    // Clear and draw text demo
    // Note: Text mode uses different Wishbone registers
    // For this demo, we'll show a simple message
    
    // The text mode requires direct Wishbone writes to text registers
    // This is a simplified demo - see text_mode_test for full example
    
    PapilioMCP.breakpoint("text_mode");
}

// ============================================================================
// Mode 2: Framebuffer Mode (160x120)
// ============================================================================
static int animFrame = 0;

void showFramebuffer() {
    Serial.println("Mode 2: Framebuffer (160x120)");
    VGA.setVideoMode(2);
    VGA.clear();
    
    // Draw a colorful demo screen
    drawFramebufferDemo();
    
    PapilioMCP.breakpoint("framebuffer");
}

void drawFramebufferDemo() {
    // Clear to black by filling entire screen
    VGA.setColor(0);  // Black
    VGA.drawRect(0, 0, 160, 120);
    
    // Draw title
    VGA.setColor(WHITE);
    VGA.printtext(25, 10, "FRAMEBUFFER MODE", false);
    VGA.setColor(YELLOW);
    VGA.printtext(40, 25, "160x120 pixels", false);
    
    // Draw color palette
    VGA.setColor(WHITE);
    VGA.printtext(10, 45, "RGB332 Colors:", false);
    
    // Draw color squares
    int startX = 10;
    int startY = 55;
    int size = 10;
    int spacing = 12;
    
    // Red gradient
    for (int i = 0; i < 8; i++) {
        uint8_t color = (i << 5);  // Red bits only
        VGA.setColor(color);
        VGA.drawRect(startX + i * spacing, startY, size, size);
    }
    
    // Green gradient
    for (int i = 0; i < 8; i++) {
        uint8_t color = (i << 2);  // Green bits only
        VGA.setColor(color);
        VGA.drawRect(startX + i * spacing, startY + 15, size, size);
    }
    
    // Blue gradient
    for (int i = 0; i < 4; i++) {
        uint8_t color = i;  // Blue bits only
        VGA.setColor(color);
        VGA.drawRect(startX + i * spacing * 2, startY + 30, size * 2, size);
    }
    
    // Draw info
    VGA.setColor(CYAN);
    VGA.printtext(10, 95, "ESP32-S3 + Gowin FPGA", false);
    VGA.setColor(PURPLE);
    VGA.printtext(10, 107, "Papilio Arcade", false);
}

void updateFramebufferAnimation() {
    // Static demo - no animation needed
    // The framebuffer demo is static to show the color capabilities
    animFrame++;
}
