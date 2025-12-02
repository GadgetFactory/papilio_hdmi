/*
  VGALiquidCrystal - LiquidCrystal-compatible library for HQVGA display
  
  This library provides a LiquidCrystal-compatible API for displaying
  text on the HQVGA framebuffer display. It emulates an LCD character
  display with scrolling support.
  
  Based on the original ZPUino VGALiquidCrystal by Alvaro Lopes
  Ported to ESP32-S3/HQVGA by Jack Gassett, Gadget Factory
  
  Hardware:
  - Papilio Arcade board with ESP32-S3 and FPGA
  - HDMI output (160x120 scaled to 720p)
*/

#ifndef VGALiquidCrystal_h
#define VGALiquidCrystal_h

#include <HQVGA.h>
#include "Print.h"
#include <inttypes.h>

#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

class VGALiquidCrystal : public Print {
public:
  // Constructor - pin parameters are ignored (for API compatibility)
  VGALiquidCrystal(uint8_t rs, uint8_t enable,
                   uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
                   uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7);
  VGALiquidCrystal(uint8_t rs, uint8_t rw, uint8_t enable,
                   uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
                   uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7);
  VGALiquidCrystal(uint8_t rs, uint8_t rw, uint8_t enable,
                   uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3);
  VGALiquidCrystal(uint8_t rs, uint8_t enable,
                   uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3);
  
  // Default constructor for simple usage
  VGALiquidCrystal();

  void init(uint8_t fourbitmode, uint8_t rs, uint8_t rw, uint8_t enable,
            uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
            uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7);
    
  void begin(uint8_t cols, uint8_t rows, uint8_t charsize = LCD_5x8DOTS);

  void clear();
  void home();

  void noDisplay();
  void display();
  void noBlink();
  void blink();
  void noCursor();
  void cursor();
  void scrollDisplayLeft();
  void scrollDisplayRight();
  void leftToRight();
  void rightToLeft();
  void autoscroll();
  void noAutoscroll();

  void createChar(uint8_t location, uint8_t charmap[]);
  void setCursor(uint8_t col, uint8_t row); 
  virtual size_t write(uint8_t);
  void command(uint8_t);

  void updateDisplay();
  
  // Set display position on screen
  void setPosition(int x, int y);
  
  // Set colors (RGB332 format)
  void setTextColor(uint8_t color);
  void setBackgroundColor(uint8_t color);
  
  // Draw border around LCD display
  void drawBorder(uint8_t color, uint8_t thickness = 2);

private:
  struct DisplayChar {
    unsigned char chr;
    uint8_t addr;
  };

  DisplayChar currentDisplayChars[32];

  void setCurrentDisplayChars();
  void initCurrentDisplayChars();
  void blankDisplay();
  void setDDRAddress(unsigned int);
  void updateCharAt(int);
  void putcharat(uint8_t chr, int pos, bool reverse = false);

  unsigned char ddram[104];
  unsigned char cgram[64];
  int shiftChars;
  unsigned int AC;
  unsigned _cols;
  unsigned _lines;
  int increment;
  int displayRange;
  int _x0, _y0;

  bool display_on, blink_on, cursor_on, scroll_on;
  bool need_update;
  
  uint8_t _textColor;
  uint8_t _bgColor;

  static unsigned char chrtbl[2048];
};

#endif
