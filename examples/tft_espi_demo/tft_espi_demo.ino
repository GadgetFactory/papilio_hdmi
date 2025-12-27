/**
 * @file tft_espi_demo.ino
 * @brief TFT_eSPI-style API demo for HQVGA 160x120 framebuffer
 * 
 * Demonstrates the TFT_eSPI-compatible adapter with various
 * drawing primitives and text functions. This allows easy
 * porting of existing TFT_eSPI projects.
 * 
 * Hardware: Papilio Arcade board with HDMI output
 * Display: 160x120 scaled to 720p via FPGA
 */

#include <HQVGA_TFT_eSPI.h>

HQVGA_TFT tft;

// Demo state
int currentDemo = 0;
int prevDemo = -1;  // Track demo changes to reset static flags
const int NUM_DEMOS = 5;
unsigned long lastSwitch = 0;
const unsigned long DEMO_DURATION = 4000;

void setup() {
    Serial.begin(115200);
    Serial.println("TFT_eSPI Demo Starting...");
    
    tft.begin();
    tft.fillScreen(TFT_BLACK);
    
    Serial.println("Display initialized!");
    Serial.printf("Resolution: %dx%d\n", tft.width(), tft.height());
}

void loop() {
    unsigned long now = millis();
    
    if (now - lastSwitch >= DEMO_DURATION) {
        lastSwitch = now;
        currentDemo = (currentDemo + 1) % NUM_DEMOS;
        tft.fillScreen(TFT_BLACK);
    }
    
    // Track demo changes
    bool demoChanged = (currentDemo != prevDemo);
    prevDemo = currentDemo;
    
    switch (currentDemo) {
        case 0: demoShapes(demoChanged); break;
        case 1: demoText(demoChanged); break;
        case 2: demoColors(demoChanged); break;
        case 3: demoAnimation(demoChanged); break;
        case 4: demoGauge(demoChanged); break;
    }
    
    delay(16);  // ~60 FPS
}

// Demo 1: Basic shapes (uses buffered mode to prevent flickering)
void demoShapes(bool reset) {
    static bool initialized = false;
    static unsigned long lastFrame = 0;
    
    if (reset) initialized = false;
    
    if (!initialized || millis() - lastFrame > 100) {
        initialized = true;
        lastFrame = millis();
        
        // Enable buffered mode
        tft.startBuffered();
        
        tft.fillScreen(TFT_NAVY);
        
        // Title
        tft.setTextColor(TFT_WHITE);
        tft.setTextSize(1);
        tft.setTextDatum(TC_DATUM);
        tft.drawString("TFT_eSPI Shapes", 80, 2);
        
        // Rectangles
        tft.drawRect(5, 15, 30, 20, TFT_RED);
        tft.fillRect(40, 15, 30, 20, TFT_GREEN);
        tft.fillRoundRect(75, 15, 30, 20, 5, TFT_BLUE);
        tft.drawRoundRect(110, 15, 30, 20, 5, TFT_YELLOW);
        
        // Circles
        tft.drawCircle(20, 55, 12, TFT_CYAN);
        tft.fillCircle(55, 55, 12, TFT_MAGENTA);
        
        // Triangles
        tft.drawTriangle(80, 68, 95, 43, 110, 68, TFT_ORANGE);
        tft.fillTriangle(115, 68, 130, 43, 145, 68, TFT_PINK);
        
        // Lines
        for (int i = 0; i < 8; i++) {
            uint16_t color = (i & 1) ? TFT_WHITE : TFT_LIGHTGREY;
            tft.drawLine(5 + i * 18, 75, 20 + i * 18, 95, color);
        }
        
        // Status bar
        tft.setTextDatum(BC_DATUM);
        tft.setTextColor(TFT_DARKGREY);
        tft.drawString("Shapes Demo", 80, 118);
        
        // Sync buffer to display
        tft.endBuffered();
        tft.syncBuffer();
    }
}

// Demo 2: Text rendering (uses buffered mode)
void demoText(bool reset) {
    static bool initialized = false;
    
    if (reset) initialized = false;
    
    if (!initialized) {
        initialized = true;
        
        // Enable buffered mode
        tft.startBuffered();
        
        tft.fillScreen(TFT_BLACK);
        
        // Different text sizes
        tft.setTextColor(TFT_WHITE);
        tft.setTextSize(1);
        tft.setCursor(5, 5);
        tft.print("Size 1: Hello!");
        
        tft.setTextSize(2);
        tft.setTextColor(TFT_YELLOW);
        tft.setCursor(5, 18);
        tft.print("Size 2");
        
        // Text alignment
        tft.setTextSize(1);
        
        // Left aligned
        tft.setTextColor(TFT_RED);
        tft.setTextDatum(TL_DATUM);
        tft.drawString("Left", 5, 45);
        
        // Center aligned
        tft.setTextColor(TFT_GREEN);
        tft.setTextDatum(TC_DATUM);
        tft.drawString("Center", 80, 45);
        
        // Right aligned
        tft.setTextColor(TFT_BLUE);
        tft.setTextDatum(TR_DATUM);
        tft.drawString("Right", 155, 45);
        
        // Horizontal line
        tft.drawFastHLine(5, 55, 150, TFT_DARKGREY);
        
        // Numbers
        tft.setTextColor(TFT_CYAN);
        tft.setTextDatum(TL_DATUM);
        tft.setCursor(5, 60);
        tft.print("Int: ");
        tft.print(12345);
        
        tft.setCursor(5, 72);
        tft.print("Float: ");
        tft.setTextDatum(TL_DATUM);
        tft.drawFloat(3.14159, 2, 47, 72);
        
        // Text with background
        tft.setTextColor(TFT_BLACK, TFT_WHITE);
        tft.drawString(" Inverted ", 5, 88);
        
        tft.setTextColor(TFT_WHITE, TFT_RED);
        tft.drawString(" Alert! ", 70, 88);
        
        // Status
        tft.setTextColor(TFT_DARKGREY);
        tft.setTextDatum(BC_DATUM);
        tft.drawString("Text Demo", 80, 118);
        
        // Sync buffer to display
        tft.endBuffered();
        tft.syncBuffer();
    }
}

// Demo 3: Color palette (uses buffered mode)
void demoColors(bool reset) {
    static bool initialized = false;
    
    if (reset) initialized = false;
    
    if (!initialized) {
        initialized = true;
        
        // Enable buffered mode
        tft.startBuffered();
        
        tft.fillScreen(TFT_BLACK);
        
        // Title
        tft.setTextColor(TFT_WHITE);
        tft.setTextSize(1);
        tft.setTextDatum(TC_DATUM);
        tft.drawString("RGB332 Palette", 80, 2);
        
        // Color swatches - 8 colors per row, 4 rows
        const uint16_t colors[] = {
            TFT_BLACK, TFT_NAVY, TFT_DARKGREEN, TFT_DARKCYAN,
            TFT_MAROON, TFT_PURPLE, TFT_OLIVE, TFT_LIGHTGREY,
            TFT_DARKGREY, TFT_BLUE, TFT_GREEN, TFT_CYAN,
            TFT_RED, TFT_MAGENTA, TFT_YELLOW, TFT_WHITE,
            TFT_ORANGE, TFT_GREENYELLOW, TFT_PINK, TFT_BROWN,
            TFT_GOLD, TFT_SILVER, TFT_SKYBLUE, TFT_VIOLET
        };
        
        const char* names[] = {
            "BLK", "NVY", "DGN", "DCY",
            "MRN", "PUR", "OLV", "LGY",
            "DGY", "BLU", "GRN", "CYN",
            "RED", "MAG", "YLW", "WHT",
            "ORG", "GYL", "PNK", "BRN",
            "GLD", "SLV", "SKY", "VIO"
        };
        
        int cols = 6;
        int boxW = 24;
        int boxH = 18;
        int startX = 4;
        int startY = 14;
        
        for (int i = 0; i < 24; i++) {
            int col = i % cols;
            int row = i / cols;
            int x = startX + col * (boxW + 2);
            int y = startY + row * (boxH + 6);
            
            // Color box
            tft.fillRect(x, y, boxW, boxH, colors[i]);
            tft.drawRect(x, y, boxW, boxH, TFT_WHITE);
            
            // Label
            tft.setTextColor(TFT_LIGHTGREY);
            tft.setTextDatum(TC_DATUM);
            tft.drawString(names[i], x + boxW/2, y + boxH + 1);
        }
        
        // Status
        tft.setTextColor(TFT_DARKGREY);
        tft.setTextDatum(BC_DATUM);
        tft.drawString("Color Palette", 80, 118);
        
        // Sync buffer to display
        tft.endBuffered();
        tft.syncBuffer();
    }
}

// Demo 4: Animation (uses buffered mode to prevent flickering)
void demoAnimation(bool reset) {
    static float angle = 0;
    static int frameCount = 0;
    
    if (reset) {
        angle = 0;
        frameCount = 0;
    }
    
    // Enable buffered mode - draw to local buffer first
    tft.startBuffered();
    
    // Clear only the animated area
    tft.fillRect(0, 10, 160, 95, TFT_BLACK);
    
    // Title
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(1);
    tft.setTextDatum(TC_DATUM);
    tft.drawString("Animation", 80, 2);
    
    // Spinning line
    int cx = 40, cy = 55;
    int len = 25;
    int x1 = cx + cos(angle) * len;
    int y1 = cy + sin(angle) * len;
    int x2 = cx - cos(angle) * len;
    int y2 = cy - sin(angle) * len;
    tft.drawLine(x1, y1, x2, y2, TFT_CYAN);
    tft.fillCircle(cx, cy, 3, TFT_WHITE);
    
    // Bouncing ball
    static float ballX = 120, ballY = 50;
    static float velX = 1.5, velY = 1.2;
    
    ballX += velX;
    ballY += velY;
    
    if (ballX < 85 || ballX > 155) velX = -velX;
    if (ballY < 20 || ballY > 90) velY = -velY;
    
    tft.fillCircle(ballX, ballY, 6, TFT_RED);
    tft.drawCircle(ballX, ballY, 6, TFT_WHITE);
    
    // Color cycling rectangle
    uint8_t hue = frameCount * 3;
    uint16_t color = tft.color565(
        (sin(hue * 0.0245) + 1) * 127,
        (sin(hue * 0.0245 + 2.094) + 1) * 127,
        (sin(hue * 0.0245 + 4.188) + 1) * 127
    );
    tft.fillRect(5, 85, 150, 8, color);
    
    // FPS counter
    tft.setTextColor(TFT_GREEN);
    tft.setTextDatum(BL_DATUM);
    char fps[16];
    snprintf(fps, sizeof(fps), "Frame: %d", frameCount % 1000);
    tft.fillRect(0, 108, 80, 12, TFT_BLACK);
    tft.drawString(fps, 5, 118);
    
    // Now sync the entire buffer to FPGA in one go
    tft.endBuffered();
    tft.syncBuffer();
    
    angle += 0.1;
    frameCount++;
}

// Demo 5: Gauge display (uses buffered mode to prevent flickering)
void demoGauge(bool reset) {
    static int value = 0;
    static int direction = 1;
    static unsigned long lastUpdate = 0;
    
    if (reset) {
        value = 0;
        direction = 1;
    }
    
    if (millis() - lastUpdate < 50) return;
    lastUpdate = millis();
    
    // Update value
    value += direction * 2;
    if (value >= 100 || value <= 0) direction = -direction;
    
    // Enable buffered mode - draw to local buffer first
    tft.startBuffered();
    
    // Clear gauge area
    tft.fillRect(0, 0, 160, 105, TFT_BLACK);
    
    // Title
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(1);
    tft.setTextDatum(TC_DATUM);
    tft.drawString("Gauge Demo", 80, 2);
    
    // Horizontal bar gauge
    int barX = 10, barY = 18;
    int barW = 140, barH = 15;
    int fillW = (barW - 4) * value / 100;
    
    tft.drawRect(barX, barY, barW, barH, TFT_WHITE);
    
    // Color based on value
    uint16_t barColor;
    if (value < 30) barColor = TFT_GREEN;
    else if (value < 70) barColor = TFT_YELLOW;
    else barColor = TFT_RED;
    
    tft.fillRect(barX + 2, barY + 2, fillW, barH - 4, barColor);
    
    // Percentage text
    char pct[8];
    snprintf(pct, sizeof(pct), "%d%%", value);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(value > 50 ? TFT_BLACK : TFT_WHITE);
    tft.drawString(pct, barX + barW/2, barY + barH/2 + 1);
    
    // Vertical bar gauges
    int vBarW = 18, vBarH = 50;
    int vBarY = 42;
    
    for (int i = 0; i < 5; i++) {
        int vBarX = 15 + i * 28;
        int vValue = (value + i * 20) % 100;
        int fillH = (vBarH - 4) * vValue / 100;
        
        tft.drawRect(vBarX, vBarY, vBarW, vBarH, TFT_LIGHTGREY);
        
        // Gradient fill
        uint16_t vColor;
        if (i == 0) vColor = TFT_CYAN;
        else if (i == 1) vColor = TFT_GREEN;
        else if (i == 2) vColor = TFT_YELLOW;
        else if (i == 3) vColor = TFT_ORANGE;
        else vColor = TFT_RED;
        
        tft.fillRect(vBarX + 2, vBarY + vBarH - 2 - fillH, vBarW - 4, fillH, vColor);
        
        // Channel label
        char ch[4];
        snprintf(ch, sizeof(ch), "CH%d", i + 1);
        tft.setTextColor(TFT_DARKGREY);
        tft.setTextDatum(TC_DATUM);
        tft.drawString(ch, vBarX + vBarW/2, vBarY + vBarH + 2);
    }
    
    // Status
    tft.setTextColor(TFT_DARKGREY);
    tft.setTextDatum(BC_DATUM);
    tft.drawString("Real-time gauge updates", 80, 118);
    
    // Now sync the entire buffer to FPGA in one go
    tft.endBuffered();
    tft.syncBuffer();
}
