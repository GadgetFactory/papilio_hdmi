/**
 * @file imagedec_demo.ino
 * @brief Image decoder demo for HQVGA 160x120 framebuffer
 * 
 * Demonstrates JPEG, PNG, and GIF decoding using bitbank2's libraries.
 * Images are decoded directly to the HQVGA framebuffer.
 * 
 * Hardware: Papilio Arcade board with HDMI output
 * Display: 160x120 scaled to 720p via FPGA
 * 
 * Note: This demo shows the capabilities with generated graphics.
 * For real use, load images from SPIFFS, SD card, or network.
 */

#include <HQVGA.h>
#include <PapilioMCP.h>

// Demo state
int currentDemo = 0;
int prevDemo = 0;  // Start at 0 to match currentDemo (avoid initial clear)
const int NUM_DEMOS = 4;
unsigned long lastSwitch = 0;
const unsigned long DEMO_DURATION = 3000;

// Demo init flags (reset when demo changes)
bool demoIntroInit = false;
bool demoButterfliesInit = false;
bool demoAnimationInit = false;
bool demoColorsInit = false;

// ============================================================================
// Embedded Butterfly Sprite (24x20 pixels, RGB332)
// Papilio = Latin for butterfly!
// RGB332: RRRGGGBB - Red=0xE0, Orange=0xF0, Yellow=0xFC, White=0xFF, Black=0x00
// ============================================================================
#define BUTTERFLY_W 24
#define BUTTERFLY_H 20

// Color definitions for sprite
#define ___ 0x00  // Transparent (black)
#define BRN 0x24  // Brown body (visible on black)
#define RED 0xE0  // Red
#define ORG 0xE4  // Orange  
#define YEL 0xFC  // Yellow
#define WHT 0xFF  // White

const uint8_t butterflySprite[BUTTERFLY_W * BUTTERFLY_H] = {
    // Orange Monarch butterfly - filled wings, visible body - 24x20
    // Row 0
    ___,___,___,___,___,RED,RED,___,___,___,___,___,___,___,___,___,RED,RED,___,___,___,___,___,___,
    // Row 1
    ___,___,___,___,RED,ORG,ORG,RED,___,___,___,BRN,BRN,___,___,RED,ORG,ORG,RED,___,___,___,___,___,
    // Row 2
    ___,___,___,RED,ORG,YEL,YEL,ORG,RED,___,___,BRN,BRN,___,RED,ORG,YEL,YEL,ORG,RED,___,___,___,___,
    // Row 3
    ___,___,RED,ORG,YEL,YEL,WHT,YEL,ORG,RED,___,BRN,BRN,RED,ORG,YEL,WHT,YEL,YEL,ORG,RED,___,___,___,
    // Row 4
    ___,RED,ORG,YEL,YEL,WHT,WHT,WHT,YEL,ORG,RED,BRN,BRN,ORG,YEL,WHT,WHT,WHT,YEL,YEL,ORG,RED,___,___,
    // Row 5
    ___,RED,ORG,YEL,WHT,WHT,WHT,WHT,WHT,YEL,ORG,BRN,BRN,YEL,WHT,WHT,WHT,WHT,WHT,YEL,ORG,RED,___,___,
    // Row 6
    RED,ORG,YEL,YEL,WHT,WHT,WHT,WHT,WHT,WHT,YEL,BRN,BRN,WHT,WHT,WHT,WHT,WHT,WHT,YEL,YEL,ORG,RED,___,
    // Row 7
    RED,ORG,YEL,WHT,WHT,WHT,WHT,WHT,WHT,WHT,YEL,BRN,BRN,WHT,WHT,WHT,WHT,WHT,WHT,WHT,YEL,ORG,RED,___,
    // Row 8
    RED,ORG,YEL,WHT,WHT,WHT,WHT,WHT,WHT,YEL,ORG,BRN,BRN,YEL,WHT,WHT,WHT,WHT,WHT,WHT,YEL,ORG,RED,___,
    // Row 9 - middle
    RED,ORG,YEL,WHT,WHT,WHT,WHT,WHT,YEL,ORG,RED,BRN,BRN,ORG,YEL,WHT,WHT,WHT,WHT,WHT,YEL,ORG,RED,___,
    // Row 10
    RED,ORG,YEL,WHT,WHT,WHT,WHT,WHT,YEL,ORG,RED,BRN,BRN,ORG,YEL,WHT,WHT,WHT,WHT,WHT,YEL,ORG,RED,___,
    // Row 11
    RED,ORG,YEL,WHT,WHT,WHT,WHT,WHT,WHT,YEL,ORG,BRN,BRN,YEL,WHT,WHT,WHT,WHT,WHT,WHT,YEL,ORG,RED,___,
    // Row 12
    RED,ORG,YEL,WHT,WHT,WHT,WHT,WHT,WHT,WHT,YEL,BRN,BRN,WHT,WHT,WHT,WHT,WHT,WHT,WHT,YEL,ORG,RED,___,
    // Row 13
    RED,ORG,YEL,YEL,WHT,WHT,WHT,WHT,WHT,WHT,YEL,BRN,BRN,WHT,WHT,WHT,WHT,WHT,WHT,YEL,YEL,ORG,RED,___,
    // Row 14
    ___,RED,ORG,YEL,WHT,WHT,WHT,WHT,WHT,YEL,ORG,BRN,BRN,YEL,WHT,WHT,WHT,WHT,WHT,YEL,ORG,RED,___,___,
    // Row 15
    ___,RED,ORG,YEL,YEL,WHT,WHT,WHT,YEL,ORG,RED,BRN,BRN,ORG,YEL,WHT,WHT,WHT,YEL,YEL,ORG,RED,___,___,
    // Row 16
    ___,___,RED,ORG,YEL,YEL,WHT,YEL,ORG,RED,___,BRN,BRN,RED,ORG,YEL,WHT,YEL,YEL,ORG,RED,___,___,___,
    // Row 17
    ___,___,___,RED,ORG,YEL,YEL,ORG,RED,___,___,BRN,BRN,___,RED,ORG,YEL,YEL,ORG,RED,___,___,___,___,
    // Row 18
    ___,___,___,___,RED,ORG,ORG,RED,___,___,___,BRN,BRN,___,___,RED,ORG,ORG,RED,___,___,___,___,___,
    // Row 19
    ___,___,___,___,___,RED,RED,___,___,___,___,___,___,___,___,___,RED,RED,___,___,___,___,___,___,
};

// Blue butterfly
#define BLU 0x03  // Blue
#define CYN 0x1F  // Cyan
#define LBL 0x17  // Light blue

const uint8_t butterfly2Sprite[BUTTERFLY_W * BUTTERFLY_H] = {
    // Blue butterfly - filled wings - 24x20
    // Row 0
    ___,___,___,___,___,BLU,BLU,___,___,___,___,___,___,___,___,___,BLU,BLU,___,___,___,___,___,___,
    // Row 1
    ___,___,___,___,BLU,CYN,CYN,BLU,___,___,___,BRN,BRN,___,___,BLU,CYN,CYN,BLU,___,___,___,___,___,
    // Row 2
    ___,___,___,BLU,CYN,LBL,LBL,CYN,BLU,___,___,BRN,BRN,___,BLU,CYN,LBL,LBL,CYN,BLU,___,___,___,___,
    // Row 3
    ___,___,BLU,CYN,LBL,LBL,WHT,LBL,CYN,BLU,___,BRN,BRN,BLU,CYN,LBL,WHT,LBL,LBL,CYN,BLU,___,___,___,
    // Row 4
    ___,BLU,CYN,LBL,LBL,WHT,WHT,WHT,LBL,CYN,BLU,BRN,BRN,CYN,LBL,WHT,WHT,WHT,LBL,LBL,CYN,BLU,___,___,
    // Row 5
    ___,BLU,CYN,LBL,WHT,WHT,WHT,WHT,WHT,LBL,CYN,BRN,BRN,LBL,WHT,WHT,WHT,WHT,WHT,LBL,CYN,BLU,___,___,
    // Row 6
    BLU,CYN,LBL,LBL,WHT,WHT,WHT,WHT,WHT,WHT,LBL,BRN,BRN,WHT,WHT,WHT,WHT,WHT,WHT,LBL,LBL,CYN,BLU,___,
    // Row 7
    BLU,CYN,LBL,WHT,WHT,WHT,WHT,WHT,WHT,WHT,LBL,BRN,BRN,WHT,WHT,WHT,WHT,WHT,WHT,WHT,LBL,CYN,BLU,___,
    // Row 8
    BLU,CYN,LBL,WHT,WHT,WHT,WHT,WHT,WHT,LBL,CYN,BRN,BRN,LBL,WHT,WHT,WHT,WHT,WHT,WHT,LBL,CYN,BLU,___,
    // Row 9 - middle
    BLU,CYN,LBL,WHT,WHT,WHT,WHT,WHT,LBL,CYN,BLU,BRN,BRN,CYN,LBL,WHT,WHT,WHT,WHT,WHT,LBL,CYN,BLU,___,
    // Row 10
    BLU,CYN,LBL,WHT,WHT,WHT,WHT,WHT,LBL,CYN,BLU,BRN,BRN,CYN,LBL,WHT,WHT,WHT,WHT,WHT,LBL,CYN,BLU,___,
    // Row 11
    BLU,CYN,LBL,WHT,WHT,WHT,WHT,WHT,WHT,LBL,CYN,BRN,BRN,LBL,WHT,WHT,WHT,WHT,WHT,WHT,LBL,CYN,BLU,___,
    // Row 12
    BLU,CYN,LBL,WHT,WHT,WHT,WHT,WHT,WHT,WHT,LBL,BRN,BRN,WHT,WHT,WHT,WHT,WHT,WHT,WHT,LBL,CYN,BLU,___,
    // Row 13
    BLU,CYN,LBL,LBL,WHT,WHT,WHT,WHT,WHT,WHT,LBL,BRN,BRN,WHT,WHT,WHT,WHT,WHT,WHT,LBL,LBL,CYN,BLU,___,
    // Row 14
    ___,BLU,CYN,LBL,WHT,WHT,WHT,WHT,WHT,LBL,CYN,BRN,BRN,LBL,WHT,WHT,WHT,WHT,WHT,LBL,CYN,BLU,___,___,
    // Row 15
    ___,BLU,CYN,LBL,LBL,WHT,WHT,WHT,LBL,CYN,BLU,BRN,BRN,CYN,LBL,WHT,WHT,WHT,LBL,LBL,CYN,BLU,___,___,
    // Row 16
    ___,___,BLU,CYN,LBL,LBL,WHT,LBL,CYN,BLU,___,BRN,BRN,BLU,CYN,LBL,WHT,LBL,LBL,CYN,BLU,___,___,___,
    // Row 17
    ___,___,___,BLU,CYN,LBL,LBL,CYN,BLU,___,___,BRN,BRN,___,BLU,CYN,LBL,LBL,CYN,BLU,___,___,___,___,
    // Row 18
    ___,___,___,___,BLU,CYN,CYN,BLU,___,___,___,BRN,BRN,___,___,BLU,CYN,CYN,BLU,___,___,___,___,___,
    // Row 19
    ___,___,___,___,___,BLU,BLU,___,___,___,___,___,___,___,___,___,BLU,BLU,___,___,___,___,___,___,
};

// Draw a sprite to the VGA display
void drawSprite(int x, int y, const uint8_t* sprite, int w, int h) {
    for (int row = 0; row < h; row++) {
        for (int col = 0; col < w; col++) {
            uint8_t pixel = sprite[row * w + col];
            if (pixel != 0) {  // 0 = transparent
                VGA.putPixel(x + col, y + row, pixel);
            }
        }
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n=== Papilio Image Demo ===");
    Serial.println("Papilio = Latin for butterfly!");
    
    // Initialize MCP debug interface
    PapilioMCP.begin();
    
    // Initialize VGA
    VGA.begin();
    VGA.waitForFPGA();
    VGA.setVideoMode(2);  // Framebuffer mode
    VGA.clear();
    
    Serial.println("Display initialized: 160x120");
}

void loop() {
    // Process MCP debug commands
    PapilioMCP.update();
    
    // Skip sketch logic when MCP is paused (allows MCP full control)
    if (PapilioMCP.isPaused()) {
        delay(10);
        return;
    }
    
    unsigned long now = millis();
    
    if (now - lastSwitch >= DEMO_DURATION) {
        lastSwitch = now;
        currentDemo = (currentDemo + 1) % NUM_DEMOS;
    }
    
    // Reset init flags when demo changes
    if (currentDemo != prevDemo) {
        prevDemo = currentDemo;
        demoIntroInit = false;
        demoButterfliesInit = false;
        demoAnimationInit = false;
        demoColorsInit = false;
        VGA.clear();
    }
    
    switch (currentDemo) {
        case 0: demoIntro(); break;
        case 1: demoButterflies(); break;
        case 2: demoAnimation(); break;
        case 3: demoColors(); break;
    }
    
    delay(16);
}

// Demo 0: Introduction
void demoIntro() {
    if (!demoIntroInit) {
        demoIntroInit = true;
        
        // Draw gradient background (sky)
        for (int y = 0; y < 60; y++) {
            uint8_t blue = 0x03 - (y / 30);  // Light to dark blue
            for (int x = 0; x < 160; x++) {
                VGA.putPixel(x, y, blue);
            }
        }
        // Green grass
        for (int y = 60; y < 120; y++) {
            uint8_t green = 0x1C - ((y - 60) / 30);
            for (int x = 0; x < 160; x++) {
                VGA.putPixel(x, y, green);
            }
        }
        
        // Draw butterflies
        drawSprite(20, 30, butterflySprite, BUTTERFLY_W, BUTTERFLY_H);
        drawSprite(100, 20, butterfly2Sprite, BUTTERFLY_W, BUTTERFLY_H);
        
        // Title
        VGA.setColor(WHITE);
        VGA.printtext(35, 75, "PAPILIO", false);
        VGA.setColor(YELLOW);
        VGA.printtext(25, 90, "\"Butterfly\"", false);
        VGA.setColor(0x1C);
        VGA.printtext(30, 105, "in Latin", false);
        
        // Pause for MCP screenshot
        PapilioMCP.breakpoint("intro_drawn");
    }
}

// Demo 1: Multiple butterflies
void demoButterflies() {
    static unsigned long lastFrame = 0;
    static int frame = 0;
    static int prevOffsets[5][2];  // Store previous positions for erasing
    
    if (!demoButterfliesInit) {
        demoButterfliesInit = true;
        VGA.clear();
        frame = 0;
        
        // Initialize previous positions to off-screen
        for (int i = 0; i < 5; i++) {
            prevOffsets[i][0] = -100;
            prevOffsets[i][1] = -100;
        }
        
        VGA.setColor(WHITE);
        VGA.printtext(20, 5, "Papilio Gallery", false);
    }
    
    // Animate butterflies
    if (millis() - lastFrame > 100) {
        lastFrame = millis();
        frame++;
        
        // Calculate new positions
        int offset1 = (int)(sin(frame * 0.1) * 10);
        int offset2 = (int)(cos(frame * 0.15) * 8);
        int offset3 = (int)(sin(frame * 0.08 + 1) * 12);
        
        int positions[5][2] = {
            {10 + offset1, 25},
            {70 + offset2, 40},
            {120 + offset3, 30},
            {40 - offset2, 70},
            {90 - offset1, 80}
        };
        
        // Erase previous butterflies (draw black rectangles)
        for (int i = 0; i < 5; i++) {
            int px = prevOffsets[i][0];
            int py = prevOffsets[i][1];
            if (px > -50) {  // Only if was on screen
                for (int row = 0; row < BUTTERFLY_H; row++) {
                    for (int col = 0; col < BUTTERFLY_W; col++) {
                        int x = px + col;
                        int y = py + row;
                        if (x >= 0 && x < 160 && y >= 20 && y < 110) {
                            VGA.putPixel(x, y, 0);
                        }
                    }
                }
            }
        }
        
        // Draw butterflies at new positions
        drawSprite(positions[0][0], positions[0][1], butterflySprite, BUTTERFLY_W, BUTTERFLY_H);
        drawSprite(positions[1][0], positions[1][1], butterfly2Sprite, BUTTERFLY_W, BUTTERFLY_H);
        drawSprite(positions[2][0], positions[2][1], butterflySprite, BUTTERFLY_W, BUTTERFLY_H);
        drawSprite(positions[3][0], positions[3][1], butterfly2Sprite, BUTTERFLY_W, BUTTERFLY_H);
        drawSprite(positions[4][0], positions[4][1], butterflySprite, BUTTERFLY_W, BUTTERFLY_H);
        
        // Save current positions for next frame
        for (int i = 0; i < 5; i++) {
            prevOffsets[i][0] = positions[i][0];
            prevOffsets[i][1] = positions[i][1];
        }
    }
}

// Demo 2: Flying animation
void demoAnimation() {
    static unsigned long lastFrame = 0;
    static float butterflyX = 0;
    static float butterflyY = 60;
    static float velX = 1.5;
    static float velY = 0.5;
    static int frame = 0;
    static int prevX = -100, prevY = -100;
    
    // Draw background once
    if (!demoAnimationInit) {
        demoAnimationInit = true;
        
        // Draw sky gradient
        for (int y = 0; y < 120; y++) {
            uint8_t color = (y < 80) ? (0x03 - y/40) : (0x04 + (y-80)/20);
            for (int x = 0; x < 160; x++) {
                VGA.putPixel(x, y, color);
            }
        }
        
        // Title
        VGA.setColor(WHITE);
        VGA.printtext(30, 5, "Flying Free", false);
        
        prevX = -100;
        prevY = -100;
    }
    
    if (millis() - lastFrame > 33) {  // ~30 FPS
        lastFrame = millis();
        frame++;
        
        // Erase previous butterfly by redrawing background at that spot
        if (prevX > -50) {
            for (int row = 0; row < BUTTERFLY_H; row++) {
                for (int col = 0; col < BUTTERFLY_W; col++) {
                    int x = prevX + col;
                    int y = prevY + row;
                    if (x >= 0 && x < 160 && y >= 0 && y < 120) {
                        // Recalculate gradient color for this pixel
                        uint8_t color = (y < 80) ? (0x03 - y/40) : (0x04 + (y-80)/20);
                        VGA.putPixel(x, y, color);
                    }
                }
            }
        }
        
        // Update butterfly position
        butterflyX += velX;
        butterflyY += velY + sin(frame * 0.2) * 0.5;
        
        // Bounce off edges
        if (butterflyX < 0 || butterflyX > 128) velX = -velX;
        if (butterflyY < 10 || butterflyY > 90) velY = -velY;
        
        // Choose sprite based on direction
        const uint8_t* sprite = (velX > 0) ? butterflySprite : butterfly2Sprite;
        drawSprite((int)butterflyX, (int)butterflyY, sprite, BUTTERFLY_W, BUTTERFLY_H);
        
        // Save position for erasing next frame
        prevX = (int)butterflyX;
        prevY = (int)butterflyY;
    }
}

// Demo 3: Color showcase
void demoColors() {
    if (!demoColorsInit) {
        demoColorsInit = true;
        VGA.clear();
        
        VGA.setColor(WHITE);
        VGA.printtext(15, 5, "RGB332 Butterflies", false);
        
        // Draw butterflies with different color tints
        // Orange/Yellow butterfly (original)
        drawSprite(10, 25, butterflySprite, BUTTERFLY_W, BUTTERFLY_H);
        VGA.setColor(YELLOW);
        VGA.printtext(8, 52, "Orange", false);
        
        // Blue/Cyan butterfly  
        drawSprite(60, 25, butterfly2Sprite, BUTTERFLY_W, BUTTERFLY_H);
        VGA.setColor(CYAN);
        VGA.printtext(65, 52, "Blue", false);
        
        // Create a purple tinted version
        for (int row = 0; row < BUTTERFLY_H; row++) {
            for (int col = 0; col < BUTTERFLY_W; col++) {
                uint8_t pixel = butterflySprite[row * BUTTERFLY_W + col];
                if (pixel != 0) {
                    // Tint toward purple (reduce green)
                    uint8_t r = (pixel >> 5) & 0x07;
                    uint8_t g = (pixel >> 2) & 0x07;
                    uint8_t b = pixel & 0x03;
                    g = g / 2;  // Reduce green
                    b = 3;      // Max blue
                    uint8_t newColor = (r << 5) | (g << 2) | b;
                    VGA.putPixel(110 + col, 25 + row, newColor);
                }
            }
        }
        VGA.setColor(PURPLE);
        VGA.printtext(110, 52, "Purple", false);
        
        // Draw green tinted butterfly
        for (int row = 0; row < BUTTERFLY_H; row++) {
            for (int col = 0; col < BUTTERFLY_W; col++) {
                uint8_t pixel = butterfly2Sprite[row * BUTTERFLY_W + col];
                if (pixel != 0) {
                    uint8_t r = (pixel >> 5) & 0x07;
                    uint8_t g = (pixel >> 2) & 0x07;
                    uint8_t b = pixel & 0x03;
                    r = r / 2;
                    g = 7;  // Max green
                    b = b / 2;
                    uint8_t newColor = (r << 5) | (g << 2) | b;
                    VGA.putPixel(35 + col, 70 + row, newColor);
                }
            }
        }
        VGA.setColor(GREEN);
        VGA.printtext(35, 97, "Green", false);
        
        // Red tinted
        for (int row = 0; row < BUTTERFLY_H; row++) {
            for (int col = 0; col < BUTTERFLY_W; col++) {
                uint8_t pixel = butterflySprite[row * BUTTERFLY_W + col];
                if (pixel != 0) {
                    uint8_t r = 7;  // Max red
                    uint8_t g = ((pixel >> 2) & 0x07) / 3;
                    uint8_t b = 0;
                    uint8_t newColor = (r << 5) | (g << 2) | b;
                    VGA.putPixel(90 + col, 70 + row, newColor);
                }
            }
        }
        VGA.setColor(RED);
        VGA.printtext(95, 97, "Red", false);
    }
}
