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
  // Shift the display window to the right (content appears to scroll left)
  if (_displayOffsetX + _cols < 80) {
    _displayOffsetX++;
    updateCursor();
  }
}

void HDMILiquidCrystal::scrollDisplayRight() {
  // Shift the display window to the left (content appears to scroll right)
  if (_displayOffsetX > 0) {
    _displayOffsetX--;
    updateCursor();
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
