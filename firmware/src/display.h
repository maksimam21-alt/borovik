#pragma once
#include <Arduino.h>
#include <SPI.h>
#include <lvgl.h>
#include "board_config.h"

// ===== ST7789 2.4" 320x240 =====
// Дисплей (FSPI)
#ifndef TFT_SCLK
#define TFT_SCLK  12   // SCL
#endif
#ifndef TFT_MOSI
#define TFT_MOSI  11   // SDA
#endif
#ifndef TFT_CS
#define TFT_CS     8   // CS
#endif
#ifndef TFT_DC
#define TFT_DC     9   // DC
#endif
#ifndef TFT_RST
#define TFT_RST   10   // RES
#endif
#ifndef TFT_BLK
#define TFT_BLK    3   // BLK
#endif

#ifndef TFT_WIDTH
#define TFT_WIDTH  320
#endif
#ifndef TFT_HEIGHT
#define TFT_HEIGHT 240
#endif
// 2.4" 320x240 — col_offset = 0, row_offset = 0

static SPIClass tft_spi(TFT_SPI_HOST);

#if defined(BOROVIK_BOARD_FREENOVE_WROVER_CAM_MVP)
#define TFT_SOFTWARE_SPI 0
#else
#define TFT_SOFTWARE_SPI 0
#endif

static void tft_select() {
  if (TFT_CS >= 0) digitalWrite(TFT_CS, LOW);
}

static void tft_unselect() {
  if (TFT_CS >= 0) digitalWrite(TFT_CS, HIGH);
}

static void tft_cmd(uint8_t cmd) {
  digitalWrite(TFT_DC, LOW);
  tft_select();
#if TFT_SOFTWARE_SPI
  for (uint8_t mask = 0x80; mask; mask >>= 1) {
    digitalWrite(TFT_SCLK, LOW);
    digitalWrite(TFT_MOSI, (cmd & mask) ? HIGH : LOW);
    digitalWrite(TFT_SCLK, HIGH);
  }
#else
  tft_spi.transfer(cmd);
#endif
  tft_unselect();
}

static void tft_data(uint8_t dat) {
  digitalWrite(TFT_DC, HIGH);
  tft_select();
#if TFT_SOFTWARE_SPI
  for (uint8_t mask = 0x80; mask; mask >>= 1) {
    digitalWrite(TFT_SCLK, LOW);
    digitalWrite(TFT_MOSI, (dat & mask) ? HIGH : LOW);
    digitalWrite(TFT_SCLK, HIGH);
  }
#else
  tft_spi.transfer(dat);
#endif
  tft_unselect();
}

static void tft_data16(uint16_t dat) {
  tft_data(dat >> 8);
  tft_data(dat & 0xFF);
}

static void tft_data_bytes(const uint8_t *data, uint32_t len) {
  digitalWrite(TFT_DC, HIGH);
  tft_select();
  for (uint32_t i = 0; i < len; i++) {
#if TFT_SOFTWARE_SPI
    uint8_t dat = data[i];
    for (uint8_t mask = 0x80; mask; mask >>= 1) {
      digitalWrite(TFT_SCLK, LOW);
      digitalWrite(TFT_MOSI, (dat & mask) ? HIGH : LOW);
      digitalWrite(TFT_SCLK, HIGH);
    }
#else
    tft_spi.transfer(data[i]);
#endif
  }
  tft_unselect();
}

static void tft_fill_raw(uint16_t color) {
  tft_cmd(0x2A);
  tft_data(0x00); tft_data(0x00);
  tft_data((TFT_WIDTH - 1) >> 8); tft_data((TFT_WIDTH - 1) & 0xFF);

  tft_cmd(0x2B);
  tft_data(0x00); tft_data(0x00);
  tft_data((TFT_HEIGHT - 1) >> 8); tft_data((TFT_HEIGHT - 1) & 0xFF);

  tft_cmd(0x2C);
#if TFT_SOFTWARE_SPI
  for (uint32_t i = 0; i < (uint32_t)TFT_WIDTH * TFT_HEIGHT; i++) {
    tft_data16(color);
  }
#else
  tft_spi.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
  digitalWrite(TFT_DC, HIGH);
  tft_select();
  for (uint32_t i = 0; i < (uint32_t)TFT_WIDTH * TFT_HEIGHT; i++) {
    tft_spi.transfer(color >> 8);
    tft_spi.transfer(color & 0xFF);
  }
  tft_unselect();
  tft_spi.endTransaction();
#endif
}

static void tft_init_hw() {
  if (TFT_CS >= 0) {
    pinMode(TFT_CS, OUTPUT);
    digitalWrite(TFT_CS, HIGH);
  }
  pinMode(TFT_DC,  OUTPUT); digitalWrite(TFT_DC, HIGH);
  if (TFT_RST >= 0) pinMode(TFT_RST, OUTPUT);
  if (TFT_BLK >= 0) {
    pinMode(TFT_BLK, OUTPUT);
    digitalWrite(TFT_BLK, HIGH); // подсветка включена сразу
  }
  pinMode(TFT_SCLK, OUTPUT);
  pinMode(TFT_MOSI, OUTPUT);
  digitalWrite(TFT_SCLK, HIGH);
  digitalWrite(TFT_MOSI, LOW);
  delay(10);

  // Аппаратный сброс
  if (TFT_RST >= 0) {
    digitalWrite(TFT_RST, HIGH); delay(50);
    digitalWrite(TFT_RST, LOW);  delay(20);
    digitalWrite(TFT_RST, HIGH); delay(150);
  }

#if TFT_SOFTWARE_SPI
  (void)tft_spi;
#else
  tft_spi.begin(TFT_SCLK, TFT_MISO, TFT_MOSI, TFT_CS >= 0 ? TFT_CS : -1);
  tft_spi.setFrequency(8000000);
#endif

  // ST7789 init sequence для 320x240
  tft_cmd(0x01); delay(150);   // Software reset
  tft_cmd(0x11); delay(255);   // Sleep out
  tft_cmd(0x13); delay(10);    // Normal display mode on

  tft_cmd(0x3A); tft_data(0x55); // Pixel format RGB565

  // MADCTL: landscape 320x240, MV+MX+BGR
  tft_cmd(0x36); tft_data(0xA0);

  // Column address: 0..319
  tft_cmd(0x2A);
  tft_data(0x00); tft_data(0x00);
  tft_data(0x01); tft_data(0x3F);

  // Row address: 0..239
  tft_cmd(0x2B);
  tft_data(0x00); tft_data(0x00);
  tft_data(0x00); tft_data(0xEF);

  // Inversion OFF for this ST7789 module.
  tft_cmd(0x20);
  tft_cmd(0x29); delay(50); // Display ON
  tft_fill_raw(0x0000);
}

static void lv_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_p) {
  uint16_t x0 = area->x1, x1 = area->x2;
  uint16_t y0 = area->y1, y1 = area->y2;

  tft_cmd(0x2A);
  tft_data(x0 >> 8); tft_data(x0 & 0xFF);
  tft_data(x1 >> 8); tft_data(x1 & 0xFF);

  tft_cmd(0x2B);
  tft_data(y0 >> 8); tft_data(y0 & 0xFF);
  tft_data(y1 >> 8); tft_data(y1 & 0xFF);

  tft_cmd(0x2C);

  uint32_t len = (uint32_t)(area->x2 - area->x1 + 1) * (area->y2 - area->y1 + 1);
#if TFT_SOFTWARE_SPI
  tft_data_bytes((const uint8_t*)color_p, len * 2);
#else
  tft_spi.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
  digitalWrite(TFT_DC, HIGH);
  tft_select();
  tft_spi.transferBytes((uint8_t*)color_p, nullptr, len * 2);
  tft_unselect();
  tft_spi.endTransaction();
#endif
  lv_disp_flush_ready(drv);
}

// Аппаратный таймер тика LVGL (1 мс)
static hw_timer_t *lvgl_timer_hw = nullptr;
void IRAM_ATTR lvgl_tick_isr() { lv_tick_inc(1); }

// Буфер — 320*10 пикселей (2 строки на 320px)
static lv_disp_drv_t     disp_drv;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf1[TFT_WIDTH * 10] __attribute__((aligned(4)));

void display_init() {
  tft_init_hw();

  lv_init();

  lvgl_timer_hw = timerBegin(0, 80, true);
  timerAttachInterrupt(lvgl_timer_hw, &lvgl_tick_isr, true);
  timerAlarmWrite(lvgl_timer_hw, 1000, true);
  timerAlarmEnable(lvgl_timer_hw);

  lv_disp_draw_buf_init(&draw_buf, buf1, nullptr, TFT_WIDTH * 10);

  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res  = TFT_WIDTH;
  disp_drv.ver_res  = TFT_HEIGHT;
  disp_drv.flush_cb = lv_flush_cb;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);
}
