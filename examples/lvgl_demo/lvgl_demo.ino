/**
 * LVGL Demo for Papilio Arcade HQVGA Display
 * 
 * Demonstrates LVGL running on the 160x120 HDMI framebuffer.
 * Shows various LVGL widgets scaled for the small display.
 * 
 * Hardware:
 *   - Papilio Arcade board (ESP32-S3 + FPGA)
 *   - HDMI output (160x120 scaled to 720p)
 * 
 * Required Libraries:
 *   - lvgl (PlatformIO: lvgl/lvgl)
 *   - papilio_hdmi (local library)
 */

#include <Arduino.h>
#include <SPI.h>

// LVGL must be included before our driver
#include <lvgl.h>

// Our HQVGA LVGL driver
#include <HQVGA_LVGL.h>

// SPI pins for ESP32-S3
#define SPI_CLK   12
#define SPI_MISO  9
#define SPI_MOSI  11
#define SPI_CS    10

SPIClass *spi = nullptr;
HQVGA_LVGL lvglDisplay;

// Demo state
int currentDemo = 0;
const int NUM_DEMOS = 4;
unsigned long lastDemoChange = 0;
const unsigned long DEMO_DURATION = 10000; // 10 seconds per demo

// Demo screens
lv_obj_t* demoScreens[NUM_DEMOS];

// Forward declarations
void createLabelDemo(lv_obj_t* parent);
void createButtonDemo(lv_obj_t* parent);
void createGaugeDemo(lv_obj_t* parent);
void createChartDemo(lv_obj_t* parent);

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n\n=== LVGL Demo for HQVGA ===");
  
  // Initialize SPI
  spi = new SPIClass(HSPI);
  spi->begin(SPI_CLK, SPI_MISO, SPI_MOSI, SPI_CS);
  Serial.println("SPI initialized");
  
  // Initialize LVGL FIRST
  lv_init();
  Serial.println("LVGL initialized");
  
  // Initialize our display driver
  lvglDisplay.begin(spi, SPI_CS, SPI_CLK, SPI_MOSI, SPI_MISO);
  
  // Wait for FPGA to be ready
  Serial.println("Waiting for FPGA...");
  if (lvglDisplay.getVGA().waitForFPGA(10000)) {
    Serial.println("FPGA ready!");
  } else {
    Serial.println("FPGA timeout - continuing anyway");
  }
  
  // Create demo screens
  Serial.println("Creating demo screens...");
  
  for (int i = 0; i < NUM_DEMOS; i++) {
    demoScreens[i] = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(demoScreens[i], lv_color_hex(0x000033), 0);
  }
  
  createLabelDemo(demoScreens[0]);
  createButtonDemo(demoScreens[1]);
  createGaugeDemo(demoScreens[2]);
  createChartDemo(demoScreens[3]);
  
  // Show first demo
  lv_scr_load(demoScreens[0]);
  lastDemoChange = millis();
  
  Serial.println("Setup complete!");
}

void loop() {
  // Handle LVGL tasks
  lv_timer_handler();
  
  // Switch demos periodically
  if (millis() - lastDemoChange > DEMO_DURATION) {
    currentDemo = (currentDemo + 1) % NUM_DEMOS;
    lv_scr_load_anim(demoScreens[currentDemo], LV_SCR_LOAD_ANIM_FADE_IN, 300, 0, false);
    lastDemoChange = millis();
    Serial.printf("Switched to demo %d\n", currentDemo);
  }
  
  delay(5);
}

// Demo 1: Labels and text
void createLabelDemo(lv_obj_t* parent) {
  // Title - use bitmap font for crisp pixels
  lv_obj_t* title = lv_label_create(parent);
  lv_label_set_text(title, "LVGL Demo");
  lv_obj_set_style_text_color(title, lv_color_hex(0xFFFF00), 0);
  lv_obj_set_style_text_font(title, &lv_font_unscii_8, 0);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 5);
  
  // Subtitle
  lv_obj_t* subtitle = lv_label_create(parent);
  lv_label_set_text(subtitle, "160x120 Display");
  lv_obj_set_style_text_color(subtitle, lv_color_hex(0x00FFFF), 0);
  lv_obj_set_style_text_font(subtitle, &lv_font_unscii_8, 0);
  lv_obj_align(subtitle, LV_ALIGN_TOP_MID, 0, 18);
  
  // Info text
  lv_obj_t* info = lv_label_create(parent);
  lv_label_set_text(info, "Papilio\nArcade\nBoard");
  lv_obj_set_style_text_color(info, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_text_font(info, &lv_font_unscii_8, 0);
  lv_obj_set_style_text_align(info, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(info, LV_ALIGN_CENTER, 0, 8);
  
  // Scrolling label at bottom
  lv_obj_t* scroll = lv_label_create(parent);
  lv_label_set_long_mode(scroll, LV_LABEL_LONG_SCROLL_CIRCULAR);
  lv_obj_set_width(scroll, 150);
  lv_label_set_text(scroll, "LVGL + ESP32-S3 + FPGA = Awesome!   ");
  lv_obj_set_style_text_color(scroll, lv_color_hex(0x00FF00), 0);
  lv_obj_align(scroll, LV_ALIGN_BOTTOM_MID, 0, -5);
}

// Demo 2: Buttons
void createButtonDemo(lv_obj_t* parent) {
  // Title
  lv_obj_t* title = lv_label_create(parent);
  lv_label_set_text(title, "Buttons");
  lv_obj_set_style_text_color(title, lv_color_hex(0xFFFF00), 0);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 2);
  
  // Create 3x2 button grid
  static const char* btnLabels[] = {"1", "2", "3", "4", "5", "6"};
  static lv_color_t btnColors[] = {
    lv_color_hex(0xFF0000), lv_color_hex(0x00FF00), lv_color_hex(0x0000FF),
    lv_color_hex(0xFFFF00), lv_color_hex(0xFF00FF), lv_color_hex(0x00FFFF)
  };
  
  for (int i = 0; i < 6; i++) {
    int row = i / 3;
    int col = i % 3;
    
    lv_obj_t* btn = lv_btn_create(parent);
    lv_obj_set_size(btn, 45, 35);
    lv_obj_set_pos(btn, 10 + col * 50, 20 + row * 45);
    lv_obj_set_style_bg_color(btn, btnColors[i], 0);
    lv_obj_set_style_radius(btn, 5, 0);
    
    lv_obj_t* label = lv_label_create(btn);
    lv_label_set_text(label, btnLabels[i]);
    lv_obj_center(label);
  }
}

// Demo 3: Arc/Gauge
void createGaugeDemo(lv_obj_t* parent) {
  // Title
  lv_obj_t* title = lv_label_create(parent);
  lv_label_set_text(title, "Meters");
  lv_obj_set_style_text_color(title, lv_color_hex(0xFFFF00), 0);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 2);
  
  // Arc 1 - left
  lv_obj_t* arc1 = lv_arc_create(parent);
  lv_obj_set_size(arc1, 50, 50);
  lv_arc_set_rotation(arc1, 135);
  lv_arc_set_bg_angles(arc1, 0, 270);
  lv_arc_set_value(arc1, 65);
  lv_obj_set_style_arc_color(arc1, lv_color_hex(0x00FF00), LV_PART_INDICATOR);
  lv_obj_set_style_arc_width(arc1, 6, LV_PART_INDICATOR);
  lv_obj_set_style_arc_width(arc1, 6, LV_PART_MAIN);
  lv_obj_align(arc1, LV_ALIGN_LEFT_MID, 15, 5);
  lv_obj_clear_flag(arc1, LV_OBJ_FLAG_CLICKABLE);
  
  // Arc 2 - right
  lv_obj_t* arc2 = lv_arc_create(parent);
  lv_obj_set_size(arc2, 50, 50);
  lv_arc_set_rotation(arc2, 135);
  lv_arc_set_bg_angles(arc2, 0, 270);
  lv_arc_set_value(arc2, 35);
  lv_obj_set_style_arc_color(arc2, lv_color_hex(0xFF0000), LV_PART_INDICATOR);
  lv_obj_set_style_arc_width(arc2, 6, LV_PART_INDICATOR);
  lv_obj_set_style_arc_width(arc2, 6, LV_PART_MAIN);
  lv_obj_align(arc2, LV_ALIGN_RIGHT_MID, -15, 5);
  lv_obj_clear_flag(arc2, LV_OBJ_FLAG_CLICKABLE);
  
  // Bar in center
  lv_obj_t* bar = lv_bar_create(parent);
  lv_obj_set_size(bar, 30, 60);
  lv_bar_set_value(bar, 75, LV_ANIM_OFF);
  lv_obj_set_style_bg_color(bar, lv_color_hex(0x333333), LV_PART_MAIN);
  lv_obj_set_style_bg_color(bar, lv_color_hex(0x00FFFF), LV_PART_INDICATOR);
  lv_obj_center(bar);
  
  // Animate the meters
  lv_anim_t a;
  lv_anim_init(&a);
  lv_anim_set_var(&a, arc1);
  lv_anim_set_values(&a, 0, 100);
  lv_anim_set_time(&a, 2000);
  lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
  lv_anim_set_playback_time(&a, 2000);
  lv_anim_set_exec_cb(&a, [](void* obj, int32_t v) {
    lv_arc_set_value((lv_obj_t*)obj, v);
  });
  lv_anim_start(&a);
  
  lv_anim_set_var(&a, arc2);
  lv_anim_set_values(&a, 100, 0);
  lv_anim_start(&a);
  
  lv_anim_set_var(&a, bar);
  lv_anim_set_exec_cb(&a, [](void* obj, int32_t v) {
    lv_bar_set_value((lv_obj_t*)obj, v, LV_ANIM_OFF);
  });
  lv_anim_start(&a);
}

// Demo 4: Chart
void createChartDemo(lv_obj_t* parent) {
  // Title
  lv_obj_t* title = lv_label_create(parent);
  lv_label_set_text(title, "Chart");
  lv_obj_set_style_text_color(title, lv_color_hex(0xFFFF00), 0);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 2);
  
  // Create chart
  lv_obj_t* chart = lv_chart_create(parent);
  lv_obj_set_size(chart, 140, 80);
  lv_obj_align(chart, LV_ALIGN_CENTER, 0, 8);
  lv_chart_set_type(chart, LV_CHART_TYPE_LINE);
  lv_chart_set_point_count(chart, 20);
  lv_obj_set_style_bg_color(chart, lv_color_hex(0x111122), 0);
  lv_obj_set_style_line_color(chart, lv_color_hex(0x333355), LV_PART_MAIN);
  
  // Add data series
  lv_chart_series_t* ser1 = lv_chart_add_series(chart, lv_color_hex(0x00FF00), LV_CHART_AXIS_PRIMARY_Y);
  lv_chart_series_t* ser2 = lv_chart_add_series(chart, lv_color_hex(0xFF0000), LV_CHART_AXIS_PRIMARY_Y);
  
  // Set some data points
  static int32_t data1[] = {10, 25, 40, 55, 45, 60, 70, 65, 80, 75, 90, 85, 70, 60, 50, 55, 65, 75, 80, 85};
  static int32_t data2[] = {90, 75, 60, 45, 55, 40, 30, 35, 20, 25, 10, 15, 30, 40, 50, 45, 35, 25, 20, 15};
  
  for (int i = 0; i < 20; i++) {
    lv_chart_set_next_value(chart, ser1, data1[i]);
    lv_chart_set_next_value(chart, ser2, data2[i]);
  }
  
  lv_chart_refresh(chart);
}
