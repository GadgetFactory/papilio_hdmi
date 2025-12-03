/*
  HQVGA_LVGL - LVGL display driver for HQVGA framebuffer
  
  This provides LVGL integration for the 160x120 HQVGA display.
  Works with LVGL v8.x (v9.x has different API, see notes below).
  
  Hardware:
  - Papilio Arcade board with ESP32-S3 and FPGA
  - HDMI output (160x120 scaled to 720p)
  
  Usage:
    #include <lvgl.h>
    #include <HQVGA_LVGL.h>
    
    HQVGA_LVGL lvglDisplay;
    
    void setup() {
      SPIClass *spi = new SPIClass(HSPI);
      spi->begin(12, 9, 11, 10);
      
      lv_init();
      lvglDisplay.begin(spi, 10, 12, 11, 9);
      
      // Create LVGL widgets...
      lv_obj_t *label = lv_label_create(lv_scr_act());
      lv_label_set_text(label, "Hello LVGL!");
      lv_obj_center(label);
    }
    
    void loop() {
      lv_timer_handler();  // Call every ~5ms
      delay(5);
    }
    
  Color Format:
    LVGL uses RGB565 (16-bit) by default, but our display uses RGB332 (8-bit).
    The driver handles conversion automatically in the flush callback.
    For best performance, configure LVGL for LV_COLOR_DEPTH 8 in lv_conf.h
*/

#ifndef HQVGA_LVGL_h
#define HQVGA_LVGL_h

#include <lvgl.h>
#include <HQVGA.h>

// Display dimensions
#define HQVGA_LVGL_WIDTH  160
#define HQVGA_LVGL_HEIGHT 120

// Buffer size - full frame for best performance at this small resolution
#define HQVGA_LVGL_BUF_SIZE (HQVGA_LVGL_WIDTH * HQVGA_LVGL_HEIGHT)

class HQVGA_LVGL {
public:
  HQVGA_LVGL() : _initialized(false) {}
  
  // Initialize display and LVGL driver
  // Call lv_init() BEFORE calling this!
  void begin(SPIClass* spi = nullptr, uint8_t csPin = 10, 
             uint8_t spiClk = 12, uint8_t spiMosi = 11, uint8_t spiMiso = 9,
             uint8_t wishboneBase = 0x00) {
    
    // Initialize HQVGA hardware
    VGA.begin(spi, csPin, spiClk, spiMosi, spiMiso, wishboneBase);
    
    // Store instance for static callback
    _instance = this;
    
    // Allocate draw buffer(s)
    // Using single buffer - for double buffering, allocate two and pass both
    _buf1 = (lv_color_t*)heap_caps_malloc(HQVGA_LVGL_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA);
    if (!_buf1) {
      // Fallback to regular malloc
      _buf1 = (lv_color_t*)malloc(HQVGA_LVGL_BUF_SIZE * sizeof(lv_color_t));
    }
    
    if (!_buf1) {
      Serial.println("HQVGA_LVGL: Failed to allocate draw buffer!");
      return;
    }
    
#if LV_VERSION_CHECK(9, 0, 0)
    // LVGL v9.x API
    _display = lv_display_create(HQVGA_LVGL_WIDTH, HQVGA_LVGL_HEIGHT);
    lv_display_set_buffers(_display, _buf1, NULL, HQVGA_LVGL_BUF_SIZE * sizeof(lv_color_t), LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(_display, flushCallback);
    lv_display_set_user_data(_display, this);
#else
    // LVGL v8.x API
    lv_disp_draw_buf_init(&_drawBuf, _buf1, NULL, HQVGA_LVGL_BUF_SIZE);
    
    lv_disp_drv_init(&_dispDrv);
    _dispDrv.hor_res = HQVGA_LVGL_WIDTH;
    _dispDrv.ver_res = HQVGA_LVGL_HEIGHT;
    _dispDrv.flush_cb = flushCallback;
    _dispDrv.draw_buf = &_drawBuf;
    _dispDrv.user_data = this;
    
    _display = lv_disp_drv_register(&_dispDrv);
#endif
    
    _initialized = true;
    Serial.println("HQVGA_LVGL: Initialized 160x120 display");
  }
  
  // Check if initialized
  bool isInitialized() const { return _initialized; }
  
  // Access underlying VGA object
  VGA_class& getVGA() { return VGA; }
  
  // Convert RGB888 to RGB332 for HQVGA display
  static uint8_t toRGB332(uint8_t r, uint8_t g, uint8_t b) {
    return ((r >> 5) << 5) | ((g >> 5) << 2) | (b >> 6);
  }
  
  // Convert lv_color_t to RGB332
  static uint8_t lvColorToRGB332(lv_color_t color) {
#if LV_COLOR_DEPTH == 8
    // Already 8-bit, but LVGL uses RGB233 by default
    // We need RGB332, so remap if needed
    return color.full;
#elif LV_COLOR_DEPTH == 16
    // RGB565 to RGB332
    uint8_t r = (color.ch.red << 3) >> 5;    // 5 bits -> 3 bits
    uint8_t g = (color.ch.green << 2) >> 5;  // 6 bits -> 3 bits
    uint8_t b = color.ch.blue >> 3;          // 5 bits -> 2 bits
    return (r << 5) | (g << 2) | b;
#elif LV_COLOR_DEPTH == 32
    // ARGB8888 to RGB332
    return toRGB332(color.ch.red, color.ch.green, color.ch.blue);
#else
    return 0;
#endif
  }

private:
  bool _initialized;
  lv_color_t* _buf1;
  static HQVGA_LVGL* _instance;
  
#if LV_VERSION_CHECK(9, 0, 0)
  lv_display_t* _display;
  
  static void flushCallback(lv_display_t* disp, const lv_area_t* area, uint8_t* px_map) {
    HQVGA_LVGL* instance = (HQVGA_LVGL*)lv_display_get_user_data(disp);
    lv_color_t* color_p = (lv_color_t*)px_map;
#else
  lv_disp_drv_t _dispDrv;
  lv_disp_draw_buf_t _drawBuf;
  lv_disp_t* _display;
  
  static void flushCallback(lv_disp_drv_t* disp, const lv_area_t* area, lv_color_t* color_p) {
    HQVGA_LVGL* instance = (HQVGA_LVGL*)disp->user_data;
#endif
    
    // Write pixels to HQVGA framebuffer
    for (int y = area->y1; y <= area->y2; y++) {
      for (int x = area->x1; x <= area->x2; x++) {
        uint8_t rgb332 = lvColorToRGB332(*color_p);
        VGA.putPixel(x, y, rgb332);
        color_p++;
      }
    }
    
    // Tell LVGL we're done flushing
#if LV_VERSION_CHECK(9, 0, 0)
    lv_display_flush_ready(disp);
#else
    lv_disp_flush_ready(disp);
#endif
  }
};

// Static instance pointer for callback
HQVGA_LVGL* HQVGA_LVGL::_instance = nullptr;

#endif
