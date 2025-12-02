/*
  VGALiquidCrystal - LiquidCrystal-compatible library for HQVGA display
  
  Based on the original ZPUino VGALiquidCrystal by Alvaro Lopes
  Ported to ESP32-S3/HQVGA by Jack Gassett, Gadget Factory
*/

#include "VGALiquidCrystal.h"
#include <Arduino.h>

// Default constructor
VGALiquidCrystal::VGALiquidCrystal() {
  init(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

VGALiquidCrystal::VGALiquidCrystal(uint8_t rs, uint8_t rw, uint8_t enable,
                                   uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
                                   uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7) {
  init(0, rs, rw, enable, d0, d1, d2, d3, d4, d5, d6, d7);
}

VGALiquidCrystal::VGALiquidCrystal(uint8_t rs, uint8_t enable,
                                   uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
                                   uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7) {
  init(0, rs, 255, enable, d0, d1, d2, d3, d4, d5, d6, d7);
}

VGALiquidCrystal::VGALiquidCrystal(uint8_t rs, uint8_t rw, uint8_t enable,
                                   uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3) {
  init(1, rs, rw, enable, d0, d1, d2, d3, 0, 0, 0, 0);
}

VGALiquidCrystal::VGALiquidCrystal(uint8_t rs, uint8_t enable,
                                   uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3) {
  init(1, rs, 255, enable, d0, d1, d2, d3, 0, 0, 0, 0);
}

void VGALiquidCrystal::init(uint8_t fourbitmode, uint8_t rs, uint8_t rw, uint8_t enable,
                            uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
                            uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7) {
  // Pin parameters are ignored - this is for API compatibility
  _textColor = GREEN;
  _bgColor = 1 << 3;  // Dark blue
  _x0 = 10;
  _y0 = 10;
}

void VGALiquidCrystal::setDDRAddress(unsigned value) {
  AC = value;
  if (_lines == 0) {
    if (AC > 79) AC = 0;
  } else {
    if (AC > 39 && AC < 64) {
      AC = 64;
    }
    if (AC > 103) {
      AC = 0;
    }
  }
}

void VGALiquidCrystal::begin(uint8_t cols, uint8_t lines, uint8_t dotsize) {
  _lines = lines;
  _cols = cols;
  
  // Ensure video mode is framebuffer (VGA.begin() should have been called first)
  // VGA.setVideoMode(2) is called by VGA.begin() already
  
  if (_lines > 1)
    displayRange = 32;
  else
    displayRange = 16;

  blink_on = cursor_on = scroll_on = false;
  display_on = true;

  increment = 1;
  shiftChars = 0;
  AC = 0;

  initCurrentDisplayChars();
  clear();
  display();
  blankDisplay();
  need_update = true;
}

void VGALiquidCrystal::setPosition(int x, int y) {
  _x0 = x;
  _y0 = y;
  need_update = true;
}

void VGALiquidCrystal::setTextColor(uint8_t color) {
  _textColor = color;
  need_update = true;
}

void VGALiquidCrystal::setBackgroundColor(uint8_t color) {
  _bgColor = color;
  need_update = true;
}

void VGALiquidCrystal::drawBorder(uint8_t color, uint8_t thickness) {
  // Character cell dimensions (matches putcharat)
  const int charWidth = 6;
  const int charHeight = 9;
  
  // Calculate LCD dimensions based on columns and rows
  int rows = (_lines == 0) ? 1 : 2;
  int lcdWidth = _cols * charWidth;
  int lcdHeight = rows * charHeight;
  
  // Draw border around the LCD area
  // Top border
  for (int t = 0; t < thickness; t++) {
    for (int x = _x0 - thickness; x < _x0 + lcdWidth + thickness; x++) {
      VGA.putPixel(x, _y0 - thickness + t, color);
    }
  }
  
  // Bottom border
  for (int t = 0; t < thickness; t++) {
    for (int x = _x0 - thickness; x < _x0 + lcdWidth + thickness; x++) {
      VGA.putPixel(x, _y0 + lcdHeight + t, color);
    }
  }
  
  // Left border
  for (int t = 0; t < thickness; t++) {
    for (int y = _y0; y < _y0 + lcdHeight; y++) {
      VGA.putPixel(_x0 - thickness + t, y, color);
    }
  }
  
  // Right border
  for (int t = 0; t < thickness; t++) {
    for (int y = _y0; y < _y0 + lcdHeight; y++) {
      VGA.putPixel(_x0 + lcdWidth + t, y, color);
    }
  }
}

void VGALiquidCrystal::clear() {
  for (unsigned i = 0; i < sizeof(ddram); i++) {
    ddram[i] = ' ';
  }
  increment = 1;
  AC = 0;
  shiftChars = 0;
  updateDisplay();
}

void VGALiquidCrystal::home() {
  AC = 0;
  shiftChars = 0;
}

void VGALiquidCrystal::setCursor(uint8_t col, uint8_t row) {
  int row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };
  if (row > _lines) {
    row = _lines - 1;
  }
  AC = col + row_offsets[row];
}

void VGALiquidCrystal::noDisplay() {
  display_on = false;
  updateDisplay();
}

void VGALiquidCrystal::display() {
  display_on = true;
  updateDisplay();
}

void VGALiquidCrystal::noCursor() {
  cursor_on = false;
}

void VGALiquidCrystal::cursor() {
  cursor_on = true;
}

void VGALiquidCrystal::noBlink() {
  blink_on = false;
}

void VGALiquidCrystal::blink() {
  blink_on = true;
}

void VGALiquidCrystal::scrollDisplayLeft(void) {
  shiftChars++;
  updateDisplay();
}

void VGALiquidCrystal::scrollDisplayRight(void) {
  shiftChars--;
  updateDisplay();
}

void VGALiquidCrystal::leftToRight(void) {
  increment = 1;
}

void VGALiquidCrystal::rightToLeft(void) {
  increment = -1;
}

void VGALiquidCrystal::autoscroll(void) {
  scroll_on = true;
}

void VGALiquidCrystal::noAutoscroll(void) {
  scroll_on = false;
}

void VGALiquidCrystal::createChar(uint8_t location, uint8_t charmap[]) {
  location &= 0x7;  // Only 8 custom characters (0-7)
  int offset = location * 8;
  
  for (int i = 0; i < 8; i++) {
    chrtbl[offset + i] = charmap[i];
  }

  // Redraw any characters using this custom char
  if (display_on) {
    for (int i = 0; i < displayRange; i++) {
      if (currentDisplayChars[i].chr == location) {
        putcharat(currentDisplayChars[i].chr, i, false);
      }
    }
  }
  updateDisplay();
}

void VGALiquidCrystal::command(uint8_t value) {
  // For compatibility - not used in VGA mode
}

void VGALiquidCrystal::putcharat(uint8_t chr, int pos, bool reverse) {
  int col = pos % 16;
  int line = pos / 16;

  // Character dimensions: 5 pixels wide + 1 spacing, 8 pixels tall + 1 spacing
  int charWidth = 6;
  int charHeight = 9;
  
  int startX = _x0 + (charWidth * col);
  int startY = _y0 + (charHeight * line);
  
  const unsigned char *vchar = &chrtbl[chr * 8];

  for (int row = 0; row < 8; row++) {
    unsigned char v = vchar[row];
    for (int c = 0; c < 5; c++) {
      int px = startX + c;
      int py = startY + row;
      
      // Bounds check
      if (px >= 0 && px < VGA_HSIZE && py >= 0 && py < VGA_VSIZE) {
        bool pixelOn = (v >> (4 - c)) & 0x01;
        if (pixelOn) {
          VGA.putPixel(px, py, reverse ? _bgColor : _textColor);
        } else {
          VGA.putPixel(px, py, reverse ? _textColor : _bgColor);
        }
      }
    }
  }
}

void VGALiquidCrystal::initCurrentDisplayChars() {
  for (int i = 0; i < 16; i++) {
    currentDisplayChars[i].chr = 0x20;
    currentDisplayChars[i].addr = i;
  }
  for (int i = 16; i < 32; i++) {
    currentDisplayChars[i].chr = 0x20;
    currentDisplayChars[i].addr = 48 + i;
  }
}

void VGALiquidCrystal::setCurrentDisplayChars() {
  if (_lines == 1) {
    for (int i = 0; i < 16; i++) {
      currentDisplayChars[i].addr = (i + 80 + shiftChars) % 80;
    }
  } else {
    for (int i = 0; i < 16; i++) {
      currentDisplayChars[i].addr = (i + 40 + shiftChars) % 40;
    }
    for (int i = 16; i < 32; i++) {
      if (shiftChars < 0) {
        currentDisplayChars[i].addr = (i + 40 + shiftChars + 48);
        if (currentDisplayChars[i].addr > 103)
          currentDisplayChars[i].addr = (currentDisplayChars[i].addr % 104) + 64;
      } else {
        currentDisplayChars[i].addr = i + shiftChars + 48;
        if (currentDisplayChars[i].addr > 103)
          currentDisplayChars[i].addr = (currentDisplayChars[i].addr % 104) + 64;
      }
    }
  }
}

void VGALiquidCrystal::blankDisplay() {
  for (int i = 0; i < displayRange; i++) {
    putcharat(' ', i);
  }

  for (int i = 0; i < 32; i++) {
    currentDisplayChars[i].chr = ' ';
  }
}

void VGALiquidCrystal::updateCharAt(int i) {
  if (currentDisplayChars[i].chr != ddram[currentDisplayChars[i].addr]) {
    putcharat(ddram[currentDisplayChars[i].addr], i, false);
    currentDisplayChars[i].chr = ddram[currentDisplayChars[i].addr];
  }
}

void VGALiquidCrystal::updateDisplay() {
  setCurrentDisplayChars();

  if (!display_on) {
    blankDisplay();
    return;
  }
  
  // Wrap shift values
  if (_lines == 0) {
    if (shiftChars > 79 || shiftChars < -79) {
      shiftChars = 0;
    }
  } else {
    if (shiftChars > 39 || shiftChars < -39) {
      shiftChars = 0;
    }
  }
  
  // Update first line
  for (int i = 0; i < 16; i++) {
    updateCharAt(i);
  }
  
  // Update second line if present
  if (_lines > 0) {
    for (int i = 16; i < 32; i++) {
      updateCharAt(i);
    }
  }
}

size_t VGALiquidCrystal::write(uint8_t value) {
  ddram[AC] = value;
  
  if (scroll_on) {
    shiftChars += increment;
  }
  
  // Advance cursor
  if (_lines == 0) {
    AC += increment;
    if (AC < 0) AC = 79;
    AC %= 80;
  } else {
    AC += increment;
    if (AC < 0) {
      AC = 103;
    }
    if (AC > 39 && AC < 64) {
      AC = 64;
    }
    if (AC > 103) {
      AC = 0;
    }
  }

  updateDisplay();
  return 1;
}

// 5x8 LCD font - ASCII characters starting from space (32)
unsigned char VGALiquidCrystal::chrtbl[2048] = {
// Custom characters 0-7 (user-definable)
0, 0, 0, 0, 0, 0, 0, 0,  // 0
0, 0, 0, 0, 0, 0, 0, 0,  // 1
0, 0, 0, 0, 0, 0, 0, 0,  // 2
0, 0, 0, 0, 0, 0, 0, 0,  // 3
0, 0, 0, 0, 0, 0, 0, 0,  // 4
0, 0, 0, 0, 0, 0, 0, 0,  // 5
0, 0, 0, 0, 0, 0, 0, 0,  // 6
0, 0, 0, 0, 0, 0, 0, 0,  // 7
// Reserved 8-31
0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0,
// Space (32)
0, 0, 0, 0, 0, 0, 0, 0,
// ! (33)
4, 4, 4, 4, 4, 0, 4, 0,
// " (34)
10, 10, 10, 0, 0, 0, 0, 0,
// # (35)
10, 10, 31, 10, 31, 10, 10, 0,
// $ (36)
4, 15, 20, 14, 5, 30, 4, 0,
// % (37)
24, 25, 2, 4, 8, 19, 3, 0,
// & (38)
12, 18, 20, 8, 21, 18, 13, 0,
// ' (39)
12, 4, 8, 0, 0, 0, 0, 0,
// ( (40)
2, 4, 8, 8, 8, 4, 2, 0,
// ) (41)
8, 4, 2, 2, 2, 4, 8, 0,
// * (42)
0, 4, 21, 14, 21, 4, 0, 0,
// + (43)
0, 4, 4, 31, 4, 4, 0, 0,
// , (44)
0, 0, 0, 0, 12, 4, 8, 0,
// - (45)
0, 0, 0, 31, 0, 0, 0, 0,
// . (46)
0, 0, 0, 0, 0, 12, 12, 0,
// / (47)
0, 1, 2, 4, 8, 16, 0, 0,
// 0 (48)
14, 17, 19, 21, 25, 17, 14, 0,
// 1 (49)
4, 12, 4, 4, 4, 4, 14, 0,
// 2 (50)
14, 17, 1, 2, 4, 8, 31, 0,
// 3 (51)
31, 2, 4, 2, 1, 17, 14, 0,
// 4 (52)
2, 6, 10, 18, 31, 2, 2, 0,
// 5 (53)
31, 16, 30, 1, 1, 17, 14, 0,
// 6 (54)
6, 8, 16, 30, 17, 17, 14, 0,
// 7 (55)
31, 1, 2, 4, 8, 8, 8, 0,
// 8 (56)
14, 17, 17, 14, 17, 17, 14, 0,
// 9 (57)
14, 17, 17, 15, 1, 2, 12, 0,
// : (58)
0, 12, 12, 0, 12, 12, 0, 0,
// ; (59)
0, 12, 12, 0, 12, 4, 8, 0,
// < (60)
2, 4, 8, 16, 8, 4, 2, 0,
// = (61)
0, 0, 31, 0, 31, 0, 0, 0,
// > (62)
16, 8, 4, 2, 4, 8, 16, 0,
// ? (63)
14, 17, 1, 2, 4, 0, 4, 0,
// @ (64)
14, 17, 1, 13, 21, 21, 14, 0,
// A (65)
14, 17, 17, 17, 31, 17, 17, 0,
// B (66)
30, 17, 17, 30, 17, 17, 30, 0,
// C (67)
14, 17, 16, 16, 16, 17, 14, 0,
// D (68)
30, 17, 17, 17, 17, 17, 30, 0,
// E (69)
31, 16, 16, 30, 16, 16, 31, 0,
// F (70)
31, 16, 16, 30, 16, 16, 16, 0,
// G (71)
14, 17, 16, 23, 17, 17, 15, 0,
// H (72)
17, 17, 17, 31, 17, 17, 17, 0,
// I (73)
14, 4, 4, 4, 4, 4, 14, 0,
// J (74)
7, 2, 2, 2, 2, 18, 12, 0,
// K (75)
17, 18, 20, 24, 20, 18, 17, 0,
// L (76)
16, 16, 16, 16, 16, 16, 31, 0,
// M (77)
17, 27, 21, 21, 17, 17, 17, 0,
// N (78)
17, 17, 25, 21, 19, 17, 17, 0,
// O (79)
14, 17, 17, 17, 17, 17, 14, 0,
// P (80)
30, 17, 17, 30, 16, 16, 16, 0,
// Q (81)
14, 17, 17, 17, 21, 18, 13, 0,
// R (82)
30, 17, 17, 30, 20, 18, 17, 0,
// S (83)
15, 16, 16, 14, 1, 1, 30, 0,
// T (84)
31, 4, 4, 4, 4, 4, 4, 0,
// U (85)
17, 17, 17, 17, 17, 17, 14, 0,
// V (86)
17, 17, 17, 17, 17, 10, 4, 0,
// W (87)
17, 17, 17, 21, 21, 21, 10, 0,
// X (88)
17, 17, 10, 4, 10, 17, 17, 0,
// Y (89)
17, 17, 17, 10, 4, 4, 4, 0,
// Z (90)
31, 1, 2, 4, 8, 16, 31, 0,
// [ (91)
14, 8, 8, 8, 8, 8, 14, 0,
// \ (92)
17, 10, 31, 4, 31, 4, 4, 0,
// ] (93)
14, 2, 2, 2, 2, 2, 14, 0,
// ^ (94)
4, 10, 17, 0, 0, 0, 0, 0,
// _ (95)
0, 0, 0, 0, 0, 0, 31, 0,
// ` (96)
8, 4, 2, 0, 0, 0, 0, 0,
// a (97)
0, 0, 14, 1, 15, 17, 15, 0,
// b (98)
16, 16, 22, 25, 17, 17, 30, 0,
// c (99)
0, 0, 14, 16, 16, 17, 14, 0,
// d (100)
1, 1, 13, 19, 17, 17, 15, 0,
// e (101)
0, 0, 14, 17, 31, 16, 14, 0,
// f (102)
6, 9, 8, 28, 8, 8, 8, 0,
// g (103)
0, 0, 15, 17, 15, 1, 14, 0,
// h (104)
16, 16, 22, 25, 17, 17, 17, 0,
// i (105)
4, 0, 12, 4, 4, 4, 14, 0,
// j (106)
2, 0, 6, 2, 2, 18, 12, 0,
// k (107)
16, 16, 18, 20, 24, 20, 18, 0,
// l (108)
12, 4, 4, 4, 4, 4, 14, 0,
// m (109)
0, 0, 26, 21, 21, 17, 17, 0,
// n (110)
0, 0, 22, 25, 17, 17, 17, 0,
// o (111)
0, 0, 14, 17, 17, 17, 14, 0,
// p (112)
0, 0, 30, 17, 30, 16, 16, 0,
// q (113)
0, 0, 13, 19, 15, 1, 1, 0,
// r (114)
0, 0, 22, 25, 16, 16, 16, 0,
// s (115)
0, 0, 15, 16, 14, 1, 30, 0,
// t (116)
8, 8, 28, 8, 8, 9, 6, 0,
// u (117)
0, 0, 17, 17, 17, 19, 13, 0,
// v (118)
0, 0, 17, 17, 17, 10, 4, 0,
// w (119)
0, 0, 17, 17, 21, 21, 10, 0,
// x (120)
0, 0, 17, 10, 4, 10, 17, 0,
// y (121)
0, 0, 17, 17, 15, 1, 14, 0,
// z (122)
0, 0, 31, 2, 4, 8, 31, 0,
// { (123)
2, 4, 4, 8, 4, 4, 2, 0,
// | (124)
4, 4, 4, 4, 4, 4, 4, 0,
// } (125)
8, 4, 4, 2, 4, 4, 8, 0,
// ~ (126)
0, 4, 2, 31, 2, 4, 0, 0,
// DEL (127)
0, 4, 8, 31, 8, 4, 0, 0,
// Padding to 2048 bytes (256 characters * 8 bytes each)
};
