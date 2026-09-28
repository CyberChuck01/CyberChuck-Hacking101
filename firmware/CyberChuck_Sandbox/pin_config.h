// ============================================================================
//  pin_config.h  --  CyberChuck // Hacking 101
//  Supports BOTH the LILYGO T-Dongle-C5 (ESP32-C5) and T-Dongle-S3 (ESP32-S3).
//  Pick your board below; pins + LED method + SD method switch automatically.
// ============================================================================
#pragma once

// ============================================================================
//   >>> SELECT YOUR BOARD  (exactly one) <<<
// ============================================================================
//#define BOARD_C5        // LILYGO T-Dongle-C5  (ESP32-C5)
#define BOARD_S3      // LILYGO T-Dongle-S3  (ESP32-S3)
// ============================================================================

#if defined(BOARD_C5) && defined(BOARD_S3)
  #error "Pick ONE board: comment out either BOARD_C5 or BOARD_S3."
#endif
#if !defined(BOARD_C5) && !defined(BOARD_S3)
  #error "Pick a board: uncomment BOARD_C5 or BOARD_S3."
#endif

// ---- Feature switches (same for both boards) -------------------------------
#define USE_DISPLAY  1     // 0 for a bare/no-screen build
#define USE_LED      1     // onboard APA102 status light
#define USE_SD       0     // OPTIONAL (voice clips); confirm SD pins first
#define LED_BRIGHTNESS 8   // 1-31
#define LED_ORDER_BGR  1   // flip to 0 if LED colours look wrong

// ---- Shared ST7735 80x160 tuning (same panel on both) ----------------------
#define LCD_IPS      true
#define LCD_BGR      true
#define LCD_COL_OFF  26
#define LCD_ROW_OFF   1
#define LCD_BL_ACTIVE_LOW 1   // both dongles: backlight is active-LOW
#define BTN_PIN      0

// ============================================================================
#if defined(BOARD_C5)
// ------------------------------- T-Dongle-C5 --------------------------------
  #define LCD_SCLK  6
  #define LCD_MOSI  2
  #define LCD_MISO  7
  #define LCD_DC    3
  #define LCD_CS    10
  #define LCD_RST   1
  #define LCD_BL    0

  // APA102 shares the display's SPI pins (DI=2/MOSI, CI=6/SCK) -> drive over SPI
  #define LED_ON_DISPLAY_BUS 1
  #define LED_DATA  2
  #define LED_CLK   6

  // C5 has NO SDMMC peripheral -> SD runs over SPI (confirm pins if you use it)
  #define SD_MODE_SPI 1
  #define SD_SCK   12
  #define SD_MISO  13
  #define SD_MOSI  11
  #define SD_CS    4

#elif defined(BOARD_S3)
// ------------------------------- T-Dongle-S3 --------------------------------
  #define LCD_SCLK  5
  #define LCD_MOSI  3
  #define LCD_MISO  -1
  #define LCD_DC    2
  #define LCD_CS    4
  #define LCD_RST   1
  #define LCD_BL    38

  // APA102 has its own dedicated pins (separate bus) -> bit-bang them
  #define LED_ON_DISPLAY_BUS 0
  #define LED_DATA  40
  #define LED_CLK   39

  // S3 HAS SDMMC hardware -> use SD_MMC (1-bit)
  #define SD_MODE_SPI 0
  #define SD_MMC_CLK  12
  #define SD_MMC_CMD  16
  #define SD_MMC_D0   14

  // If S3 colours look inverted, try flipping LCD_IPS/LCD_BGR above.
#endif
