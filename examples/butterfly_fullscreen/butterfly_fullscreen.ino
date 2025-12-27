/**
 * @file butterfly_fullscreen.ino
 * @brief Display a butterfly image fullscreen on HQVGA 160x120 framebuffer
 * 
 * Hardware: Papilio Arcade board with HDMI output
 * Display: 160x120 scaled to 720p via FPGA
 */

#include <HQVGA.h>

// Include the converted image data
#include "butterfly_image.h"

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n=== Butterfly Fullscreen ===");
    
    // Initialize VGA
    VGA.begin();
    VGA.waitForFPGA();
    VGA.setVideoMode(2);  // Framebuffer mode
    
    // Draw the image
    Serial.println("Drawing image...");
    for (int y = 0; y < IMAGE_HEIGHT; y++) {
        for (int x = 0; x < IMAGE_WIDTH; x++) {
            VGA.putPixel(x, y, imageData[y][x]);
        }
    }
    
    Serial.println("Done!");
}

void loop() {
    // Nothing to do - static image
    delay(1000);
}
