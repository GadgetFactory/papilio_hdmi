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
