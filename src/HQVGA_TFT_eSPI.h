/**
 * @file HQVGA_TFT_eSPI.h
 * @brief TFT_eSPI-compatible adapter for HQVGA 160x120 framebuffer
 * 
 * This adapter provides a TFT_eSPI-like API for the HQVGA framebuffer,
 * allowing easy porting of existing TFT_eSPI projects. It implements
 * the most commonly used drawing functions with RGB332 color support.
 * 
 * Usage:
 *   #include <HQVGA_TFT_eSPI.h>
 *   
 *   HQVGA_TFT tft;
 *   
 *   void setup() {
 *     tft.begin();
 *     tft.fillScreen(TFT_BLACK);
 *     tft.setTextColor(TFT_WHITE, TFT_BLACK);
 *     tft.drawString("Hello!", 10, 10, 2);
 *   }
 * 
 * Color format: Uses RGB565 input (16-bit) converted to RGB332 (8-bit)
 * for compatibility with existing TFT_eSPI code.
 */

#ifndef HQVGA_TFT_ESPI_H
#define HQVGA_TFT_ESPI_H

#include <Arduino.h>
#include "HQVGA.h"

// Convenience macros for display dimensions
#define HQVGA_WIDTH  VGA_HSIZE
#define HQVGA_HEIGHT VGA_VSIZE

// Local framebuffer for fast drawing operations
// This is synced to FPGA via Wishbone-over-SPI
#define HQVGA_FRAMEBUFFER_SIZE (HQVGA_WIDTH * HQVGA_HEIGHT)

// TFT_eSPI compatible color definitions (RGB565)
#define TFT_BLACK       0x0000
#define TFT_NAVY        0x000F
#define TFT_DARKGREEN   0x03E0
#define TFT_DARKCYAN    0x03EF
#define TFT_MAROON      0x7800
#define TFT_PURPLE      0x780F
#define TFT_OLIVE       0x7BE0
#define TFT_LIGHTGREY   0xC618
#define TFT_DARKGREY    0x7BEF
#define TFT_BLUE        0x001F
#define TFT_GREEN       0x07E0
#define TFT_CYAN        0x07FF
#define TFT_RED         0xF800
#define TFT_MAGENTA     0xF81F
#define TFT_YELLOW      0xFFE0
#define TFT_WHITE       0xFFFF
#define TFT_ORANGE      0xFDA0
#define TFT_GREENYELLOW 0xB7E0
#define TFT_PINK        0xFE19
#define TFT_BROWN       0x9A60
#define TFT_GOLD        0xFEA0
#define TFT_SILVER      0xC618
#define TFT_SKYBLUE     0x867D
#define TFT_VIOLET      0x915C

// Text datum positions (for setTextDatum)
#define TL_DATUM 0  // Top left
#define TC_DATUM 1  // Top center
#define TR_DATUM 2  // Top right
#define ML_DATUM 3  // Middle left
#define MC_DATUM 4  // Middle center (also CL_DATUM, CC_DATUM)
#define MR_DATUM 5  // Middle right
#define BL_DATUM 6  // Bottom left
#define BC_DATUM 7  // Bottom center
#define BR_DATUM 8  // Bottom right

// Font size constants
#define FONT_SIZE_1  1
#define FONT_SIZE_2  2
#define FONT_SIZE_4  4

class HQVGA_TFT {
public:
    // Local framebuffer for fast drawing (synced to FPGA)
    uint8_t frameBuffer[HQVGA_FRAMEBUFFER_SIZE];
    
    HQVGA_TFT() : _vga(nullptr), _ownsVga(true), _textColor(0xFF), _textBgColor(0x00),
                  _textSize(1), _textDatum(TL_DATUM), _cursorX(0), _cursorY(0),
                  _wrap(true), _buffered(false) {
        memset(frameBuffer, 0, HQVGA_FRAMEBUFFER_SIZE);
    }
    
    /**
     * @brief Initialize with existing VGA instance
     */
    HQVGA_TFT(VGA_class* vga) : _vga(vga), _ownsVga(false), _textColor(0xFF), 
                                 _textBgColor(0x00), _textSize(1), _textDatum(TL_DATUM),
                                 _cursorX(0), _cursorY(0), _wrap(true), _buffered(false) {
        memset(frameBuffer, 0, HQVGA_FRAMEBUFFER_SIZE);
    }
    
    ~HQVGA_TFT() {
        if (_ownsVga && _vga) {
            delete _vga;
        }
    }
    
    /**
     * @brief Initialize the display
     */
    void begin() {
        if (!_vga) {
            _vga = new VGA_class();
            _ownsVga = true;
        }
        _vga->begin();
        _vga->waitForFPGA();
    }
    
    /**
     * @brief Initialize with specific SPI pins
     */
    void begin(uint8_t csPin, uint8_t clk, uint8_t mosi, uint8_t miso) {
        if (!_vga) {
            _vga = new VGA_class();
            _ownsVga = true;
        }
        _vga->begin(nullptr, csPin, clk, mosi, miso);
        _vga->waitForFPGA();
    }
    
    /**
     * @brief Get display width
     */
    int16_t width() const { return VGA_HSIZE; }
    
    /**
     * @brief Get display height
     */
    int16_t height() const { return VGA_VSIZE; }
    
    /**
     * @brief Enable buffered mode (draw to local buffer, call syncBuffer() to update display)
     * Use this for animations to avoid flickering
     */
    void startBuffered() { _buffered = true; }
    
    /**
     * @brief Disable buffered mode (each draw call updates display immediately)
     */
    void endBuffered() { _buffered = false; }
    
    /**
     * @brief Check if buffered mode is enabled
     */
    bool isBuffered() const { return _buffered; }
    
    /**
     * @brief Sync the local framebuffer to the FPGA display
     * Call this after drawing when in buffered mode
     */
    void syncBuffer() {
        for (int y = 0; y < HQVGA_HEIGHT; y++) {
            for (int x = 0; x < HQVGA_WIDTH; x++) {
                _vga->putPixel(x, y, frameBuffer[y * HQVGA_WIDTH + x]);
            }
        }
    }
    
    /**
     * @brief Sync a rectangular region of the framebuffer to FPGA
     * More efficient than full sync for partial updates
     */
    void syncRegion(int16_t x, int16_t y, int16_t w, int16_t h) {
        if (x < 0) { w += x; x = 0; }
        if (y < 0) { h += y; y = 0; }
        if (x + w > HQVGA_WIDTH) w = HQVGA_WIDTH - x;
        if (y + h > HQVGA_HEIGHT) h = HQVGA_HEIGHT - y;
        if (w <= 0 || h <= 0) return;
        
        for (int16_t py = y; py < y + h; py++) {
            for (int16_t px = x; px < x + w; px++) {
                _vga->putPixel(px, py, frameBuffer[py * HQVGA_WIDTH + px]);
            }
        }
    }
    
    /**
     * @brief Convert RGB565 to RGB332
     */
    static uint8_t color565to332(uint16_t color565) {
        uint8_t r = (color565 >> 11) & 0x1F;  // 5 bits
        uint8_t g = (color565 >> 5) & 0x3F;   // 6 bits
        uint8_t b = color565 & 0x1F;          // 5 bits
        return ((r >> 2) << 5) | ((g >> 3) << 2) | (b >> 3);
    }
    
    /**
     * @brief Convert RGB components to RGB565
     */
    static uint16_t color565(uint8_t r, uint8_t g, uint8_t b) {
        return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
    }
    
    /**
     * @brief Convert RGB components to RGB332
     */
    static uint8_t color332(uint8_t r, uint8_t g, uint8_t b) {
        return ((r >> 5) << 5) | ((g >> 5) << 2) | (b >> 6);
    }
    
    // ===== Drawing primitives =====
    
    /**
     * @brief Draw a single pixel (updates local buffer, syncs to FPGA unless buffered)
     */
    void drawPixel(int16_t x, int16_t y, uint16_t color) {
        if (x >= 0 && x < HQVGA_WIDTH && y >= 0 && y < HQVGA_HEIGHT) {
            uint8_t c332 = color565to332(color);
            frameBuffer[y * HQVGA_WIDTH + x] = c332;
            if (!_buffered) {
                _vga->putPixel(x, y, c332);
            }
        }
    }
    
    /**
     * @brief Fill the entire screen with a color
     */
    void fillScreen(uint16_t color) {
        uint8_t c332 = color565to332(color);
        memset(frameBuffer, c332, HQVGA_FRAMEBUFFER_SIZE);
        if (!_buffered) {
            // Sync to FPGA
            for (int y = 0; y < HQVGA_HEIGHT; y++) {
                for (int x = 0; x < HQVGA_WIDTH; x++) {
                    _vga->putPixel(x, y, c332);
                }
            }
        }
    }
    
    /**
     * @brief Draw a horizontal line
     */
    void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
        if (y < 0 || y >= HQVGA_HEIGHT || w <= 0) return;
        if (x < 0) { w += x; x = 0; }
        if (x + w > HQVGA_WIDTH) w = HQVGA_WIDTH - x;
        if (w <= 0) return;
        
        uint8_t c332 = color565to332(color);
        uint8_t* ptr = &frameBuffer[y * HQVGA_WIDTH + x];
        memset(ptr, c332, w);
        if (!_buffered) {
            // Sync to FPGA
            for (int16_t i = 0; i < w; i++) {
                _vga->putPixel(x + i, y, c332);
            }
        }
    }
    
    /**
     * @brief Draw a vertical line
     */
    void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
        if (x < 0 || x >= HQVGA_WIDTH || h <= 0) return;
        if (y < 0) { h += y; y = 0; }
        if (y + h > HQVGA_HEIGHT) h = HQVGA_HEIGHT - y;
        if (h <= 0) return;
        
        uint8_t c332 = color565to332(color);
        uint8_t* ptr = &frameBuffer[y * HQVGA_WIDTH + x];
        for (int16_t i = 0; i < h; i++) {
            *ptr = c332;
            ptr += HQVGA_WIDTH;
        }
        if (!_buffered) {
            for (int16_t i = 0; i < h; i++) {
                _vga->putPixel(x, y + i, c332);
            }
        }
    }
    
    /**
     * @brief Draw a line between two points (Bresenham's algorithm)
     */
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
        // Optimize for horizontal and vertical lines
        if (y0 == y1) {
            if (x0 > x1) { int16_t t = x0; x0 = x1; x1 = t; }
            drawFastHLine(x0, y0, x1 - x0 + 1, color);
            return;
        }
        if (x0 == x1) {
            if (y0 > y1) { int16_t t = y0; y0 = y1; y1 = t; }
            drawFastVLine(x0, y0, y1 - y0 + 1, color);
            return;
        }
        
        // Bresenham's algorithm
        int16_t steep = abs(y1 - y0) > abs(x1 - x0);
        if (steep) {
            int16_t t;
            t = x0; x0 = y0; y0 = t;
            t = x1; x1 = y1; y1 = t;
        }
        if (x0 > x1) {
            int16_t t;
            t = x0; x0 = x1; x1 = t;
            t = y0; y0 = y1; y1 = t;
        }
        
        int16_t dx = x1 - x0;
        int16_t dy = abs(y1 - y0);
        int16_t err = dx / 2;
        int16_t ystep = (y0 < y1) ? 1 : -1;
        
        for (; x0 <= x1; x0++) {
            if (steep) {
                drawPixel(y0, x0, color);
            } else {
                drawPixel(x0, y0, color);
            }
            err -= dy;
            if (err < 0) {
                y0 += ystep;
                err += dx;
            }
        }
    }
    
    /**
     * @brief Draw a rectangle outline
     */
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
        drawFastHLine(x, y, w, color);
        drawFastHLine(x, y + h - 1, w, color);
        drawFastVLine(x, y, h, color);
        drawFastVLine(x + w - 1, y, h, color);
    }
    
    /**
     * @brief Draw a filled rectangle
     */
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
        if (w <= 0 || h <= 0) return;
        if (x < 0) { w += x; x = 0; }
        if (y < 0) { h += y; y = 0; }
        if (x + w > HQVGA_WIDTH) w = HQVGA_WIDTH - x;
        if (y + h > HQVGA_HEIGHT) h = HQVGA_HEIGHT - y;
        if (w <= 0 || h <= 0) return;
        
        uint8_t c332 = color565to332(color);
        for (int16_t j = 0; j < h; j++) {
            uint8_t* ptr = &frameBuffer[(y + j) * HQVGA_WIDTH + x];
            memset(ptr, c332, w);
        }
        if (!_buffered) {
            // Sync to FPGA
            for (int16_t j = 0; j < h; j++) {
                for (int16_t i = 0; i < w; i++) {
                    _vga->putPixel(x + i, y + j, c332);
                }
            }
        }
    }
    
    /**
     * @brief Draw a circle outline
     */
    void drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
        int16_t f = 1 - r;
        int16_t ddF_x = 1;
        int16_t ddF_y = -2 * r;
        int16_t x = 0;
        int16_t y = r;
        
        drawPixel(x0, y0 + r, color);
        drawPixel(x0, y0 - r, color);
        drawPixel(x0 + r, y0, color);
        drawPixel(x0 - r, y0, color);
        
        while (x < y) {
            if (f >= 0) {
                y--;
                ddF_y += 2;
                f += ddF_y;
            }
            x++;
            ddF_x += 2;
            f += ddF_x;
            
            drawPixel(x0 + x, y0 + y, color);
            drawPixel(x0 - x, y0 + y, color);
            drawPixel(x0 + x, y0 - y, color);
            drawPixel(x0 - x, y0 - y, color);
            drawPixel(x0 + y, y0 + x, color);
            drawPixel(x0 - y, y0 + x, color);
            drawPixel(x0 + y, y0 - x, color);
            drawPixel(x0 - y, y0 - x, color);
        }
    }
    
    /**
     * @brief Draw a filled circle
     */
    void fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
        drawFastVLine(x0, y0 - r, 2 * r + 1, color);
        fillCircleHelper(x0, y0, r, 3, 0, color);
    }
    
    /**
     * @brief Draw a rounded rectangle outline
     */
    void drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
        drawFastHLine(x + r, y, w - 2 * r, color);
        drawFastHLine(x + r, y + h - 1, w - 2 * r, color);
        drawFastVLine(x, y + r, h - 2 * r, color);
        drawFastVLine(x + w - 1, y + r, h - 2 * r, color);
        drawCircleHelper(x + r, y + r, r, 1, color);
        drawCircleHelper(x + w - r - 1, y + r, r, 2, color);
        drawCircleHelper(x + w - r - 1, y + h - r - 1, r, 4, color);
        drawCircleHelper(x + r, y + h - r - 1, r, 8, color);
    }
    
    /**
     * @brief Draw a filled rounded rectangle
     */
    void fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
        fillRect(x + r, y, w - 2 * r, h, color);
        fillCircleHelper(x + w - r - 1, y + r, r, 1, h - 2 * r - 1, color);
        fillCircleHelper(x + r, y + r, r, 2, h - 2 * r - 1, color);
    }
    
    /**
     * @brief Draw a triangle outline
     */
    void drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, 
                      int16_t x2, int16_t y2, uint16_t color) {
        drawLine(x0, y0, x1, y1, color);
        drawLine(x1, y1, x2, y2, color);
        drawLine(x2, y2, x0, y0, color);
    }
    
    /**
     * @brief Draw a filled triangle
     */
    void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                      int16_t x2, int16_t y2, uint16_t color) {
        int16_t a, b, y, last;
        
        // Sort coordinates by Y order (y2 >= y1 >= y0)
        if (y0 > y1) { int16_t t = y0; y0 = y1; y1 = t; t = x0; x0 = x1; x1 = t; }
        if (y1 > y2) { int16_t t = y1; y1 = y2; y2 = t; t = x1; x1 = x2; x2 = t; }
        if (y0 > y1) { int16_t t = y0; y0 = y1; y1 = t; t = x0; x0 = x1; x1 = t; }
        
        if (y0 == y2) {
            a = b = x0;
            if (x1 < a) a = x1;
            else if (x1 > b) b = x1;
            if (x2 < a) a = x2;
            else if (x2 > b) b = x2;
            drawFastHLine(a, y0, b - a + 1, color);
            return;
        }
        
        int16_t dx01 = x1 - x0, dy01 = y1 - y0;
        int16_t dx02 = x2 - x0, dy02 = y2 - y0;
        int16_t dx12 = x2 - x1, dy12 = y2 - y1;
        int32_t sa = 0, sb = 0;
        
        if (y1 == y2) last = y1;
        else last = y1 - 1;
        
        for (y = y0; y <= last; y++) {
            a = x0 + sa / dy01;
            b = x0 + sb / dy02;
            sa += dx01;
            sb += dx02;
            if (a > b) { int16_t t = a; a = b; b = t; }
            drawFastHLine(a, y, b - a + 1, color);
        }
        
        sa = dx12 * (y - y1);
        sb = dx02 * (y - y0);
        for (; y <= y2; y++) {
            a = x1 + sa / dy12;
            b = x0 + sb / dy02;
            sa += dx12;
            sb += dx02;
            if (a > b) { int16_t t = a; a = b; b = t; }
            drawFastHLine(a, y, b - a + 1, color);
        }
    }
    
    // ===== Text functions =====
    
    /**
     * @brief Set text color (with transparent background)
     */
    void setTextColor(uint16_t color) {
        _textColor = color565to332(color);
        _textBgColor = _textColor;  // Same = transparent
    }
    
    /**
     * @brief Set text color with background
     */
    void setTextColor(uint16_t color, uint16_t bg) {
        _textColor = color565to332(color);
        _textBgColor = color565to332(bg);
    }
    
    /**
     * @brief Set text size multiplier
     */
    void setTextSize(uint8_t size) {
        _textSize = (size > 0) ? size : 1;
    }
    
    /**
     * @brief Set text datum (alignment point)
     */
    void setTextDatum(uint8_t datum) {
        _textDatum = datum;
    }
    
    /**
     * @brief Set cursor position
     */
    void setCursor(int16_t x, int16_t y) {
        _cursorX = x;
        _cursorY = y;
    }
    
    /**
     * @brief Set text wrapping
     */
    void setTextWrap(bool wrap) {
        _wrap = wrap;
    }
    
    /**
     * @brief Get text width in pixels
     */
    int16_t textWidth(const char* str, uint8_t font = 1) {
        (void)font;
        return strlen(str) * 6 * _textSize;
    }
    
    /**
     * @brief Get font height in pixels
     */
    int16_t fontHeight(uint8_t font = 1) {
        (void)font;
        return 8 * _textSize;
    }
    
    /**
     * @brief Draw a string at specified position
     * @return Width of the string in pixels
     */
    int16_t drawString(const char* str, int16_t x, int16_t y, uint8_t font = 1) {
        (void)font;
        
        int16_t strWidth = textWidth(str);
        int16_t strHeight = fontHeight();
        
        // Apply text datum alignment
        switch (_textDatum) {
            case TC_DATUM: x -= strWidth / 2; break;
            case TR_DATUM: x -= strWidth; break;
            case ML_DATUM: y -= strHeight / 2; break;
            case MC_DATUM: x -= strWidth / 2; y -= strHeight / 2; break;
            case MR_DATUM: x -= strWidth; y -= strHeight / 2; break;
            case BL_DATUM: y -= strHeight; break;
            case BC_DATUM: x -= strWidth / 2; y -= strHeight; break;
            case BR_DATUM: x -= strWidth; y -= strHeight; break;
            default: break;  // TL_DATUM
        }
        
        _cursorX = x;
        _cursorY = y;
        
        while (*str) {
            drawChar(*str++);
        }
        
        return strWidth;
    }
    
    /**
     * @brief Draw a string (String object version)
     */
    int16_t drawString(const String& str, int16_t x, int16_t y, uint8_t font = 1) {
        return drawString(str.c_str(), x, y, font);
    }
    
    /**
     * @brief Draw a number
     */
    int16_t drawNumber(long num, int16_t x, int16_t y, uint8_t font = 1) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%ld", num);
        return drawString(buf, x, y, font);
    }
    
    /**
     * @brief Draw a float
     */
    int16_t drawFloat(float num, uint8_t dp, int16_t x, int16_t y, uint8_t font = 1) {
        char buf[32];
        snprintf(buf, sizeof(buf), "%.*f", dp, num);
        return drawString(buf, x, y, font);
    }
    
    /**
     * @brief Draw centered string
     */
    int16_t drawCentreString(const char* str, int16_t x, int16_t y, uint8_t font = 1) {
        uint8_t oldDatum = _textDatum;
        _textDatum = TC_DATUM;
        int16_t w = drawString(str, x, y, font);
        _textDatum = oldDatum;
        return w;
    }
    
    /**
     * @brief Draw right-aligned string
     */
    int16_t drawRightString(const char* str, int16_t x, int16_t y, uint8_t font = 1) {
        uint8_t oldDatum = _textDatum;
        _textDatum = TR_DATUM;
        int16_t w = drawString(str, x, y, font);
        _textDatum = oldDatum;
        return w;
    }
    
    /**
     * @brief Print character at cursor position
     */
    void print(char c) {
        drawChar(c);
    }
    
    /**
     * @brief Print string at cursor position
     */
    void print(const char* str) {
        while (*str) {
            drawChar(*str++);
        }
    }
    
    /**
     * @brief Print string with newline
     */
    void println(const char* str = "") {
        print(str);
        _cursorX = 0;
        _cursorY += 8 * _textSize;
    }
    
    /**
     * @brief Print number
     */
    void print(int num) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%d", num);
        print(buf);
    }
    
    /**
     * @brief Print number with newline
     */
    void println(int num) {
        print(num);
        _cursorX = 0;
        _cursorY += 8 * _textSize;
    }
    
    // ===== Sprite/Image functions =====
    
    /**
     * @brief Push a rectangular area of RGB565 pixels
     */
    void pushImage(int16_t x, int16_t y, int16_t w, int16_t h, const uint16_t* data) {
        for (int16_t j = 0; j < h; j++) {
            for (int16_t i = 0; i < w; i++) {
                drawPixel(x + i, y + j, data[j * w + i]);
            }
        }
    }
    
    /**
     * @brief Push a rectangular area of RGB332 pixels (native format)
     */
    void pushImage332(int16_t x, int16_t y, int16_t w, int16_t h, const uint8_t* data) {
        for (int16_t j = 0; j < h; j++) {
            for (int16_t i = 0; i < w; i++) {
                int16_t px = x + i;
                int16_t py = y + j;
                if (px >= 0 && px < HQVGA_WIDTH && py >= 0 && py < HQVGA_HEIGHT) {
                    uint8_t c = data[j * w + i];
                    frameBuffer[py * HQVGA_WIDTH + px] = c;
                    if (!_buffered) {
                        _vga->putPixel(px, py, c);
                    }
                }
            }
        }
    }
    
    /**
     * @brief Read a pixel color from local framebuffer
     */
    uint16_t readPixel(int16_t x, int16_t y) {
        if (x < 0 || x >= HQVGA_WIDTH || y < 0 || y >= HQVGA_HEIGHT) return 0;
        uint8_t c332 = frameBuffer[y * HQVGA_WIDTH + x];
        // Convert RGB332 to RGB565
        uint8_t r = (c332 >> 5) & 0x07;
        uint8_t g = (c332 >> 2) & 0x07;
        uint8_t b = c332 & 0x03;
        return (r << 13) | (g << 8) | (b << 3);
    }
    
    /**
     * @brief Get access to the underlying VGA class
     */
    VGA_class* getVGA() { return _vga; }
    
    /**
     * @brief Get direct access to local framebuffer
     */
    uint8_t* getFrameBuffer() { return frameBuffer; }

private:
    VGA_class* _vga;
    bool _ownsVga;
    uint8_t _textColor;
    uint8_t _textBgColor;
    uint8_t _textSize;
    uint8_t _textDatum;
    int16_t _cursorX;
    int16_t _cursorY;
    bool _wrap;
    bool _buffered;  // When true, drawing only updates local buffer; call syncBuffer() to update display
    
    // Simple 5x7 font data (ASCII 32-127)
    static const uint8_t font5x7[];
    
    void drawChar(char c) {
        if (c < 32 || c > 127) c = '?';
        
        uint8_t charWidth = 5 * _textSize;
        uint8_t charHeight = 7 * _textSize;
        
        // Draw background if different from foreground
        if (_textBgColor != _textColor) {
            for (int16_t j = 0; j < 8 * _textSize; j++) {
                for (int16_t i = 0; i < 6 * _textSize; i++) {
                    int16_t px = _cursorX + i;
                    int16_t py = _cursorY + j;
                    if (px >= 0 && px < HQVGA_WIDTH && py >= 0 && py < HQVGA_HEIGHT) {
                        frameBuffer[py * HQVGA_WIDTH + px] = _textBgColor;
                        if (!_buffered) {
                            _vga->putPixel(px, py, _textBgColor);
                        }
                    }
                }
            }
        }
        
        // Draw character
        const uint8_t* charData = &font5x7[(c - 32) * 5];
        for (int8_t col = 0; col < 5; col++) {
            uint8_t line = charData[col];
            for (int8_t row = 0; row < 7; row++) {
                if (line & (1 << row)) {
                    if (_textSize == 1) {
                        int16_t px = _cursorX + col;
                        int16_t py = _cursorY + row;
                        if (px >= 0 && px < HQVGA_WIDTH && py >= 0 && py < HQVGA_HEIGHT) {
                            frameBuffer[py * HQVGA_WIDTH + px] = _textColor;
                            if (!_buffered) {
                                _vga->putPixel(px, py, _textColor);
                            }
                        }
                    } else {
                        for (uint8_t sy = 0; sy < _textSize; sy++) {
                            for (uint8_t sx = 0; sx < _textSize; sx++) {
                                int16_t px = _cursorX + col * _textSize + sx;
                                int16_t py = _cursorY + row * _textSize + sy;
                                if (px >= 0 && px < HQVGA_WIDTH && py >= 0 && py < HQVGA_HEIGHT) {
                                    frameBuffer[py * HQVGA_WIDTH + px] = _textColor;
                                    if (!_buffered) {
                                        _vga->putPixel(px, py, _textColor);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        
        _cursorX += 6 * _textSize;
        
        // Handle wrapping
        if (_wrap && _cursorX > HQVGA_WIDTH - 6 * _textSize) {
            _cursorX = 0;
            _cursorY += 8 * _textSize;
        }
    }
    
    void drawCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t corner, uint16_t color) {
        int16_t f = 1 - r;
        int16_t ddF_x = 1;
        int16_t ddF_y = -2 * r;
        int16_t x = 0;
        int16_t y = r;
        
        while (x < y) {
            if (f >= 0) {
                y--;
                ddF_y += 2;
                f += ddF_y;
            }
            x++;
            ddF_x += 2;
            f += ddF_x;
            
            if (corner & 0x4) {
                drawPixel(x0 + x, y0 + y, color);
                drawPixel(x0 + y, y0 + x, color);
            }
            if (corner & 0x2) {
                drawPixel(x0 + x, y0 - y, color);
                drawPixel(x0 + y, y0 - x, color);
            }
            if (corner & 0x8) {
                drawPixel(x0 - y, y0 + x, color);
                drawPixel(x0 - x, y0 + y, color);
            }
            if (corner & 0x1) {
                drawPixel(x0 - y, y0 - x, color);
                drawPixel(x0 - x, y0 - y, color);
            }
        }
    }
    
    void fillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t corners, 
                          int16_t delta, uint16_t color) {
        int16_t f = 1 - r;
        int16_t ddF_x = 1;
        int16_t ddF_y = -2 * r;
        int16_t x = 0;
        int16_t y = r;
        int16_t px = x;
        int16_t py = y;
        
        delta++;
        
        while (x < y) {
            if (f >= 0) {
                y--;
                ddF_y += 2;
                f += ddF_y;
            }
            x++;
            ddF_x += 2;
            f += ddF_x;
            
            if (x < (y + 1)) {
                if (corners & 1) drawFastVLine(x0 + x, y0 - y, 2 * y + delta, color);
                if (corners & 2) drawFastVLine(x0 - x, y0 - y, 2 * y + delta, color);
            }
            if (y != py) {
                if (corners & 1) drawFastVLine(x0 + py, y0 - px, 2 * px + delta, color);
                if (corners & 2) drawFastVLine(x0 - py, y0 - px, 2 * px + delta, color);
                py = y;
            }
            px = x;
        }
    }
};

// 5x7 font data - standard ASCII characters 32-127
const uint8_t HQVGA_TFT::font5x7[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, // Space
    0x00, 0x00, 0x5F, 0x00, 0x00, // !
    0x00, 0x07, 0x00, 0x07, 0x00, // "
    0x14, 0x7F, 0x14, 0x7F, 0x14, // #
    0x24, 0x2A, 0x7F, 0x2A, 0x12, // $
    0x23, 0x13, 0x08, 0x64, 0x62, // %
    0x36, 0x49, 0x55, 0x22, 0x50, // &
    0x00, 0x05, 0x03, 0x00, 0x00, // '
    0x00, 0x1C, 0x22, 0x41, 0x00, // (
    0x00, 0x41, 0x22, 0x1C, 0x00, // )
    0x08, 0x2A, 0x1C, 0x2A, 0x08, // *
    0x08, 0x08, 0x3E, 0x08, 0x08, // +
    0x00, 0x50, 0x30, 0x00, 0x00, // ,
    0x08, 0x08, 0x08, 0x08, 0x08, // -
    0x00, 0x60, 0x60, 0x00, 0x00, // .
    0x20, 0x10, 0x08, 0x04, 0x02, // /
    0x3E, 0x51, 0x49, 0x45, 0x3E, // 0
    0x00, 0x42, 0x7F, 0x40, 0x00, // 1
    0x42, 0x61, 0x51, 0x49, 0x46, // 2
    0x21, 0x41, 0x45, 0x4B, 0x31, // 3
    0x18, 0x14, 0x12, 0x7F, 0x10, // 4
    0x27, 0x45, 0x45, 0x45, 0x39, // 5
    0x3C, 0x4A, 0x49, 0x49, 0x30, // 6
    0x01, 0x71, 0x09, 0x05, 0x03, // 7
    0x36, 0x49, 0x49, 0x49, 0x36, // 8
    0x06, 0x49, 0x49, 0x29, 0x1E, // 9
    0x00, 0x36, 0x36, 0x00, 0x00, // :
    0x00, 0x56, 0x36, 0x00, 0x00, // ;
    0x00, 0x08, 0x14, 0x22, 0x41, // <
    0x14, 0x14, 0x14, 0x14, 0x14, // =
    0x41, 0x22, 0x14, 0x08, 0x00, // >
    0x02, 0x01, 0x51, 0x09, 0x06, // ?
    0x32, 0x49, 0x79, 0x41, 0x3E, // @
    0x7E, 0x11, 0x11, 0x11, 0x7E, // A
    0x7F, 0x49, 0x49, 0x49, 0x36, // B
    0x3E, 0x41, 0x41, 0x41, 0x22, // C
    0x7F, 0x41, 0x41, 0x22, 0x1C, // D
    0x7F, 0x49, 0x49, 0x49, 0x41, // E
    0x7F, 0x09, 0x09, 0x01, 0x01, // F
    0x3E, 0x41, 0x41, 0x51, 0x32, // G
    0x7F, 0x08, 0x08, 0x08, 0x7F, // H
    0x00, 0x41, 0x7F, 0x41, 0x00, // I
    0x20, 0x40, 0x41, 0x3F, 0x01, // J
    0x7F, 0x08, 0x14, 0x22, 0x41, // K
    0x7F, 0x40, 0x40, 0x40, 0x40, // L
    0x7F, 0x02, 0x04, 0x02, 0x7F, // M
    0x7F, 0x04, 0x08, 0x10, 0x7F, // N
    0x3E, 0x41, 0x41, 0x41, 0x3E, // O
    0x7F, 0x09, 0x09, 0x09, 0x06, // P
    0x3E, 0x41, 0x51, 0x21, 0x5E, // Q
    0x7F, 0x09, 0x19, 0x29, 0x46, // R
    0x46, 0x49, 0x49, 0x49, 0x31, // S
    0x01, 0x01, 0x7F, 0x01, 0x01, // T
    0x3F, 0x40, 0x40, 0x40, 0x3F, // U
    0x1F, 0x20, 0x40, 0x20, 0x1F, // V
    0x7F, 0x20, 0x18, 0x20, 0x7F, // W
    0x63, 0x14, 0x08, 0x14, 0x63, // X
    0x03, 0x04, 0x78, 0x04, 0x03, // Y
    0x61, 0x51, 0x49, 0x45, 0x43, // Z
    0x00, 0x00, 0x7F, 0x41, 0x41, // [
    0x02, 0x04, 0x08, 0x10, 0x20, // backslash
    0x41, 0x41, 0x7F, 0x00, 0x00, // ]
    0x04, 0x02, 0x01, 0x02, 0x04, // ^
    0x40, 0x40, 0x40, 0x40, 0x40, // _
    0x00, 0x01, 0x02, 0x04, 0x00, // `
    0x20, 0x54, 0x54, 0x54, 0x78, // a
    0x7F, 0x48, 0x44, 0x44, 0x38, // b
    0x38, 0x44, 0x44, 0x44, 0x20, // c
    0x38, 0x44, 0x44, 0x48, 0x7F, // d
    0x38, 0x54, 0x54, 0x54, 0x18, // e
    0x08, 0x7E, 0x09, 0x01, 0x02, // f
    0x08, 0x14, 0x54, 0x54, 0x3C, // g
    0x7F, 0x08, 0x04, 0x04, 0x78, // h
    0x00, 0x44, 0x7D, 0x40, 0x00, // i
    0x20, 0x40, 0x44, 0x3D, 0x00, // j
    0x00, 0x7F, 0x10, 0x28, 0x44, // k
    0x00, 0x41, 0x7F, 0x40, 0x00, // l
    0x7C, 0x04, 0x18, 0x04, 0x78, // m
    0x7C, 0x08, 0x04, 0x04, 0x78, // n
    0x38, 0x44, 0x44, 0x44, 0x38, // o
    0x7C, 0x14, 0x14, 0x14, 0x08, // p
    0x08, 0x14, 0x14, 0x18, 0x7C, // q
    0x7C, 0x08, 0x04, 0x04, 0x08, // r
    0x48, 0x54, 0x54, 0x54, 0x20, // s
    0x04, 0x3F, 0x44, 0x40, 0x20, // t
    0x3C, 0x40, 0x40, 0x20, 0x7C, // u
    0x1C, 0x20, 0x40, 0x20, 0x1C, // v
    0x3C, 0x40, 0x30, 0x40, 0x3C, // w
    0x44, 0x28, 0x10, 0x28, 0x44, // x
    0x0C, 0x50, 0x50, 0x50, 0x3C, // y
    0x44, 0x64, 0x54, 0x4C, 0x44, // z
    0x00, 0x08, 0x36, 0x41, 0x00, // {
    0x00, 0x00, 0x7F, 0x00, 0x00, // |
    0x00, 0x41, 0x36, 0x08, 0x00, // }
    0x08, 0x08, 0x2A, 0x1C, 0x08, // ~
    0x08, 0x1C, 0x2A, 0x08, 0x08  // DEL (arrow)
};

#endif // HQVGA_TFT_ESPI_H
