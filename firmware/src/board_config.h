#pragma once

// Select one board profile from platformio.ini build_flags:
//   -DBOROVIK_BOARD_DEVKITC_S3
//   -DBOROVIK_BOARD_FREENOVE_WROVER_CAM_MVP

#if defined(BOROVIK_BOARD_FREENOVE_WROVER_CAM_MVP)

#define BOROVIK_BOARD_NAME "Freenove ESP32-Wrover CAM MVP"
#define BOROVIK_HAS_ENCODER 0
#define BOROVIK_HAS_BTN_B   0
#define BOROVIK_HAS_RGB_LED 0

#define BTN_A_PIN  0
#define BTN_B_PIN  -1

#define GPS_RX     32
#define GPS_TX     33
#define GPS_BAUD   9600

#define TFT_SCLK   14
#define TFT_MOSI   13
#define TFT_MISO   -1
#define TFT_CS     15
#define TFT_DC     12
#define TFT_RST    2
#define TFT_BLK    -1
#define TFT_SPI_HOST VSPI

#define SD_SCLK    TFT_SCLK
#define SD_MOSI    TFT_MOSI
#define SD_MISO    -1
#define SD_CS      -1

#define CAMERA_MODEL_WROVER_KIT
#define BOROVIK_DISPLAY_SMOKE_TEST 0
#define BOROVIK_PIN_BEACON_TEST 0

#elif defined(BOROVIK_BOARD_DEVKITC_S3)

#define BOROVIK_BOARD_NAME "ESP32-S3-DevKitC-1 N16R8"
#define BOROVIK_HAS_ENCODER 1
#define BOROVIK_HAS_BTN_B   1
#define BOROVIK_HAS_RGB_LED 1
#define BOROVIK_DISPLAY_SMOKE_TEST 0
#define BOROVIK_PIN_BEACON_TEST 0

#define BTN_A_PIN  15
#define BTN_B_PIN  14
#define ENC_CLK    4
#define ENC_DT     5
#define ENC_SW     6

#define GPS_RX     18
#define GPS_TX     17
#define GPS_BAUD   9600

#define TFT_SCLK   12
#define TFT_MOSI   11
#define TFT_MISO   -1
#define TFT_CS     8
#define TFT_DC     9
#define TFT_RST    10
#define TFT_BLK    3
#define TFT_SPI_HOST FSPI

#define SD_SCLK    TFT_SCLK
#define SD_MOSI    TFT_MOSI
#define SD_MISO    13
#define SD_CS      7

#else
#error "Select a BOROVIK_BOARD_* profile in platformio.ini build_flags"
#endif

#define TFT_WIDTH   320
#define TFT_HEIGHT  240
