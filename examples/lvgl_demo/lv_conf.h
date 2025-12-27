/**
 * @file lv_conf.h
 * Configuration file for LVGL v8.x on Papilio Arcade HQVGA display
 * 
 * Copy this file to your project's src/ or include/ folder
 * 
 * For 160x120 RGB332 display optimized settings
 */

#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

/*====================
   COLOR SETTINGS
 *====================*/

/* Color depth: 8 (RGB332), 16 (RGB565), or 32 (ARGB8888)
 * Use 8 for best performance with HQVGA's native RGB332 format */
#define LV_COLOR_DEPTH 16

/* Swap the 2 bytes of RGB565 color (for big-endian displays) */
#define LV_COLOR_16_SWAP 0

/* Enable anti-aliasing (text, lines) - DISABLED for crisp pixels at low res */
#define LV_ANTIALIAS 0

/* Default display refresh period in milliseconds */
#define LV_DISP_DEF_REFR_PERIOD 30

/* Dot Per Inch: used to scale font sizes */
#define LV_DPI_DEF 130

/*====================
   MEMORY SETTINGS
 *====================*/

/* Size of the memory used for `lv_mem_alloc` in bytes (>= 2kB) */
#define LV_MEM_SIZE (32U * 1024U)

/* Use standard C malloc/free */
#define LV_MEM_CUSTOM 0

/* Use the standard `memcpy` and `memset` */
#define LV_MEMCPY_MEMSET_STD 1

/*====================
   HAL SETTINGS
 *====================*/

/* Default tick period in milliseconds */
#define LV_TICK_CUSTOM 1
#define LV_TICK_CUSTOM_INCLUDE "Arduino.h"
#define LV_TICK_CUSTOM_SYS_TIME_EXPR (millis())

/*====================
   FEATURE CONFIGURATION
 *====================*/

/*-------------
 * Drawing
 *-----------*/

/* Enable complex draw engine features */
#define LV_DRAW_COMPLEX 1

/* Shadow drawing - disable for small display */
#define LV_DRAW_SW_SHADOW_CACHE_SIZE 0

/* Allow buffering some shadow calculation */
#define LV_SHADOW_CACHE_SIZE 0

/* Set number of maximally cached circle data */
#define LV_CIRCLE_CACHE_SIZE 4

/*-------------
 * Logging
 *-----------*/

/* Enable logging - useful for debugging */
#define LV_USE_LOG 1
#if LV_USE_LOG
    /* How important log should be added:
     * LV_LOG_LEVEL_TRACE/DEBUG/INFO/WARN/ERROR/USER/NONE */
    #define LV_LOG_LEVEL LV_LOG_LEVEL_WARN
    
    /* Print to serial */
    #define LV_LOG_PRINTF 1
#endif

/*-------------
 * Asserts
 *-----------*/

#define LV_USE_ASSERT_NULL          1
#define LV_USE_ASSERT_MALLOC        1
#define LV_USE_ASSERT_STYLE         0
#define LV_USE_ASSERT_MEM_INTEGRITY 0
#define LV_USE_ASSERT_OBJ           0

/*-------------
 * Others
 *-----------*/

/* Garbage Collector settings */
#define LV_ENABLE_GC 0

/*====================
 * COMPILER SETTINGS
 *====================*/

/* For big endian systems */
#define LV_BIG_ENDIAN_SYSTEM 0

/* Define a custom attribute to `lv_tick_inc` function */
#define LV_ATTRIBUTE_TICK_INC

/* Define a custom attribute to `lv_timer_handler` function */
#define LV_ATTRIBUTE_TIMER_HANDLER

/* Define a custom attribute for `lv_disp_flush_ready` function */
#define LV_ATTRIBUTE_FLUSH_READY

/* Required alignment size for buffers */
#define LV_ATTRIBUTE_MEM_ALIGN_SIZE 4

/* Prefix performance critical functions to place in faster memory */
#define LV_ATTRIBUTE_FAST_MEM

/* Export integer constant to binding (for MicroPython) */
#define LV_EXPORT_CONST_INT(int_value) struct _silence_gcc_warning

/* Prefix variables in the GPU accelerated operations */
#define LV_ATTRIBUTE_LARGE_CONST
#define LV_ATTRIBUTE_LARGE_RAM_ARRAY

/*====================
 *   FONT USAGE
 *====================*/

/* Built-in fonts - enable small fonts for 160x120 display */
#define LV_FONT_MONTSERRAT_8  1
#define LV_FONT_MONTSERRAT_10 1
#define LV_FONT_MONTSERRAT_12 1
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_16 0
#define LV_FONT_MONTSERRAT_18 0
#define LV_FONT_MONTSERRAT_20 0
#define LV_FONT_MONTSERRAT_22 0
#define LV_FONT_MONTSERRAT_24 0
#define LV_FONT_MONTSERRAT_26 0
#define LV_FONT_MONTSERRAT_28 0
#define LV_FONT_MONTSERRAT_30 0
#define LV_FONT_MONTSERRAT_32 0
#define LV_FONT_MONTSERRAT_34 0
#define LV_FONT_MONTSERRAT_36 0
#define LV_FONT_MONTSERRAT_38 0
#define LV_FONT_MONTSERRAT_40 0
#define LV_FONT_MONTSERRAT_42 0
#define LV_FONT_MONTSERRAT_44 0
#define LV_FONT_MONTSERRAT_46 0
#define LV_FONT_MONTSERRAT_48 0

/* Monospace font for small displays */
#define LV_FONT_UNSCII_8  1
#define LV_FONT_UNSCII_16 0

/* Default font - use UNSCII bitmap font for crisp pixels at 160x120 */
#define LV_FONT_DEFAULT &lv_font_unscii_8

/* Enable text encoding support */
#define LV_TXT_ENC LV_TXT_ENC_UTF8

/*====================
 *   WIDGETS
 *====================*/

/* Basic widgets - enable commonly used ones */
#define LV_USE_ARC        1
#define LV_USE_BAR        1
#define LV_USE_BTN        1
#define LV_USE_BTNMATRIX  1
#define LV_USE_CANVAS     0  /* Memory intensive */
#define LV_USE_CHECKBOX   1
#define LV_USE_DROPDOWN   1
#define LV_USE_IMG        1
#define LV_USE_LABEL      1
#define LV_USE_LINE       1
#define LV_USE_ROLLER     1
#define LV_USE_SLIDER     1
#define LV_USE_SWITCH     1
#define LV_USE_TEXTAREA   1
#define LV_USE_TABLE      1

/* Extra widgets */
#define LV_USE_ANIMIMG    0
#define LV_USE_CALENDAR   0
#define LV_USE_CHART      1
#define LV_USE_COLORWHEEL 0
#define LV_USE_IMGBTN     0
#define LV_USE_KEYBOARD   0
#define LV_USE_LED        1
#define LV_USE_LIST       1
#define LV_USE_MENU       0
#define LV_USE_METER      1
#define LV_USE_MSGBOX     1
#define LV_USE_SPAN       0
#define LV_USE_SPINBOX    1
#define LV_USE_SPINNER    1
#define LV_USE_TABVIEW    0
#define LV_USE_TILEVIEW   0
#define LV_USE_WIN        0

/*====================
 *   THEMES
 *====================*/

/* Simple theme - good for small displays */
#define LV_USE_THEME_DEFAULT 1
#if LV_USE_THEME_DEFAULT
    /* Dark mode for better contrast on small display */
    #define LV_THEME_DEFAULT_DARK 1
    /* Enable grow on press */
    #define LV_THEME_DEFAULT_GROW 0
    /* Transition time in ms */
    #define LV_THEME_DEFAULT_TRANSITION_TIME 80
#endif

/* Basic theme - minimal resources */
#define LV_USE_THEME_BASIC 1

/* Mono theme for very limited colors */
#define LV_USE_THEME_MONO 0

/*====================
 *   LAYOUTS
 *====================*/

#define LV_USE_FLEX 1
#define LV_USE_GRID 1

/*====================
 * 3RD PARTY LIBRARIES
 *====================*/

/* File system interfaces - disable to save memory */
#define LV_USE_FS_STDIO 0
#define LV_USE_FS_POSIX 0
#define LV_USE_FS_WIN32 0
#define LV_USE_FS_FATFS 0

/* PNG/BMP/GIF decoders - disable to save memory */
#define LV_USE_PNG 0
#define LV_USE_BMP 0
#define LV_USE_SJPG 0
#define LV_USE_GIF 0
#define LV_USE_QRCODE 0
#define LV_USE_FREETYPE 0
#define LV_USE_TINY_TTF 0
#define LV_USE_RLOTTIE 0
#define LV_USE_FFMPEG 0

/*====================
 *   EXAMPLES
 *====================*/

/* Enable examples to be built with library */
#define LV_BUILD_EXAMPLES 0

#endif /* LV_CONF_H */
