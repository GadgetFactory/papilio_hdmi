/*
 * HQVGA Library for ESP32-S3
 * 
 * Original HQVGA VGA controller by Alvaro Lopes <alvieboy@alvie.com>
 * ESP32-S3 port by Jack Gassett, Gadget Factory
 * 
 * Ported from ZPUINO direct memory access to Wishbone-over-SPI for ESP32-S3
 */

#ifndef __HQVGA_H__
#define __HQVGA_H__

#include <Arduino.h>
#include <SPI.h>

// HQVGA resolution: 160x120 pixels (scaled 5x5 to 800x600@72Hz)
const unsigned int VGA_HSIZE = 160;
const unsigned int VGA_VSIZE = 120;

#define BYTES_PER_PIXEL 1
#define COLOR_WEIGHT_R 3
#define COLOR_WEIGHT_G 3
#define COLOR_WEIGHT_B 2

#define COLOR_SHIFT_R (COLOR_WEIGHT_B+COLOR_WEIGHT_G)
#define COLOR_SHIFT_G (COLOR_WEIGHT_B)
#define COLOR_SHIFT_B 0

// Wishbone base address for HQVGA (slave 3)
// New address map: HQVGA at 0x0000-0x7FFF, no base offset needed
#define HQVGA_WISHBONE_BASE 0x00

class VGA_class {
public:
	typedef unsigned char pixel_t;
	
	VGA_class();
	~VGA_class();

	inline unsigned int getHSize() const { return VGA_HSIZE; }
	inline unsigned int getVSize() const { return VGA_VSIZE; }

	// Initialize with SPI interface and Wishbone base address
	void begin(SPIClass* spi = nullptr, uint8_t csPin = 10, 
	          uint8_t spiClk = 12, uint8_t spiMosi = 11, uint8_t spiMiso = 9,
	          uint8_t wishboneBase = HQVGA_WISHBONE_BASE);

	// Video mode control (0=TestPattern, 1=Text, 2=Framebuffer)
	void setVideoMode(uint8_t mode);
	uint8_t getVideoMode();

	// Color management
	void setColor(pixel_t color) { fg = color; }
	void setBackgroundColor(pixel_t color) { bg = color; }
	inline void setColor(unsigned r, unsigned g, unsigned b) {
		setColor((r << COLOR_SHIFT_R) | (g << COLOR_SHIFT_G) | b);
	}

	// Pixel operations
	void putPixel(int x, int y);
	void putPixel(int x, int y, pixel_t color);
	pixel_t getPixel(int x, int y);

	// Drawing primitives
	void clear();
	void drawRect(unsigned x, unsigned y, unsigned width, unsigned height);
	void clearArea(unsigned x, unsigned y, unsigned width, unsigned height);
	void drawLine(int x0, int y0, int x1, int y1);

	// Text rendering
	void printchar(unsigned int x, unsigned int y, unsigned char c, bool trans = false);
	void printtext(unsigned x, unsigned y, const char *text, bool trans = false);

	// Area operations
	void readArea(int x, int y, int width, int height, pixel_t *dest);
	void writeArea(int x, int y, int width, int height, pixel_t *source);
	void moveArea(unsigned x, unsigned y, unsigned width, unsigned height, unsigned tx, unsigned ty);

	// Stream operations
	void blitStreamInit(int x, int y, int w);
	void blitStreamAppend(unsigned char c);

 private:
	// Wishbone SPI interface
	void writeWishbone(uint16_t addr, uint8_t data);
	uint8_t readWishbone(uint16_t addr);
	
	// Internal offset calculation
	uint16_t getOffset(unsigned x, unsigned y) { return x + (y * VGA_HSIZE); }
	
	SPIClass* _spi;
	bool _ownSpi;
	uint8_t _cs;
	uint8_t _clk;
	uint8_t _mosi;
	uint8_t _miso;
	uint8_t _wbBase;
	
	pixel_t fg, bg;
	uint16_t blitOffset;
	int blitw, cblit;
};

const VGA_class::pixel_t RED = (((1<<COLOR_WEIGHT_R)-1) << COLOR_SHIFT_R);
const VGA_class::pixel_t GREEN = (((1<<COLOR_WEIGHT_G)-1) << COLOR_SHIFT_G);
const VGA_class::pixel_t BLUE = (((1<<COLOR_WEIGHT_B)-1) << COLOR_SHIFT_B);
const VGA_class::pixel_t YELLOW = (RED|GREEN);
const VGA_class::pixel_t PURPLE = (RED|BLUE);
const VGA_class::pixel_t CYAN = (GREEN|BLUE);
const VGA_class::pixel_t WHITE = (RED|GREEN|BLUE);
const VGA_class::pixel_t BLACK = 0;

extern VGA_class VGA;

#endif
