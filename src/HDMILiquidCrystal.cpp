#include "HDMILiquidCrystal.h"

HDMILiquidCrystal::HDMILiquidCrystal(HDMIController* hdmi, uint8_t cols, uint8_t rows)
  : _hdmi(hdmi), _cols(cols), _rows(rows), 
    _displayOffsetX(0), _displayOffsetY(0),
    _cursorCol(0), _cursorRow(0),
    _displayOn(true), _cursorOn(false), _blinkOn(false),
    _leftToRight(true), _autoscroll(false) {
}

void HDMILiquidCrystal::begin(uint8_t cols, uint8_t rows) {
  if (cols > 0) _cols = cols;
  if (rows > 0) _rows = rows;
  
  if (_hdmi) {
    _hdmi->enableTextMode();
    clear();
    _hdmi->setTextColor(HDMI_COLOR_WHITE, HDMI_COLOR_BLACK);
  }
}

void HDMILiquidCrystal::clear() {
  if (!_hdmi) return;
  
  // Clear only the LCD window area
  for (uint8_t row = 0; row < _rows; row++) {
    _hdmi->setCursor(_displayOffsetX, _displayOffsetY + row);
    for (uint8_t col = 0; col < _cols; col++) {
      _hdmi->writeChar(' ');
    }
  }
  
  home();
}

void HDMILiquidCrystal::home() {
  setCursor(0, 0);
}

void HDMILiquidCrystal::noDisplay() {
  _displayOn = false;
  // Could blank the display area with spaces
}

void HDMILiquidCrystal::display() {
  _displayOn = true;
}

void HDMILiquidCrystal::noCursor() {
  _cursorOn = false;
  updateCursor();
}

void HDMILiquidCrystal::cursor() {
  _cursorOn = true;
  updateCursor();
}

void HDMILiquidCrystal::noBlink() {
  _blinkOn = false;
  updateCursor();
}

void HDMILiquidCrystal::blink() {
  _blinkOn = true;
  updateCursor();
}

void HDMILiquidCrystal::scrollDisplayLeft() {
  // Software scrolling: shift all characters one position left
  for (uint8_t row = 0; row < _rows; row++) {
    uint16_t rowStart = row * 80;
    // Shift characters left within the LCD window
    for (uint8_t col = 0; col < _cols - 1; col++) {
      uint16_t srcAddr = rowStart + _displayOffsetX + col + 1;
      uint16_t dstAddr = rowStart + _displayOffsetX + col;
      
      // Read character from source position using ram_addr_ptr
      _hdmi->writeRegister(REG_CHARRAM_ADDR_HI, (srcAddr >> 8) & 0xFF);
      _hdmi->writeRegister(REG_CHARRAM_ADDR_LO, srcAddr & 0xFF);
      uint8_t ch = _hdmi->readRegister(REG_CHARRAM_DATA_WR);  // Reads from ram_addr_ptr
      
      // Read attribute from source position (ram_addr_ptr was auto-incremented, so set it again)
      _hdmi->writeRegister(REG_CHARRAM_ADDR_HI, (srcAddr >> 8) & 0xFF);
      _hdmi->writeRegister(REG_CHARRAM_ADDR_LO, srcAddr & 0xFF);
      uint8_t attr = _hdmi->readRegister(REG_CHARRAM_ATTR_DATA);  // Reads attr from ram_addr_ptr
      
      // Write to destination position
      _hdmi->writeRegister(REG_CHARRAM_ADDR_HI, (dstAddr >> 8) & 0xFF);
      _hdmi->writeRegister(REG_CHARRAM_ADDR_LO, dstAddr & 0xFF);
      _hdmi->writeRegister(REG_CHARRAM_DATA_WR, ch);
      
      // Write attribute (need to set address again since DATA_WR auto-incremented)
      _hdmi->writeRegister(REG_CHARRAM_ADDR_HI, (dstAddr >> 8) & 0xFF);
      _hdmi->writeRegister(REG_CHARRAM_ADDR_LO, dstAddr & 0xFF);
      _hdmi->writeRegister(REG_CHARRAM_ATTR_DATA, attr);
    }
    // Fill last position with space
    uint16_t lastAddr = rowStart + _displayOffsetX + _cols - 1;
    _hdmi->writeRegister(REG_CHARRAM_ADDR_HI, (lastAddr >> 8) & 0xFF);
    _hdmi->writeRegister(REG_CHARRAM_ADDR_LO, lastAddr & 0xFF);
    _hdmi->writeRegister(REG_CHARRAM_DATA_WR, ' ');
    _hdmi->writeRegister(REG_CHARRAM_ADDR_HI, (lastAddr >> 8) & 0xFF);
    _hdmi->writeRegister(REG_CHARRAM_ADDR_LO, lastAddr & 0xFF);
    _hdmi->writeRegister(REG_CHARRAM_ATTR_DATA, _currentAttr);
  }
}

void HDMILiquidCrystal::scrollDisplayRight() {
  // Software scrolling: shift all characters one position right
  for (uint8_t row = 0; row < _rows; row++) {
    uint16_t rowStart = row * 80;
    // Shift characters right within the LCD window (go backwards to avoid overwriting)
    for (int8_t col = _cols - 1; col > 0; col--) {
      uint16_t srcAddr = rowStart + _displayOffsetX + col - 1;
      uint16_t dstAddr = rowStart + _displayOffsetX + col;
      
      // Read character from source position using ram_addr_ptr
      _hdmi->writeRegister(REG_CHARRAM_ADDR_HI, (srcAddr >> 8) & 0xFF);
      _hdmi->writeRegister(REG_CHARRAM_ADDR_LO, srcAddr & 0xFF);
      uint8_t ch = _hdmi->readRegister(REG_CHARRAM_DATA_WR);  // Reads from ram_addr_ptr
      
      // Read attribute from source position (ram_addr_ptr was auto-incremented, so set it again)
      _hdmi->writeRegister(REG_CHARRAM_ADDR_HI, (srcAddr >> 8) & 0xFF);
      _hdmi->writeRegister(REG_CHARRAM_ADDR_LO, srcAddr & 0xFF);
      uint8_t attr = _hdmi->readRegister(REG_CHARRAM_ATTR_DATA);  // Reads attr from ram_addr_ptr
      
      // Write to destination position
      _hdmi->writeRegister(REG_CHARRAM_ADDR_HI, (dstAddr >> 8) & 0xFF);
      _hdmi->writeRegister(REG_CHARRAM_ADDR_LO, dstAddr & 0xFF);
      _hdmi->writeRegister(REG_CHARRAM_DATA_WR, ch);
      
      // Write attribute (need to set address again since DATA_WR auto-incremented)
      _hdmi->writeRegister(REG_CHARRAM_ADDR_HI, (dstAddr >> 8) & 0xFF);
      _hdmi->writeRegister(REG_CHARRAM_ADDR_LO, dstAddr & 0xFF);
      _hdmi->writeRegister(REG_CHARRAM_ATTR_DATA, attr);
    }
    // Fill first position with space
    uint16_t firstAddr = rowStart + _displayOffsetX;
    _hdmi->writeRegister(REG_CHARRAM_ADDR_HI, (firstAddr >> 8) & 0xFF);
    _hdmi->writeRegister(REG_CHARRAM_ADDR_LO, firstAddr & 0xFF);
    _hdmi->writeRegister(REG_CHARRAM_DATA_WR, ' ');
    _hdmi->writeRegister(REG_CHARRAM_ADDR_HI, (firstAddr >> 8) & 0xFF);
    _hdmi->writeRegister(REG_CHARRAM_ADDR_LO, firstAddr & 0xFF);
    _hdmi->writeRegister(REG_CHARRAM_ATTR_DATA, _currentAttr);
  }
}

void HDMILiquidCrystal::leftToRight() {
  _leftToRight = true;
}

void HDMILiquidCrystal::rightToLeft() {
  _leftToRight = false;
}

void HDMILiquidCrystal::autoscroll() {
  _autoscroll = true;
}

void HDMILiquidCrystal::noAutoscroll() {
  _autoscroll = false;
}

void HDMILiquidCrystal::setCursor(uint8_t col, uint8_t row) {
  if (col >= _cols) col = _cols - 1;
  if (row >= _rows) row = _rows - 1;
  
  _cursorCol = col;
  _cursorRow = row;
  updateCursor();
}

void HDMILiquidCrystal::updateCursor() {
  if (!_hdmi) return;
  _hdmi->setCursor(_displayOffsetX + _cursorCol, _displayOffsetY + _cursorRow);
}

size_t HDMILiquidCrystal::write(uint8_t c) {
  if (!_hdmi || !_displayOn) return 0;
  
  if (c == '\n') {
    // Move to start of next row
    _cursorCol = 0;
    if (_cursorRow < _rows - 1) {
      _cursorRow++;
    } else if (_autoscroll) {
      // Scroll up
      _displayOffsetY++;
    }
    updateCursor();
  } else if (c == '\r') {
    _cursorCol = 0;
    updateCursor();
  } else if (c >= 32 && c <= 126) {
    // Printable character
    if (_cursorCol < _cols) {
      _hdmi->writeChar(c);
      
      if (_leftToRight) {
        _cursorCol++;
        if (_cursorCol >= _cols) {
          if (_autoscroll) {
            // Wrap to next line
            _cursorCol = 0;
            if (_cursorRow < _rows - 1) {
              _cursorRow++;
            } else {
              _displayOffsetY++;
            }
          } else {
            _cursorCol = _cols - 1;
          }
        }
      } else {
        if (_cursorCol > 0) {
          _cursorCol--;
        }
      }
      updateCursor();
    }
  }
  
  return 1;
}

void HDMILiquidCrystal::print(const char* str) {
  while (*str) {
    write(*str++);
  }
}

void HDMILiquidCrystal::println(const char* str) {
  print(str);
  write('\n');
}

void HDMILiquidCrystal::print(int value) {
  char buffer[12];
  snprintf(buffer, sizeof(buffer), "%d", value);
  print(buffer);
}

void HDMILiquidCrystal::setColor(uint8_t foreground, uint8_t background) {
  if (_hdmi) {
    _hdmi->setTextColor(foreground, background);
  }
}
