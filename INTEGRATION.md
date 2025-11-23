# papilio_hdmi Integration Guide

This library provides HDMI video output control for the Papilio Arcade Board via Wishbone-over-SPI.

## Quick Start

### Firmware Integration

**Add to `platformio.ini`:**
```ini
lib_deps = 
    https://github.com/GadgetFactory/papilio_hdmi.git#main
```

**Basic Usage:**
```cpp
#include <HDMIController.h>

SPIClass *fpgaSPI = nullptr;
HDMIController *hdmi = nullptr;

void setup() {
    // Initialize SPI
    fpgaSPI = new SPIClass(HSPI);
    fpgaSPI->begin(12, 9, 11, 10); // CLK, MISO, MOSI, CS
    
    // Initialize HDMI (make persistent)
    hdmi = new HDMIController(fpgaSPI, 10, 12, 11, 9);
    hdmi->begin();
    hdmi->setVideoPattern(1); // 0=color bar, 1=net grid, 2=gray, 3=single color
}

void loop() {
    // Change pattern
    hdmi->setVideoPattern(0);
    delay(2000);
}
```

### Gateware Integration

**Wishbone Address:** 0x10-0x1F (Slave 1 recommended)

**Top-Level Module Ports:**
```verilog
module top (
    // ... existing ports ...
    
    // HDMI TMDS differential outputs
    output wire O_tmds_clk_p,
    output wire O_tmds_clk_n,
    output wire [2:0] O_tmds_data_p,  // {red, green, blue}
    output wire [2:0] O_tmds_data_n
);
```

**Wishbone Slave Signals:**
```verilog
// Add to your top module
wire [7:0] s1_wb_adr;
wire [7:0] s1_wb_dat_o;
wire [7:0] s1_wb_dat_i;
wire s1_wb_cyc;
wire s1_wb_stb;
wire s1_wb_we;
wire s1_wb_ack;
```

**Module Instantiation:**
```verilog
wb_video_ctrl u_wb_video_ctrl (
    .clk(clk_27mhz),
    .rst_n(rst_n),
    .wb_adr_i(s1_wb_adr),
    .wb_dat_i(s1_wb_dat_o),
    .wb_dat_o(s1_wb_dat_i),
    .wb_cyc_i(s1_wb_cyc),
    .wb_stb_i(s1_wb_stb),
    .wb_we_i(s1_wb_we),
    .wb_ack_o(s1_wb_ack),
    .O_tmds_clk_p(O_tmds_clk_p),
    .O_tmds_clk_n(O_tmds_clk_n),
    .O_tmds_data_p(O_tmds_data_p),
    .O_tmds_data_n(O_tmds_data_n)
);
```

### Pin Constraints (pins.cst)

**For GW2A-18C (Papilio Arcade Board):**
```
IO_LOC "O_tmds_clk_p" P6,T6;
IO_PORT "O_tmds_clk_p" IO_TYPE=LVCMOS18D PULL_MODE=NONE DRIVE=8;
IO_LOC "O_tmds_data_p[0]" M6,T8;
IO_PORT "O_tmds_data_p[0]" IO_TYPE=LVCMOS18D PULL_MODE=NONE DRIVE=8;
IO_LOC "O_tmds_data_p[1]" T11,P11;
IO_PORT "O_tmds_data_p[1]" IO_TYPE=LVCMOS18D PULL_MODE=NONE DRIVE=8;
IO_LOC "O_tmds_data_p[2]" T12,R11;
IO_PORT "O_tmds_data_p[2]" IO_TYPE=LVCMOS18D PULL_MODE=NONE DRIVE=8;
```

**Note:** 
- Uses LVCMOS18D (1.8V differential) - verify your FPGA bank voltage
- First pin is positive (_p), second is negative (_n)
- Adjust pin numbers for your specific board

### Required Gateware Files

Add these files to your `.gprj` project file:

**Core modules:**
```xml
<File path="path/to/papilio_hdmi/gateware/src/wb_video_ctrl.v" type="file.verilog" enable="1"/>
<File path="path/to/papilio_hdmi/gateware/src/video_top_wb.v" type="file.verilog" enable="1"/>
<File path="path/to/papilio_hdmi/gateware/src/testpattern.v" type="file.verilog" enable="1"/>
<File path="path/to/papilio_hdmi/gateware/src/TMDS_rPLL.v" type="file.verilog" enable="1"/>
```

**DVI transmitter (Required - Gowin IP core):**
```xml
<File path="path/to/papilio_hdmi/gateware/src/dvi_tx/dvi_tx.v" type="file.verilog" enable="1"/>
```

**Note:** The `dvi_tx.v` is an encrypted Gowin IP core and must be included for proper HDMI output.

## Video Patterns

The library supports 4 video modes (I_mode in testpattern.v):

| Pattern | Name | Description |
|---------|------|-------------|
| 0 | color bar | 8-color vertical bars |
| 1 | net grid | 32-pixel grid pattern |
| 2 | gray | Grayscale gradient |
| 3 | text mode | 80x30 character display with 16 colors |

## Text Mode Feature

### Overview

Text mode provides a classic 80x30 character display with 16 foreground and 16 background colors, similar to VGA text mode. Characters are rendered using an 8x8 VGA font.

### Text Mode Integration

**Additional Wishbone Slave Required:**

Text mode requires a second Wishbone slave (Slave 2) for character RAM access at address 0x20-0x2F.

**Add to address decoder in `top.v`:**
```verilog
// Slave 2 signals for character RAM
wire [7:0] s2_wb_adr;
wire [7:0] s2_wb_dat_o;
wire [7:0] s2_wb_dat_i;
wire s2_wb_cyc;
wire s2_wb_stb;
wire s2_wb_we;
wire s2_wb_ack;
```

**Instantiate character RAM module:**
```verilog
wb_char_ram u_wb_char_ram (
    .clk(clk_27mhz),
    .rst_n(rst_n),
    .wb_adr_i(s2_wb_adr),
    .wb_dat_i(s2_wb_dat_o),
    .wb_dat_o(s2_wb_dat_i),
    .wb_cyc_i(s2_wb_cyc),
    .wb_stb_i(s2_wb_stb),
    .wb_we_i(s2_wb_we),
    .wb_ack_o(s2_wb_ack)
);
```

**Add to `.gprj` project:**
```xml
<File path="path/to/papilio_hdmi/gateware/src/wb_char_ram.v" type="file.verilog" enable="1"/>
<File path="path/to/papilio_hdmi/gateware/src/char_ram_8x8.v" type="file.verilog" enable="1"/>
<File path="path/to/papilio_hdmi/gateware/src/wb_text_mode.v" type="file.verilog" enable="1"/>
```

### Firmware Text Mode API

**Enable/Disable:**
```cpp
hdmi.enableTextMode();   // Switch to text mode (pattern 3)
hdmi.disableTextMode();  // Switch back to color bars
```

**Screen Control:**
```cpp
hdmi.clearScreen();                     // Clear screen, reset cursor to 0,0
hdmi.setCursor(x, y);                   // Set cursor position (0-79, 0-29)
hdmi.setTextColor(fg, bg);             // Set foreground and background colors
```

**Text Output:**
```cpp
hdmi.writeChar('A');                    // Write single character at cursor
hdmi.writeString("Hello World");       // Write string
hdmi.print("Text");                     // Write text (no newline)
hdmi.println("Text");                   // Write text with newline
```

**Cursor Query:**
```cpp
uint8_t x = hdmi.getCursorX();         // Get current X position (0-79)
uint8_t y = hdmi.getCursorY();         // Get current Y position (0-29)
```

### Text Colors

16 standard VGA colors available:

| Value | Name | Value | Name |
|-------|------|-------|------|
| 0x00 | COLOR_BLACK | 0x08 | COLOR_DARK_GRAY |
| 0x01 | COLOR_BLUE | 0x09 | COLOR_LIGHT_BLUE |
| 0x02 | COLOR_GREEN | 0x0A | COLOR_LIGHT_GREEN |
| 0x03 | COLOR_CYAN | 0x0B | COLOR_LIGHT_CYAN |
| 0x04 | COLOR_RED | 0x0C | COLOR_LIGHT_RED |
| 0x05 | COLOR_MAGENTA | 0x0D | COLOR_LIGHT_MAGENTA |
| 0x06 | COLOR_BROWN | 0x0E | COLOR_YELLOW |
| 0x07 | COLOR_LIGHT_GRAY | 0x0F | COLOR_WHITE |

**Example:**
```cpp
// Green text on black background
hdmi.setTextColor(COLOR_LIGHT_GREEN, COLOR_BLACK);
hdmi.println("System Ready");

// Yellow text on blue background
hdmi.setTextColor(COLOR_YELLOW, COLOR_BLUE);
hdmi.println("Warning!");
```

### Character RAM Register Map (Slave 2: 0x20-0x2F)

| Address | Name | Access | Description |
|---------|------|--------|-------------|
| 0x20 | CONTROL | R/W | Control register (bit 0: clear screen) |
| 0x21 | CURSOR_X | R/W | Cursor X position (0-79) |
| 0x22 | CURSOR_Y | R/W | Cursor Y position (0-29) |
| 0x23 | ATTR | R/W | Default attribute (FG/BG color) |
| 0x24 | CHAR | W | Write character at cursor (auto-advance) |
| 0x25 | ATTR_WR | W | Write attribute at cursor |
| 0x26 | ADDR_HI | R/W | RAM address pointer (high nibble) |
| 0x27 | ADDR_LO | R/W | RAM address pointer (low byte) |
| 0x28 | DATA_WR | R/W | Direct RAM write with auto-increment |
| 0x29 | ATTR_DATA | R/W | Direct attribute write with auto-increment |

### Complete Example

See `examples/papilio_hdmi_text_example/papilio_hdmi_text_example.ino` for a complete working example showing:
- Text mode initialization
- Color palette demonstration
- Live counter display
- Screen positioning and formatting

## Video Timing

Default: **720p @ 60Hz (1280x720)**
- Pixel clock: 74.25 MHz (generated by TMDS_rPLL from 27 MHz input)
- Horizontal: 1650 total, 1280 active
- Vertical: 750 total, 720 active

## Architecture Details

**Signal Flow:**
```
ESP32-S3 (SPI) → Wishbone Bridge → wb_video_ctrl → video_top_wb → DVI_TX_Top → HDMI
                                         ↓
                                    TMDS_rPLL (74.25 MHz)
                                         ↓
                                   testpattern → TMDS encoder
```

**Clock Domains:**
- System clock: 27 MHz (FPGA input)
- Serial clock: 371.25 MHz (from TMDS_rPLL, 5x pixel clock)
- Pixel clock: 74.25 MHz (from CLKDIV, serial/5)

## Hardware Requirements

- **FPGA:** Gowin GW2A-18C or compatible
- **I/O Bank:** 1.8V for LVCMOS18D differential pairs
- **Resources:** ~3000 LUTs, 1 PLL, 1 CLKDIV
- **External:** HDMI connector with proper differential routing

## Troubleshooting

**No HDMI output:**
- Verify DVI IP core (`dvi_tx.v`) is included in project
- Check pin constraints match top-level port names exactly
- Ensure LVCMOS18D is supported on your FPGA I/O bank
- Verify 27 MHz clock is present and stable

**Pattern doesn't change:**
- Check Wishbone address (should be 0x10)
- Verify SPI communication is working (test with RGB LED)
- Monitor serial output for debug messages

**Display shows "no signal":**
- Confirm pixel clock is generated (74.25 MHz)
- Check HDMI cable and display compatibility
- Verify differential pairs are routed correctly (p/n not swapped)

## Reference Implementation

See working example: [papilio_arcade_template](https://github.com/GadgetFactory/papilio_arcade_template)

Key files:
- `src/papliio_arcade_template.ino` - Firmware integration
- `src/gateware/top.v` - Gateware instantiation
- `src/gateware/pins.cst` - Pin constraints

## API Reference

### HDMIController Class

**Constructor:**
```cpp
HDMIController(SPIClass* spi, uint8_t csPin, uint8_t spiClk, uint8_t spiMosi, uint8_t spiMiso);
```

**Video Control Methods:**
- `void begin()` - Initialize HDMI controller
- `void setVideoPattern(uint8_t pattern)` - Set video mode (0-3)
- `uint8_t getVideoPattern()` - Read current pattern
- `uint8_t getVideoStatus()` - Read status register (returns version 0x02 for text mode support)

**Text Mode Methods:**
- `void enableTextMode()` - Enable text mode (pattern 3)
- `void disableTextMode()` - Disable text mode (return to color bars)
- `void clearScreen()` - Clear screen and reset cursor to 0,0
- `void setCursor(uint8_t x, uint8_t y)` - Set cursor position (x: 0-79, y: 0-29)
- `void setTextColor(uint8_t fg, uint8_t bg)` - Set text colors (4-bit each)
- `void writeChar(char c)` - Write single character at cursor
- `void writeString(const char* str)` - Write string at cursor
- `void print(const char* str)` - Write text without newline
- `void println(const char* str)` - Write text with newline
- `uint8_t getCursorX()` - Get current cursor X position
- `uint8_t getCursorY()` - Get current cursor Y position

**Video Register Map (Wishbone Slave 1: 0x10-0x1F):**
- `0x10` - Pattern mode (read/write): 0=color bars, 1=grid, 2=grayscale, 3=text mode
- `0x11` - Version/status (read-only): 0x02 indicates text mode support

**Character RAM Register Map (Wishbone Slave 2: 0x20-0x2F):**
- `0x20` - Control register (bit 0: clear screen trigger)
- `0x21` - Cursor X position (0-79)
- `0x22` - Cursor Y position (0-29)
- `0x23` - Default attribute byte (foreground/background color)
- `0x24` - Write character at cursor (auto-advances)
- `0x25` - Write attribute at cursor
- `0x26-0x27` - Direct RAM address pointer
- `0x28-0x29` - Direct RAM/attribute access with auto-increment

## License

[Include appropriate license information]

## Support

For issues and questions:
- GitHub Issues: https://github.com/GadgetFactory/papilio_hdmi/issues
- Forum: [forum link if available]
