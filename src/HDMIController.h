#ifndef HDMI_CONTROLLER_H
#define HDMI_CONTROLLER_H

#include <Arduino.h>
#include <SPI.h>

// SPI Wishbone Protocol Commands
#define CMD_WRITE 0x01
#define CMD_READ  0x02

// 8-bit Wishbone Register Addresses - RGB LED (Slave 0: 0x00-0x0F)
#define REG_LED_GREEN  0x0000
#define REG_LED_RED    0x0001
#define REG_LED_BLUE   0x0002
#define REG_LED_CTRL   0x0003

// 8-bit Wishbone Register Addresses - HDMI Video (Slave 1: 0x10-0x1F)
#define REG_VIDEO_PATTERN  0x0010
#define REG_VIDEO_STATUS   0x0011

// 8-bit Wishbone Register Addresses - Character RAM (Slave 2: 0x20-0x2F)
#define REG_CHARRAM_CONTROL   0x0020
#define REG_CHARRAM_CURSOR_X  0x0021
#define REG_CHARRAM_CURSOR_Y  0x0022
#define REG_CHARRAM_ATTR      0x0023
#define REG_CHARRAM_CHAR      0x0024
#define REG_CHARRAM_ATTR_WR   0x0025
#define REG_CHARRAM_ADDR_HI   0x0026
#define REG_CHARRAM_ADDR_LO   0x0027
#define REG_CHARRAM_DATA_WR   0x0028
#define REG_CHARRAM_ATTR_DATA 0x0029

// Video pattern modes
#define PATTERN_COLOR_BARS  0x00
#define PATTERN_GRID        0x01
#define PATTERN_GRAYSCALE   0x02
#define PATTERN_TEXT_MODE   0x03

// Text colors (4-bit: [3]=bright, [2]=red, [1]=green, [0]=blue)
#define COLOR_BLACK         0x00
#define COLOR_BLUE          0x01
#define COLOR_GREEN         0x02
#define COLOR_CYAN          0x03
#define COLOR_RED           0x04
#define COLOR_MAGENTA       0x05
#define COLOR_BROWN         0x06
#define COLOR_LIGHT_GRAY    0x07
#define COLOR_DARK_GRAY     0x08
#define COLOR_LIGHT_BLUE    0x09
#define COLOR_LIGHT_GREEN   0x0A
#define COLOR_LIGHT_CYAN    0x0B
#define COLOR_LIGHT_RED     0x0C
#define COLOR_LIGHT_MAGENTA 0x0D
#define COLOR_YELLOW        0x0E
#define COLOR_WHITE         0x0F

class HDMIController {
public:
  HDMIController(SPIClass* spi = nullptr, uint8_t csPin = 10, uint8_t spiClk = 12, uint8_t spiMosi = 11, uint8_t spiMiso = 9);
  ~HDMIController();

  void begin();

  void setLEDColor(uint32_t color);
  void setLEDColorRGB(uint8_t red, uint8_t green, uint8_t blue);
  bool isLEDBusy();

  void setVideoPattern(uint8_t pattern);
  uint8_t getVideoPattern();
  uint8_t getVideoStatus();

  // Text mode functions
  void enableTextMode();
  void disableTextMode();
  void clearScreen();
  void setCursor(uint8_t x, uint8_t y);
  void setTextColor(uint8_t foreground, uint8_t background);
  void writeChar(char c);
  void writeString(const char* str);
  void println(const char* str);
  void print(const char* str);
  uint8_t getCursorX();
  uint8_t getCursorY();

private:
  SPIClass* _spi;
  bool _ownSpi;
  uint8_t _cs;
  uint8_t _clk, _mosi, _miso;

  void wishboneWrite8(uint16_t address, uint8_t data);
  uint8_t wishboneRead8(uint16_t address);
  void wishboneWrite(uint32_t address, uint32_t data);
  uint32_t wishboneRead(uint32_t address);
};

#endif // HDMI_CONTROLLER_H
