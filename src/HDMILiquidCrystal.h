#ifndef HDMI_LIQUID_CRYSTAL_H
#define HDMI_LIQUID_CRYSTAL_H

#include <Arduino.h>
#include "HDMIController.h"

/**
 * HDMILiquidCrystal - LCD-compatible interface for HDMI text mode
 * 
 * This class provides an Arduino LiquidCrystal-compatible API for controlling
 * text display on HDMI output. It wraps the HDMIController text mode functions.
 * 
 * The HDMI display is 80x26 characters (or configurable), but this class
 * emulates a smaller LCD window (default 16x2) that can scroll within the
 * larger display area.
 */
class HDMILiquidCrystal {
public:
  /**
   * Constructor - creates LCD emulation on HDMI display
   * @param hdmi Pointer to initialized HDMIController instance
   * @param cols Number of columns for LCD emulation (default 16)
   * @param rows Number of rows for LCD emulation (default 2)
   */
  HDMILiquidCrystal(HDMIController* hdmi, uint8_t cols = 16, uint8_t rows = 2);

  /**
   * Initialize the LCD display
   * @param cols Number of columns (optional, uses constructor value if 0)
   * @param rows Number of rows (optional, uses constructor value if 0)
   */
  void begin(uint8_t cols = 0, uint8_t rows = 0);

  /**
   * Clear the LCD display and position cursor at (0,0)
   */
  void clear();

  /**
   * Position cursor at home (0,0) without clearing
   */
  void home();

  /**
   * Turn off the display (blank it)
   */
  void noDisplay();

  /**
   * Turn on the display
   */
  void display();

  /**
   * Turn off the cursor
   */
  void noCursor();

  /**
   * Turn on the cursor (underline)
   */
  void cursor();

  /**
   * Turn off cursor blinking
   */
  void noBlink();

  /**
   * Turn on cursor blinking
   */
  void blink();

  /**
   * Scroll display left without changing RAM
   */
  void scrollDisplayLeft();

  /**
   * Scroll display right without changing RAM
   */
  void scrollDisplayRight();

  /**
   * Set text flow left to right
   */
  void leftToRight();

  /**
   * Set text flow right to left
   */
  void rightToLeft();

  /**
   * Right-justify text from cursor
   */
  void autoscroll();

  /**
   * Left-justify text from cursor
   */
  void noAutoscroll();

  /**
   * Set cursor position
   * @param col Column (0-based)
   * @param row Row (0-based)
   */
  void setCursor(uint8_t col, uint8_t row);

  /**
   * Write a single character at current cursor position
   * @param c Character to write
   */
  size_t write(uint8_t c);

  /**
   * Print a string at current cursor position
   * @param str String to print
   */
  void print(const char* str);

  /**
   * Print a string followed by newline
   * @param str String to print
   */
  void println(const char* str);

  /**
   * Print an integer
   * @param value Integer to print
   */
  void print(int value);

  /**
   * Set text and background color
   * @param foreground Foreground color (use HDMI_COLOR_* constants)
   * @param background Background color (use HDMI_COLOR_* constants)
   */
  void setColor(uint8_t foreground, uint8_t background = HDMI_COLOR_BLACK);

  /**
   * Create a custom character
   * @param location Custom character slot (0-7)
   * @param charmap Array of 8 bytes defining the 5x8 character pattern
   * 
   * Note: Custom characters 0-7 are uploaded to FPGA custom font RAM
   */
  void createChar(uint8_t location, uint8_t charmap[]);

private:
  HDMIController* _hdmi;
  uint8_t _cols;
  uint8_t _rows;
  uint8_t _displayOffsetX;
  uint8_t _displayOffsetY;
  uint8_t _cursorCol;
  uint8_t _cursorRow;
  bool _displayOn;
  bool _cursorOn;
  bool _blinkOn;
  bool _leftToRight;
  bool _autoscroll;

  void updateCursor();
  void drawChar(char c);
};

#endif // HDMI_LIQUID_CRYSTAL_H
