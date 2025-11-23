# papilio_hdmi

PlatformIO/Arduino library to control the Papilio Arcade Board FPGA HDMI output and RGB LED via a Wishbone-over-SPI interface.

## Features

- **Video Test Patterns**: Color bars, grid, grayscale patterns for display testing
- **Text Mode**: Classic 80x30 character display with 16 colors (VGA-style)
- **RGB LED Control**: On-board LED control via Wishbone
- **Simple API**: Easy-to-use Arduino-style interface

## Quick Start

```cpp
#include <HDMIController.h>

HDMIController hdmi(nullptr, 10, 12, 11, 9);  // CS, CLK, MOSI, MISO

void setup() {
    hdmi.begin();
    
    // Test patterns
    hdmi.setVideoPattern(0);  // Color bars
    
    // Or use text mode
    hdmi.enableTextMode();
    hdmi.clearScreen();
    hdmi.setTextColor(COLOR_WHITE, COLOR_BLACK);
    hdmi.println("Hello, HDMI!");
}
```

## Examples

- `papilio_hdmi_example/` - Basic HDMI test patterns and RGB LED control
- `papilio_hdmi_text_example/` - Text mode demonstration with colors and cursor control

## Documentation

See `INTEGRATION.md` for complete integration guide including:
- Gateware integration instructions
- Pin constraints
- Address map
- Text mode setup
- API reference
