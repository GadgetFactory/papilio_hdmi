# Papilio Arcade Board - Development Changelog

## 2025-11-10 - HDMI Video Output with Wishbone Control

### Added
- **HDMI Video Output (1280x720@60Hz)**
  - Integrated Gowin DVI_TX_Top IP core for TMDS encoding and serialization
  - Implemented TMDS_rPLL for clock generation (27MHz → 371.25MHz serial, 74.25MHz pixel)
  - Added CLKDIV for pixel clock generation (÷5)
  - Integrated testpattern module with 4 display modes:
    - Mode 0: Color bars
    - Mode 1: Grid pattern
    - Mode 2: Grayscale ramp
    - Mode 3: Solid color (configurable)
  - Pin mapping on Bank 3 (1.8V differential):
    - P6/T6: TMDS Clock
    - M6/T8: TMDS Data[0]
    - T11/P11: TMDS Data[1]
    - T12/R11: TMDS Data[2]

- **Wishbone HDMI Controller (wb_video_ctrl.v)**
  - Wishbone slave at addresses 0x10-0x1F
  - Register 0x10: Pattern mode selection (read/write)
  - Register 0x11: Status/version (read-only)
  - video_top_wb.v wrapper for pattern mode control

- **RGB LED Wishbone Controller**
  - Wishbone slave at addresses 0x00-0x0F
  - WS2812B protocol implementation
  - Registers:
    - 0x00: Green value (0-255)
    - 0x01: Red value (0-255)
    - 0x02: Blue value (0-255)
    - 0x03: Control/status

- **Wishbone Infrastructure**
  - SPI to Wishbone bridge (simple_spi_wb_bridge_debug.v)
  - UART debug output on MISO line during idle
  - 2-slave address decoder (wb_address_decoder.v)
  - 8-bit address and data width

- **ESP32 Control Software**
  - Arduino sketch (esp32_rgb_led_example.ino)
  - SPI interface for Wishbone communication
  - Functions:
    - `setLEDColor()` - Control RGB LED
    - `setVideoPattern()` - Change HDMI test pattern
    - `getVideoPattern()` - Read current pattern
    - `getVideoStatus()` - Get controller version
  - Example loop demonstrating synchronized LED and video control

### Technical Details
- **FPGA**: GW2A-LV18PG256C8/I7 (GW2A-18C series)
- **System Clock**: 27MHz
- **Video Timing**: 1280x720@60Hz (SMPTE 296M)
  - H: 1650 total / 40 sync / 220 back porch / 1280 active
  - V: 750 total / 5 sync / 20 back porch / 720 active
  - Positive sync polarity
- **PLL Configuration**:
  - FCLKIN: 27MHz
  - IDIV: 3 (÷4)
  - FBDIV: 54 (×55)
  - ODIV: 2 (÷2)
  - VCO: 371.25MHz
  - CLKOUT: 371.25MHz (serial clock for TMDS)
  - Pixel clock: 74.25MHz (serial ÷5)

### Fixed
- **Bank 3 Voltage Constraint**: Added IO_TYPE=LVCMOS18D override to force 1.8V differential signaling
- **PLL Frequency**: Corrected from GW2AR-18C (Tang Primer) to GW2A-18C (Papilio) parameters
- **Timing Constraints**: Added base clock constraint for 27MHz input (37.04ns period)

### Module Hierarchy
```
top.v
├── simple_spi_wb_bridge_debug.v (SPI → Wishbone)
│   └── uart_tx.v (Debug output)
├── wb_address_decoder.v (2-slave decoder)
├── wb_simple_rgb_led.v (Slave 0: RGB LED)
│   └── wb_rgb_led_ctrl.v
│       └── ws2812b_controller.v
└── wb_video_ctrl.v (Slave 1: HDMI)
    └── video_top_wb.v
        ├── testpattern.v (Pattern generator)
        ├── TMDS_rPLL.v (Clock generation)
        ├── CLKDIV.v (Pixel clock divider)
        └── DVI_TX_Top.v (TMDS encoder - Gowin IP)
```

### Files Added
- `src/wb_video_ctrl.v` - Wishbone HDMI controller
- `src/video_top_wb.v` - HDMI wrapper with pattern control
- `src/video_top.v` - Core HDMI implementation (from Tang Primer 20K)
- `src/testpattern.v` - Video pattern generator
- `src/dvi_tx/dvi_tx.v` - Gowin DVI transmitter IP (encrypted)
- `src/gowin_rpll/TMDS_rPLL.v` - PLL for HDMI clocking
- `src/wb_simple_rgb_led.v` - RGB LED Wishbone controller
- `src/wb_rgb_led_ctrl.v` - WS2812B controller
- `src/wb_address_decoder.v` - Wishbone address decoder
- `src/simple_spi_wb_bridge_debug.v` - SPI bridge with UART debug
- `src/uart_tx.v` - UART transmitter
- `src/pins.cst` - Pin constraints
- `src/timing.sdc` - Timing constraints

### Files Modified
- `src/top.v` - Added Wishbone infrastructure and HDMI integration
- `src/esp32_rgb_led_example.ino` - Added HDMI control functions
- `papilio_arcade_board.gprj` - Updated project file list

### Known Issues
- Bank 3 operates at 1.8V instead of standard 3.3V for HDMI, but works correctly with LVCMOS18D
- Address decoder generates optimization warning (non-critical, slave 1 still functional)
- CLKDIV clock constraint warning (non-critical, derived clocks inferred automatically)

### Performance
- Compilation: ~2 minutes (depends on system)
- Resource utilization:
  - Logic elements: ~15% (video + Wishbone)
  - Memory blocks: Used for pattern generator
  - PLLs: 1/2 used
- HDMI signal quality: Stable, no artifacts at 1280x720@60Hz

### Testing
- ✅ HDMI output verified on monitor (1280x720@60Hz detected)
- ✅ All 4 test patterns display correctly
- ✅ RGB LED control via SPI working
- ✅ Pattern switching via Wishbone verified
- ✅ ESP32 SPI communication stable
- ✅ UART debug output functional

### Future Enhancements
- Add framebuffer support for custom graphics
- Implement sprite engine
- Add more Wishbone slaves (audio, GPIO, etc.)
- Support additional video resolutions (640x480, 800x600)
- Add DMA controller for high-speed data transfer
