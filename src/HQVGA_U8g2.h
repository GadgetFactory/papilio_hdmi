/*
  HQVGA_U8g2 - U8g2 adapter for HQVGA framebuffer display
  
  This class provides U8g2 compatibility for the HQVGA display,
  giving access to U8g2's excellent pixel-perfect bitmap fonts
  and drawing primitives.
  
  U8g2 is ideal for retro/arcade displays because:
  - Bitmap fonts are pixel-perfect (no anti-aliasing blur)
  - Many classic/retro font styles available
  - Efficient for text-heavy displays
  - Full buffer mode works well with our framebuffer
  
  Hardware:
  - Papilio Arcade board with ESP32-S3 and FPGA
  - HDMI output (160x120 scaled to 720p)
  
  Usage:
    #include <SPI.h>
    #include <HQVGA_U8g2.h>
    
    HQVGA_U8g2 u8g2;
    
    void setup() {
      SPIClass *spi = new SPIClass(HSPI);
      spi->begin(12, 9, 11, 10);
      u8g2.begin(spi, 10, 12, 11, 9);
      
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_ncenB08_tr);
      u8g2.drawStr(10, 20, "Hello U8g2!");
      u8g2.sendBuffer();
    }
    
  Color Support:
    U8g2 is monochrome by default. This adapter extends it with color:
    - setDrawColor(rgb332) - Set drawing color (RGB332 format)
    - setDrawColor(r, g, b) - Set drawing color (RGB888, converted to RGB332)
    - Drawing uses foreground color, clear uses background color
    
  Note: U8g2 fonts are 1-bit (on/off). The adapter draws "on" pixels
  in the foreground color and leaves "off" pixels unchanged (transparent)
  or clears to background color when using clearBuffer().
*/

#ifndef HQVGA_U8G2_h
#define HQVGA_U8G2_h

#include <U8g2lib.h>
#include <HQVGA.h>

// Display dimensions
#define HQVGA_U8G2_WIDTH  160
#define HQVGA_U8G2_HEIGHT 120

// Custom U8g2 class for HQVGA framebuffer
class HQVGA_U8g2 : public U8G2 {
public:
  HQVGA_U8g2() : _fgColor(0xFF), _bgColor(0x00), _spi(nullptr) {
    // Use full framebuffer mode (F = full buffer)
    // We'll handle the actual drawing ourselves
  }
  
  // Initialize the display
  void begin(SPIClass* spi = nullptr, uint8_t csPin = 10, 
             uint8_t spiClk = 12, uint8_t spiMosi = 11, uint8_t spiMiso = 9,
             uint8_t wishboneBase = 0x00) {
    
    _spi = spi;
    
    // Initialize HQVGA hardware
    VGA.begin(spi, csPin, spiClk, spiMosi, spiMiso, wishboneBase);
    
    // Initialize U8g2 with our custom callback
    // Using a generic full-buffer setup
    u8g2_SetupBuffer_Null(&u8g2, &u8g2_cb_r0, HQVGA_U8G2_WIDTH, HQVGA_U8G2_HEIGHT);
    
    // Allocate our own buffer for U8g2's 1-bit drawing
    _buffer = (uint8_t*)malloc((HQVGA_U8G2_WIDTH * HQVGA_U8G2_HEIGHT + 7) / 8);
    if (_buffer) {
      memset(_buffer, 0, (HQVGA_U8G2_WIDTH * HQVGA_U8G2_HEIGHT + 7) / 8);
    }
    
    // Set tile dimensions for U8g2
    u8g2.tile_buf_height = HQVGA_U8G2_HEIGHT / 8;
    u8g2.tile_buf_ptr = _buffer;
    
    u8x8_InitDisplay(&u8g2.u8x8);
    u8x8_SetPowerSave(&u8g2.u8x8, 0);
    
    Serial.println("HQVGA_U8g2: Initialized 160x120 display");
  }
  
  // Set foreground color (RGB332)
  void setFgColor(uint8_t color) { _fgColor = color; }
  
  // Set background color (RGB332)
  void setBgColor(uint8_t color) { _bgColor = color; }
  
  // Set foreground color (RGB888 -> RGB332)
  void setFgColor(uint8_t r, uint8_t g, uint8_t b) {
    _fgColor = toRGB332(r, g, b);
  }
  
  // Set background color (RGB888 -> RGB332)
  void setBgColor(uint8_t r, uint8_t g, uint8_t b) {
    _bgColor = toRGB332(r, g, b);
  }
  
  // Clear the buffer (fills with background color)
  void clearBuffer() {
    if (_buffer) {
      memset(_buffer, 0, (HQVGA_U8G2_WIDTH * HQVGA_U8G2_HEIGHT + 7) / 8);
    }
    // Also clear the HQVGA framebuffer
    VGA.setBackgroundColor(_bgColor);
    VGA.clear();
  }
  
  // Send the U8g2 buffer to the HQVGA framebuffer
  void sendBuffer() {
    if (!_buffer) return;
    
    // Convert 1-bit U8g2 buffer to RGB332 HQVGA framebuffer
    for (int y = 0; y < HQVGA_U8G2_HEIGHT; y++) {
      for (int x = 0; x < HQVGA_U8G2_WIDTH; x++) {
        // U8g2 stores pixels in vertical bytes (8 pixels per byte, LSB at top)
        int byteIndex = (y / 8) * HQVGA_U8G2_WIDTH + x;
        int bitIndex = y % 8;
        
        if (_buffer[byteIndex] & (1 << bitIndex)) {
          VGA.putPixel(x, y, _fgColor);
        }
        // Background pixels already set by clearBuffer()
      }
    }
  }
  
  // Draw a pixel (for direct drawing, bypasses U8g2 buffer)
  void drawPixelDirect(int16_t x, int16_t y, uint8_t color) {
    if (x >= 0 && x < HQVGA_U8G2_WIDTH && y >= 0 && y < HQVGA_U8G2_HEIGHT) {
      VGA.putPixel(x, y, color);
    }
  }
  
  // Get current foreground color
  uint8_t getFgColor() const { return _fgColor; }
  
  // Get current background color
  uint8_t getBgColor() const { return _bgColor; }
  
  // Convert RGB888 to RGB332
  static uint8_t toRGB332(uint8_t r, uint8_t g, uint8_t b) {
    return ((r >> 5) << 5) | ((g >> 5) << 2) | (b >> 6);
  }
  
  // Predefined colors (RGB332)
  static const uint8_t BLACK   = 0x00;
  static const uint8_t RED     = 0xE0;
  static const uint8_t GREEN   = 0x1C;
  static const uint8_t BLUE    = 0x03;
  static const uint8_t YELLOW  = 0xFC;
  static const uint8_t CYAN    = 0x1F;
  static const uint8_t MAGENTA = 0xE3;
  static const uint8_t WHITE   = 0xFF;
  static const uint8_t ORANGE  = 0xF4;
  static const uint8_t PURPLE  = 0x63;
  static const uint8_t PINK    = 0xF3;
  
  // Access to underlying VGA object
  VGA_class& getVGA() { return VGA; }

private:
  uint8_t _fgColor;
  uint8_t _bgColor;
  uint8_t* _buffer;
  SPIClass* _spi;
  
  // Custom U8g2 setup for null/custom display
  static void u8g2_SetupBuffer_Null(u8g2_t *u8g2, const u8g2_cb_t *rotation,
                                     uint16_t width, uint16_t height) {
    u8g2_SetupDisplay(u8g2, u8x8_d_null_cb, u8x8_cad_empty, u8x8_byte_empty, u8x8_gpio_and_delay_cb);
    u8g2->u8x8.display_info = &null_display_info;
    u8g2_SetupBuffer(u8g2, nullptr, 0, u8g2_ll_hvline_vertical_top_lsb, rotation);
  }
  
  // Null display callbacks
  static uint8_t u8x8_d_null_cb(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
    return 1;
  }
  
  static uint8_t u8x8_gpio_and_delay_cb(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
    return 1;
  }
  
  // Display info for 160x120
  static const u8x8_display_info_t null_display_info;
};

// Static display info definition
const u8x8_display_info_t HQVGA_U8g2::null_display_info = {
  /* chip_enable_level = */ 0,
  /* chip_disable_level = */ 1,
  /* post_chip_enable_wait_ns = */ 0,
  /* pre_chip_disable_wait_ns = */ 0,
  /* reset_pulse_width_ms = */ 0,
  /* post_reset_wait_ms = */ 0,
  /* sda_setup_time_ns = */ 0,
  /* sck_pulse_width_ns = */ 0,
  /* sck_clock_hz = */ 0,
  /* spi_mode = */ 0,
  /* i2c_bus_clock_100kHz = */ 0,
  /* data_setup_time_ns = */ 0,
  /* write_pulse_width_ns = */ 0,
  /* tile_width = */ HQVGA_U8G2_WIDTH / 8,
  /* tile_height = */ HQVGA_U8G2_HEIGHT / 8,
  /* default_x_offset = */ 0,
  /* flipmode_x_offset = */ 0,
  /* pixel_width = */ HQVGA_U8G2_WIDTH,
  /* pixel_height = */ HQVGA_U8G2_HEIGHT
};

#endif
