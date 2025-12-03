/**
 * @file HQVGA_ImageDec.h
 * @brief Image decoder adapters for HQVGA 160x120 framebuffer
 * 
 * Provides easy-to-use wrappers for bitbank2's image decoders:
 * - JPEGDEC: JPEG image decoding
 * - PNGdec: PNG image decoding  
 * - AnimatedGIF: GIF animation playback
 * 
 * All decoders output directly to the HQVGA framebuffer with automatic
 * RGB888/RGB565 to RGB332 color conversion and optional scaling.
 * 
 * Dependencies (add to platformio.ini lib_deps):
 *   bitbank2/JPEGDEC
 *   bitbank2/PNGdec
 *   bitbank2/AnimatedGIF
 * 
 * Hardware: Papilio Arcade board with HDMI output
 * Display: 160x120 scaled to 720p via FPGA
 */

#ifndef HQVGA_IMAGEDEC_H
#define HQVGA_IMAGEDEC_H

#include <Arduino.h>
#include "HQVGA.h"

// Display dimensions
#define HQVGA_IMG_WIDTH  160
#define HQVGA_IMG_HEIGHT 120

// Forward declarations - include the actual libraries in your sketch
#ifdef JPEGDEC_H
#define HQVGA_HAS_JPEG
#endif

#ifdef __PNGDEC__
#define HQVGA_HAS_PNG
#endif

#ifdef __AnimatedGIF__
#define HQVGA_HAS_GIF
#endif

/**
 * @brief Convert RGB888 to RGB332 (HQVGA native format)
 */
inline uint8_t rgb888to332(uint8_t r, uint8_t g, uint8_t b) {
    return ((r >> 5) << 5) | ((g >> 5) << 2) | (b >> 6);
}

/**
 * @brief Convert RGB565 to RGB332
 */
inline uint8_t rgb565to332(uint16_t color) {
    uint8_t r = (color >> 11) & 0x1F;  // 5 bits
    uint8_t g = (color >> 5) & 0x3F;   // 6 bits
    uint8_t b = color & 0x1F;          // 5 bits
    // Scale to RGB332: R(3), G(3), B(2)
    return ((r >> 2) << 5) | ((g >> 3) << 2) | (b >> 3);
}

// ============================================================================
// HQVGA Image Decoder Context
// ============================================================================

/**
 * @brief Shared context for image decoder callbacks
 */
struct HQVGA_ImageContext {
    VGA_class* vga;      // Pointer to VGA instance
    int16_t offsetX;     // X offset for drawing
    int16_t offsetY;     // Y offset for drawing
    uint8_t* buffer;     // Optional local buffer for buffered mode
    bool buffered;       // Use local buffer instead of direct write
    
    HQVGA_ImageContext(VGA_class* v = &VGA) : 
        vga(v), offsetX(0), offsetY(0), buffer(nullptr), buffered(false) {}
    
    void setOffset(int16_t x, int16_t y) { offsetX = x; offsetY = y; }
    void setBuffer(uint8_t* buf) { buffer = buf; buffered = (buf != nullptr); }
};

// Global context - set this before decoding
extern HQVGA_ImageContext hqvgaImageCtx;

// ============================================================================
// JPEG Decoder Callbacks
// ============================================================================

#ifdef JPEGDEC_H

/**
 * @brief JPEG draw callback for HQVGA framebuffer
 * 
 * Use with: jpeg.setDrawFunction(HQVGA_JPEGDraw);
 * 
 * Handles MCU blocks from JPEGDEC and writes pixels to framebuffer.
 * Supports 8-bit grayscale and 16-bit RGB565 output.
 */
inline int HQVGA_JPEGDraw(JPEGDRAW *pDraw) {
    int16_t x = pDraw->x + hqvgaImageCtx.offsetX;
    int16_t y = pDraw->y + hqvgaImageCtx.offsetY;
    int16_t w = pDraw->iWidth;
    int16_t h = pDraw->iHeight;
    uint16_t *pixels = pDraw->pPixels;
    
    for (int16_t row = 0; row < h; row++) {
        int16_t py = y + row;
        if (py < 0 || py >= HQVGA_IMG_HEIGHT) {
            pixels += w;
            continue;
        }
        
        for (int16_t col = 0; col < w; col++) {
            int16_t px = x + col;
            if (px >= 0 && px < HQVGA_IMG_WIDTH) {
                uint16_t color565 = *pixels;
                uint8_t color332 = rgb565to332(color565);
                
                if (hqvgaImageCtx.buffered && hqvgaImageCtx.buffer) {
                    hqvgaImageCtx.buffer[py * HQVGA_IMG_WIDTH + px] = color332;
                } else {
                    hqvgaImageCtx.vga->putPixel(px, py, color332);
                }
            }
            pixels++;
        }
    }
    return 1;  // Continue decoding
}

/**
 * @brief Helper class for JPEG decoding to HQVGA
 */
class HQVGA_JPEG {
public:
    JPEGDEC jpeg;
    
    HQVGA_JPEG(VGA_class* vga = &VGA) {
        hqvgaImageCtx.vga = vga;
    }
    
    /**
     * @brief Decode JPEG from memory buffer
     * @param data Pointer to JPEG data
     * @param size Size of JPEG data
     * @param x X offset for drawing (default: centered)
     * @param y Y offset for drawing (default: centered)
     * @param buffer Optional local buffer for buffered mode
     * @return true on success
     */
    bool decode(const uint8_t* data, size_t size, int16_t x = -1, int16_t y = -1, uint8_t* buffer = nullptr) {
        hqvgaImageCtx.setBuffer(buffer);
        
        if (jpeg.openRAM((uint8_t*)data, size, HQVGA_JPEGDraw)) {
            // Auto-center if coordinates are -1
            if (x < 0) x = (HQVGA_IMG_WIDTH - jpeg.getWidth()) / 2;
            if (y < 0) y = (HQVGA_IMG_HEIGHT - jpeg.getHeight()) / 2;
            
            hqvgaImageCtx.setOffset(x, y);
            jpeg.setPixelType(RGB565_LITTLE_ENDIAN);
            jpeg.decode(0, 0, 0);  // Decode full image
            jpeg.close();
            return true;
        }
        return false;
    }
    
    /**
     * @brief Decode JPEG with scaling (1/2, 1/4, or 1/8)
     */
    bool decodeScaled(const uint8_t* data, size_t size, int scale, int16_t x = -1, int16_t y = -1, uint8_t* buffer = nullptr) {
        hqvgaImageCtx.setBuffer(buffer);
        
        if (jpeg.openRAM((uint8_t*)data, size, HQVGA_JPEGDraw)) {
            int scaledW = jpeg.getWidth() / scale;
            int scaledH = jpeg.getHeight() / scale;
            
            if (x < 0) x = (HQVGA_IMG_WIDTH - scaledW) / 2;
            if (y < 0) y = (HQVGA_IMG_HEIGHT - scaledH) / 2;
            
            hqvgaImageCtx.setOffset(x, y);
            jpeg.setPixelType(RGB565_LITTLE_ENDIAN);
            
            int options = 0;
            if (scale == 2) options = JPEG_SCALE_HALF;
            else if (scale == 4) options = JPEG_SCALE_QUARTER;
            else if (scale == 8) options = JPEG_SCALE_EIGHTH;
            
            jpeg.decode(0, 0, options);
            jpeg.close();
            return true;
        }
        return false;
    }
    
    int getWidth() { return jpeg.getWidth(); }
    int getHeight() { return jpeg.getHeight(); }
};

#endif // JPEGDEC_H

// ============================================================================
// PNG Decoder Callbacks  
// ============================================================================

#ifdef __PNGDEC__

/**
 * @brief PNG draw callback for HQVGA framebuffer
 * 
 * Use with: png.setDrawCallback(HQVGA_PNGDraw);
 */
inline int HQVGA_PNGDraw(PNGDRAW *pDraw) {
    int16_t x = hqvgaImageCtx.offsetX;
    int16_t y = pDraw->y + hqvgaImageCtx.offsetY;
    
    if (y < 0 || y >= HQVGA_IMG_HEIGHT) return 1;  // Skip but continue
    
    uint16_t *pixels = (uint16_t*)pDraw->pUser;  // RGB565 line buffer
    
    // Convert indexed/RGB to RGB565 line
    if (pDraw->pPalette) {
        // Indexed color
        for (int i = 0; i < pDraw->iWidth; i++) {
            uint8_t idx = pDraw->pPixels[i];
            pixels[i] = pDraw->pPalette[idx];
        }
    } else {
        // Direct RGB - already in pPixels as RGB565
        memcpy(pixels, pDraw->pPixels, pDraw->iWidth * 2);
    }
    
    // Write to framebuffer
    for (int16_t col = 0; col < pDraw->iWidth; col++) {
        int16_t px = x + col;
        if (px >= 0 && px < HQVGA_IMG_WIDTH) {
            uint8_t color332 = rgb565to332(pixels[col]);
            
            if (hqvgaImageCtx.buffered && hqvgaImageCtx.buffer) {
                hqvgaImageCtx.buffer[y * HQVGA_IMG_WIDTH + px] = color332;
            } else {
                hqvgaImageCtx.vga->putPixel(px, y, color332);
            }
        }
    }
    return 1;  // Continue decoding
}

/**
 * @brief Helper class for PNG decoding to HQVGA
 */
class HQVGA_PNG {
public:
    PNG png;
    uint8_t lineBuffer[HQVGA_IMG_WIDTH * 4];  // Line buffer for RGBA decoding
    
    HQVGA_PNG(VGA_class* vga = &VGA) {
        hqvgaImageCtx.vga = vga;
    }
    
    /**
     * @brief Decode PNG from memory buffer
     */
    bool decode(const uint8_t* data, size_t size, int16_t x = -1, int16_t y = -1, uint8_t* buffer = nullptr) {
        hqvgaImageCtx.setBuffer(buffer);
        
        int rc = png.openRAM((uint8_t*)data, size, HQVGA_PNGDraw);
        if (rc == PNG_SUCCESS) {
            // Auto-center if coordinates are -1
            if (x < 0) x = (HQVGA_IMG_WIDTH - png.getWidth()) / 2;
            if (y < 0) y = (HQVGA_IMG_HEIGHT - png.getHeight()) / 2;
            
            hqvgaImageCtx.setOffset(x, y);
            png.setBuffer((uint8_t*)lineBuffer);
            png.decode((void*)lineBuffer, 0);
            png.close();
            return true;
        }
        return false;
    }
    
    int getWidth() { return png.getWidth(); }
    int getHeight() { return png.getHeight(); }
};

#endif // __PNGDEC__

// ============================================================================
// GIF Decoder Callbacks
// ============================================================================

#ifdef __AnimatedGIF__

/**
 * @brief GIF draw callback for HQVGA framebuffer
 * 
 * Use with: gif.begin(GIF_PALETTE_RGB565_LE);
 *           and set pUser to point to HQVGA_ImageContext
 */
inline void HQVGA_GIFDraw(GIFDRAW *pDraw) {
    int16_t x = hqvgaImageCtx.offsetX;
    int16_t y = pDraw->y + hqvgaImageCtx.offsetY + pDraw->iY;
    
    if (y < 0 || y >= HQVGA_IMG_HEIGHT) return;
    if (pDraw->iY + pDraw->iHeight > HQVGA_IMG_HEIGHT) return;
    
    uint8_t *s = pDraw->pPixels;
    uint16_t *palette = pDraw->pPalette;
    
    // Handle transparency
    uint8_t ucTransparent = pDraw->ucTransparent;
    bool hasTransparency = pDraw->ucHasTransparency;
    
    for (int16_t col = 0; col < pDraw->iWidth; col++) {
        int16_t px = x + pDraw->iX + col;
        
        if (px >= 0 && px < HQVGA_IMG_WIDTH) {
            uint8_t idx = s[col];
            
            // Skip transparent pixels
            if (hasTransparency && idx == ucTransparent) continue;
            
            uint16_t color565 = palette[idx];
            uint8_t color332 = rgb565to332(color565);
            
            if (hqvgaImageCtx.buffered && hqvgaImageCtx.buffer) {
                hqvgaImageCtx.buffer[y * HQVGA_IMG_WIDTH + px] = color332;
            } else {
                hqvgaImageCtx.vga->putPixel(px, y, color332);
            }
        }
    }
}

/**
 * @brief Helper class for GIF playback to HQVGA
 */
class HQVGA_GIF {
public:
    AnimatedGIF gif;
    bool playing;
    unsigned long lastFrameTime;
    int frameDelay;
    
    HQVGA_GIF(VGA_class* vga = &VGA) : playing(false), lastFrameTime(0), frameDelay(0) {
        hqvgaImageCtx.vga = vga;
    }
    
    /**
     * @brief Open GIF from memory buffer
     */
    bool open(const uint8_t* data, size_t size, int16_t x = -1, int16_t y = -1, uint8_t* buffer = nullptr) {
        hqvgaImageCtx.setBuffer(buffer);
        
        gif.begin(GIF_PALETTE_RGB565_LE);
        
        if (gif.open((uint8_t*)data, size, HQVGA_GIFDraw)) {
            // Auto-center if coordinates are -1
            if (x < 0) x = (HQVGA_IMG_WIDTH - gif.getCanvasWidth()) / 2;
            if (y < 0) y = (HQVGA_IMG_HEIGHT - gif.getCanvasHeight()) / 2;
            
            hqvgaImageCtx.setOffset(x, y);
            playing = true;
            lastFrameTime = millis();
            return true;
        }
        return false;
    }
    
    /**
     * @brief Play next frame (call in loop)
     * @return true if frame was drawn, false if waiting or ended
     */
    bool playFrame() {
        if (!playing) return false;
        
        unsigned long now = millis();
        if (now - lastFrameTime < (unsigned long)frameDelay) {
            return false;  // Not time for next frame yet
        }
        
        int result = gif.playFrame(false, &frameDelay);
        lastFrameTime = now;
        
        if (result == 0) {
            // Animation ended - loop
            gif.reset();
        }
        
        return true;
    }
    
    /**
     * @brief Play single frame (static GIF or single shot)
     */
    bool playSingleFrame() {
        if (!playing) return false;
        gif.playFrame(true, nullptr);
        return true;
    }
    
    void close() {
        gif.close();
        playing = false;
    }
    
    void reset() { gif.reset(); }
    bool isPlaying() { return playing; }
    int getWidth() { return gif.getCanvasWidth(); }
    int getHeight() { return gif.getCanvasHeight(); }
    int getFrameCount() { return gif.getFrameCount(); }
    int getLoopCount() { return gif.getLoopCount(); }
};

#endif // __AnimatedGIF__

// ============================================================================
// Global Context Instance
// ============================================================================

// Define in one .cpp file or use inline
#ifndef HQVGA_IMAGEDEC_IMPL
extern HQVGA_ImageContext hqvgaImageCtx;
#else
HQVGA_ImageContext hqvgaImageCtx;
#endif

#endif // HQVGA_IMAGEDEC_H
