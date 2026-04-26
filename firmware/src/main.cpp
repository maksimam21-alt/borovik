#include <Arduino.h>
#include <lvgl.h>
#include <TinyGPSPlus.h>
#include "board_config.h"
#include "display.h"

LV_FONT_DECLARE(mont_ru_12);
LV_FONT_DECLARE(mont_ru_14);
LV_FONT_DECLARE(mont_ru_16);
LV_FONT_DECLARE(mont_ru_20);

// ===== Пины =====
#ifndef BTN_A_PIN
#define BTN_A_PIN   15   // Красная кнопка — фото (в будущем)
#endif
#ifndef BTN_B_PIN
#define BTN_B_PIN   14   // Чёрная кнопка — назад
#endif
#ifndef ENC_CLK
#define ENC_CLK     4    // Энкодер CLK (прерывание)
#endif
#ifndef ENC_DT
#define ENC_DT      5    // Энкодер DT
#endif
#ifndef ENC_SW
#define ENC_SW      6    // Энкодер кнопка
#endif
#ifndef GPS_RX
#define GPS_RX      18
#endif
#ifndef GPS_TX
#define GPS_TX      17
#endif
#ifndef GPS_BAUD
#define GPS_BAUD    9600
#endif

TinyGPSPlus gps;

// ===== Экраны =====
static int current_screen = 0;
static const int SCREEN_COUNT = 3;
static lv_obj_t *screens[SCREEN_COUNT] = {nullptr, nullptr, nullptr};

// ===== Энкодер =====
volatile int encoder_delta = 0;   // +1 или -1 от прерывания
volatile int last_clk = HIGH;

#if BOROVIK_HAS_ENCODER
void IRAM_ATTR encoder_isr() {
  int clk = digitalRead(ENC_CLK);
  int dt  = digitalRead(ENC_DT);
  // Считаем только по FALLING фронту (HIGH→LOW) — один щелчок = один импульс
  if (last_clk == HIGH && clk == LOW) {
    encoder_delta += (dt == LOW) ? +1 : -1;
  }
  last_clk = clk;
}
#endif

// ===== Кнопки =====
// Кнопка A (красная, GPIO15) — пока переключение, потом фото
int btnA_state = HIGH, btnA_last = HIGH;
unsigned long btnA_debounce = 0;
unsigned long btnA_last_press = 0;

// Кнопка B (чёрная, GPIO14) — назад
int btnB_state = HIGH, btnB_last = HIGH;
unsigned long btnB_debounce = 0;

// Энкодер SW (GPIO6) — подтвердить
int encSW_state = HIGH, encSW_last = HIGH;
unsigned long encSW_debounce = 0;

void setRGB(uint8_t r, uint8_t g, uint8_t b) {
#if BOROVIK_HAS_RGB_LED
  neopixelWrite(48, r, g, b);
#else
  (void)r;
  (void)g;
  (void)b;
#endif
}

// ===== Глобальные метки для обновления данных GPS =====
// Экран 2 (GPS)
lv_obj_t *lbl_lat        = nullptr;
lv_obj_t *lbl_lon        = nullptr;
lv_obj_t *lbl_sat        = nullptr;
lv_obj_t *lbl_gps_status = nullptr;
// Экран 3 (отладка)
lv_obj_t *lbl_dbg_raw    = nullptr;
lv_obj_t *lbl_dbg_speed  = nullptr;
lv_obj_t *lbl_dbg_alt    = nullptr;
lv_obj_t *lbl_dbg_hdop   = nullptr;
lv_obj_t *lbl_dbg_age    = nullptr;
lv_obj_t *lbl_dbg_chars  = nullptr;

// ===== Экран 1: Главный =====
lv_obj_t* screen_home_create() {
  lv_obj_t *scr = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(scr, lv_color_hex(0x0D1117), 0);
  lv_obj_set_style_border_width(scr, 0, 0);

  lv_obj_t *title = lv_label_create(scr);
  lv_label_set_text(title, "БОРОВИК");
  lv_obj_set_style_text_font(title, &mont_ru_20, 0);
  lv_obj_set_style_text_color(title, lv_color_hex(0x3FB950), 0);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);

  lv_obj_t *ver = lv_label_create(scr);
  lv_label_set_text(ver, "v0.2");
  lv_obj_set_style_text_font(ver, &mont_ru_12, 0);
  lv_obj_set_style_text_color(ver, lv_color_hex(0x484F58), 0);
  lv_obj_align(ver, LV_ALIGN_TOP_MID, 0, 46);

  lv_obj_t *icon = lv_label_create(scr);
  lv_label_set_text(icon, "СКАНИРОВАТЬ");
  lv_obj_set_style_text_font(icon, &mont_ru_16, 0);
  lv_obj_set_style_text_color(icon, lv_color_hex(0x3FB950), 0);
  lv_obj_align(icon, LV_ALIGN_CENTER, 0, -20);

  lv_obj_t *hint = lv_label_create(scr);
  lv_label_set_text(hint, "Крути энкодер \xe2\x86\x92 меню");
  lv_obj_set_style_text_font(hint, &mont_ru_12, 0);
  lv_obj_set_style_text_color(hint, lv_color_hex(0x8B949E), 0);
  lv_obj_align(hint, LV_ALIGN_CENTER, 0, 15);

  lv_obj_t *bar = lv_obj_create(scr);
  lv_obj_set_size(bar, 320, 24);
  lv_obj_set_style_bg_color(bar, lv_color_hex(0x161B22), 0);
  lv_obj_set_style_border_width(bar, 0, 0);
  lv_obj_set_style_radius(bar, 0, 0);
  lv_obj_align(bar, LV_ALIGN_BOTTOM_MID, 0, 0);

  lv_obj_t *pg = lv_label_create(bar);
  lv_label_set_text(pg, "1 / 3");
  lv_obj_set_style_text_font(pg, &mont_ru_12, 0);
  lv_obj_set_style_text_color(pg, lv_color_hex(0x484F58), 0);
  lv_obj_align(pg, LV_ALIGN_RIGHT_MID, -8, 0);
  return scr;
}

// ===== Экран 2: GPS данные =====
lv_obj_t* screen_gps_create() {
  lv_obj_t *scr = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(scr, lv_color_hex(0x0D1117), 0);
  lv_obj_set_style_border_width(scr, 0, 0);

  // Заголовок
  lv_obj_t *title = lv_label_create(scr);
  lv_label_set_text(title, "GPS");
  lv_obj_set_style_text_font(title, &mont_ru_20, 0);
  lv_obj_set_style_text_color(title, lv_color_hex(0x58A6FF), 0);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 8);

  // --- Левая колонка: Спутники ---
  lv_obj_t *sl = lv_label_create(scr);
  lv_label_set_text(sl, "Спутники");
  lv_obj_set_style_text_font(sl, &mont_ru_12, 0);
  lv_obj_set_style_text_color(sl, lv_color_hex(0x484F58), 0);
  lv_obj_align(sl, LV_ALIGN_TOP_LEFT, 20, 50);

  lbl_sat = lv_label_create(scr);
  lv_label_set_text(lbl_sat, "--");
  lv_obj_set_style_text_font(lbl_sat, &mont_ru_20, 0);
  lv_obj_set_style_text_color(lbl_sat, lv_color_hex(0x58A6FF), 0);
  lv_obj_align(lbl_sat, LV_ALIGN_TOP_LEFT, 20, 68);

  // --- Правая колонка: Статус ---
  lbl_gps_status = lv_label_create(scr);
  lv_label_set_text(lbl_gps_status, "Ожидание...");
  lv_obj_set_style_text_font(lbl_gps_status, &mont_ru_12, 0);
  lv_obj_set_style_text_color(lbl_gps_status, lv_color_hex(0xF0883E), 0);
  lv_obj_align(lbl_gps_status, LV_ALIGN_TOP_RIGHT, -20, 68);

  // --- Широта ---
  lv_obj_t *latl = lv_label_create(scr);
  lv_label_set_text(latl, "Широта");
  lv_obj_set_style_text_font(latl, &mont_ru_12, 0);
  lv_obj_set_style_text_color(latl, lv_color_hex(0x484F58), 0);
  lv_obj_align(latl, LV_ALIGN_TOP_LEFT, 20, 118);

  lbl_lat = lv_label_create(scr);
  lv_label_set_text(lbl_lat, "---.------");
  lv_obj_set_style_text_font(lbl_lat, &mont_ru_14, 0);
  lv_obj_set_style_text_color(lbl_lat, lv_color_hex(0xE6EDF3), 0);
  lv_obj_align(lbl_lat, LV_ALIGN_TOP_LEFT, 20, 134);

  // --- Долгота ---
  lv_obj_t *lonl = lv_label_create(scr);
  lv_label_set_text(lonl, "Долгота");
  lv_obj_set_style_text_font(lonl, &mont_ru_12, 0);
  lv_obj_set_style_text_color(lonl, lv_color_hex(0x484F58), 0);
  lv_obj_align(lonl, LV_ALIGN_TOP_RIGHT, -20, 118);

  lbl_lon = lv_label_create(scr);
  lv_label_set_text(lbl_lon, "---.------");
  lv_obj_set_style_text_font(lbl_lon, &mont_ru_14, 0);
  lv_obj_set_style_text_color(lbl_lon, lv_color_hex(0xE6EDF3), 0);
  lv_obj_align(lbl_lon, LV_ALIGN_TOP_RIGHT, -20, 134);

  // Бар навигации
  lv_obj_t *bar = lv_obj_create(scr);
  lv_obj_set_size(bar, 320, 24);
  lv_obj_set_style_bg_color(bar, lv_color_hex(0x161B22), 0);
  lv_obj_set_style_border_width(bar, 0, 0);
  lv_obj_set_style_radius(bar, 0, 0);
  lv_obj_align(bar, LV_ALIGN_BOTTOM_MID, 0, 0);
  lv_obj_t *pg = lv_label_create(bar);
  lv_label_set_text(pg, "2 / 3");
  lv_obj_set_style_text_font(pg, &mont_ru_12, 0);
  lv_obj_set_style_text_color(pg, lv_color_hex(0x484F58), 0);
  lv_obj_align(pg, LV_ALIGN_RIGHT_MID, -8, 0);
  return scr;
}

// ===== Экран 3: Отладка GPS (320x240, 2 колонки) =====
static void dbg_row_left(lv_obj_t *scr, const char *name, lv_obj_t **val_out, const char *def, int x, int y) {
  lv_obj_t *lbl = lv_label_create(scr);
  lv_label_set_text(lbl, name);
  lv_obj_set_style_text_font(lbl, &mont_ru_12, 0);
  lv_obj_set_style_text_color(lbl, lv_color_hex(0x484F58), 0);
  lv_obj_align(lbl, LV_ALIGN_TOP_LEFT, x, y);
  *val_out = lv_label_create(scr);
  lv_label_set_text(*val_out, def);
  lv_obj_set_style_text_font(*val_out, &mont_ru_12, 0);
  lv_obj_set_style_text_color(*val_out, lv_color_hex(0x3FB950), 0);
  lv_obj_align(*val_out, LV_ALIGN_TOP_LEFT, x, y + 16);
}

lv_obj_t* screen_debug_create() {
  lv_obj_t *scr = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(scr, lv_color_hex(0x0D1117), 0);
  lv_obj_set_style_border_width(scr, 0, 0);

  lv_obj_t *title = lv_label_create(scr);
  lv_label_set_text(title, "GPS DEBUG");
  lv_obj_set_style_text_font(title, &mont_ru_14, 0);
  lv_obj_set_style_text_color(title, lv_color_hex(0xF0883E), 0);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 8);

  // Левая колонка: NMEA, Скорость, Высота
  dbg_row_left(scr, "NMEA:",     &lbl_dbg_raw,   "--",       16,  38);
  dbg_row_left(scr, "Скорость:", &lbl_dbg_speed, "-- км/ч",  16,  90);
  dbg_row_left(scr, "Высота:",   &lbl_dbg_alt,   "-- м",     16, 142);

  // Правая колонка: HDOP, Возраст, Байт
  dbg_row_left(scr, "HDOP:",     &lbl_dbg_hdop,  "--",      172,  38);
  dbg_row_left(scr, "Возраст:",  &lbl_dbg_age,   "-- мс",   172,  90);
  dbg_row_left(scr, "Байт:",     &lbl_dbg_chars, "--",      172, 142);

  lv_obj_t *bar = lv_obj_create(scr);
  lv_obj_set_size(bar, 320, 24);
  lv_obj_set_style_bg_color(bar, lv_color_hex(0x161B22), 0);
  lv_obj_set_style_border_width(bar, 0, 0);
  lv_obj_set_style_radius(bar, 0, 0);
  lv_obj_align(bar, LV_ALIGN_BOTTOM_MID, 0, 0);
  lv_obj_t *pg = lv_label_create(bar);
  lv_label_set_text(pg, "3 / 3");
  lv_obj_set_style_text_font(pg, &mont_ru_12, 0);
  lv_obj_set_style_text_color(pg, lv_color_hex(0x484F58), 0);
  lv_obj_align(pg, LV_ALIGN_RIGHT_MID, -8, 0);
  return scr;
}

// ===== Обновление GPS данных на экранах =====
void update_gps_ui() {
  if (current_screen == 0) return;
  if (current_screen == 1) {
    if (!lbl_sat || !lbl_lat || !lbl_lon || !lbl_gps_status) return;
  }
  if (current_screen == 2) {
    if (!lbl_dbg_chars) return;
  }
  static char buf[32];

  // Anti-spoof: Шереметьево ~55.97N 37.41E
  bool spoofed = false;
  if (gps.location.isValid()) {
    double dlat = gps.location.lat() - 55.972;
    double dlon = gps.location.lng() - 37.413;
    if ((dlat*dlat + dlon*dlon) < 0.02) spoofed = true;
  }

  if (lbl_sat) {
    snprintf(buf, sizeof(buf), "%d", (int)gps.satellites.value());
    lv_label_set_text(lbl_sat, gps.satellites.isValid() ? buf : "--");
  }
  if (lbl_lat) {
    if (gps.location.isValid() && !spoofed) {
      snprintf(buf, sizeof(buf), "%.6f", gps.location.lat());
      lv_label_set_text(lbl_lat, buf);
    } else lv_label_set_text(lbl_lat, "---.------");
  }
  if (lbl_lon) {
    if (gps.location.isValid() && !spoofed) {
      snprintf(buf, sizeof(buf), "%.6f", gps.location.lng());
      lv_label_set_text(lbl_lon, buf);
    } else lv_label_set_text(lbl_lon, "---.------");
  }
  if (lbl_gps_status) {
    if (spoofed) {
      lv_label_set_text(lbl_gps_status, "SPOOFING!");
      lv_obj_set_style_text_color(lbl_gps_status, lv_color_hex(0xFF4444), 0);
    } else if (gps.location.isValid()) {
      lv_label_set_text(lbl_gps_status, "Сигнал OK!");
      lv_obj_set_style_text_color(lbl_gps_status, lv_color_hex(0x3FB950), 0);
    } else if (gps.satellites.value() > 0) {
      lv_label_set_text(lbl_gps_status, "Захват...");
      lv_obj_set_style_text_color(lbl_gps_status, lv_color_hex(0xF0883E), 0);
    } else {
      lv_label_set_text(lbl_gps_status, "Ожидание...");
      lv_obj_set_style_text_color(lbl_gps_status, lv_color_hex(0xF0883E), 0);
    }
  }
  if (lbl_dbg_speed && gps.speed.isValid()) {
    snprintf(buf, sizeof(buf), "%.1f км/ч", gps.speed.kmph());
    lv_label_set_text(lbl_dbg_speed, buf);
  }
  if (lbl_dbg_alt && gps.altitude.isValid()) {
    snprintf(buf, sizeof(buf), "%.1f м", gps.altitude.meters());
    lv_label_set_text(lbl_dbg_alt, buf);
  }
  if (lbl_dbg_hdop && gps.hdop.isValid()) {
    snprintf(buf, sizeof(buf), "%.2f", gps.hdop.hdop());
    lv_label_set_text(lbl_dbg_hdop, buf);
  }
  if (lbl_dbg_age) {
    snprintf(buf, sizeof(buf), "%lu мс", gps.location.age());
    lv_label_set_text(lbl_dbg_age, buf);
  }
  if (lbl_dbg_chars) {
    snprintf(buf, sizeof(buf), "%lu", gps.charsProcessed());
    lv_label_set_text(lbl_dbg_chars, buf);
  }
}

// ===== Переключение экранов =====
void switch_screen(int direction) {
  current_screen = (current_screen + direction + SCREEN_COUNT) % SCREEN_COUNT;
  lv_scr_load(screens[current_screen]);
  lv_timer_handler();

#if BOROVIK_HAS_RGB_LED
  if (current_screen == 0) {
    setRGB(0, 15, 0); delay(80); setRGB(0, 0, 0);
  } else if (current_screen == 1) {
    setRGB(0, 0, 15);
  } else if (current_screen == 2) {
    setRGB(15, 8, 0);
  }
#endif
}

// ===== Обработка энкодера =====
void check_encoder() {
#if BOROVIK_HAS_ENCODER
  // Читаем накопленный delta из прерывания атомарно
  int delta = 0;
  noInterrupts();
  delta = encoder_delta;
  encoder_delta = 0;
  interrupts();

  // Таймаут 300 мс между переключениями — защита от дребезга энкодера
  static unsigned long lastSwitch = 0;
  if (delta != 0 && (millis() - lastSwitch) > 150) {
    if (delta > 0) switch_screen(+1);
    else           switch_screen(-1);
    lastSwitch = millis();
  }

  // Кнопка энкодера SW — подтвердить (пока просто мигает)
  int reading = digitalRead(ENC_SW);
  if (reading != encSW_last) encSW_debounce = millis();
  if ((millis() - encSW_debounce) > 50 && reading != encSW_state) {
    encSW_state = reading;
    if (encSW_state == LOW) {
      setRGB(10, 10, 0); delay(60); setRGB(0, 0, 0);
      Serial.println("ENC_SW pressed");
    }
  }
  encSW_last = reading;
#endif
}

// ===== Обработка кнопок =====
void check_buttons() {
  // Кнопка A (красная, GPIO15) — пока тоже переключает экран вперёд
  int rA = digitalRead(BTN_A_PIN);
  if (rA != btnA_last) btnA_debounce = millis();
  if ((millis() - btnA_debounce) > 50 && rA != btnA_state) {
    btnA_state = rA;
    if (btnA_state == LOW) {
      if (millis() - btnA_last_press > 450) {
        btnA_last_press = millis();
        switch_screen(+1);
      }
    }
  }
  btnA_last = rA;

#if BOROVIK_HAS_BTN_B
  // Кнопка B (чёрная, GPIO14) — назад
  int rB = digitalRead(BTN_B_PIN);
  if (rB != btnB_last) btnB_debounce = millis();
  if ((millis() - btnB_debounce) > 50 && rB != btnB_state) {
    btnB_state = rB;
    if (btnB_state == LOW) {
      switch_screen(-1);
    }
  }
  btnB_last = rB;
#endif
}

// ===== Setup =====
void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.print("Borovik v0.2 on ");
  Serial.println(BOROVIK_BOARD_NAME);

#if BOROVIK_PIN_BEACON_TEST
  const int testPins[] = {TFT_MOSI, TFT_SCLK, TFT_DC, TFT_RST};
  const char *testNames[] = {"IO13 MOSI", "IO14 SCK", "IO12 DC", "IO2 RESET"};
  for (int i = 0; i < 4; i++) {
    pinMode(testPins[i], OUTPUT);
    digitalWrite(testPins[i], LOW);
  }
  Serial.println("Pin beacon test started");
  while (true) {
    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 4; j++) digitalWrite(testPins[j], LOW);
      digitalWrite(testPins[i], HIGH);
      Serial.print("HIGH: ");
      Serial.println(testNames[i]);
      delay(3000);
    }
  }
#endif

  // Кнопки
  pinMode(BTN_A_PIN, INPUT_PULLUP);
#if BOROVIK_HAS_BTN_B
  pinMode(BTN_B_PIN, INPUT_PULLUP);
#endif

#if BOROVIK_HAS_ENCODER
  // Энкодер
  pinMode(ENC_CLK, INPUT_PULLUP);
  pinMode(ENC_DT,  INPUT_PULLUP);
  pinMode(ENC_SW,  INPUT_PULLUP);
  last_clk = digitalRead(ENC_CLK);
  attachInterrupt(digitalPinToInterrupt(ENC_CLK), encoder_isr, CHANGE);
#endif

#if BOROVIK_HAS_RGB_LED
  setRGB(20, 0, 0); delay(200); setRGB(0, 0, 0);
#endif

  // GPS
  Serial1.begin(GPS_BAUD, SERIAL_8N1, GPS_RX, GPS_TX);
  Serial.println("GPS Serial1 started");

  display_init();
#if BOROVIK_DISPLAY_SMOKE_TEST
  Serial.println("Display smoke test: holding white screen");
  while (true) {
    delay(1000);
  }
#endif
  screens[0] = screen_home_create();
  screens[1] = screen_gps_create();
  screens[2] = screen_debug_create();
  lv_scr_load(screens[0]);
  Serial.println("UI ready");
}

// ===== Loop =====
void loop() {
  // GPS
  while (Serial1.available()) {
    char c = Serial1.read();
    gps.encode(c);
    if (lbl_dbg_raw && c == '\n') {
      lv_label_set_text(lbl_dbg_raw, "OK");
    }
  }

  // Обновляем GPS UI каждые 500 мс
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 500) {
    lastUpdate = millis();
    update_gps_ui();
  }

  lv_timer_handler();
  check_encoder();
  check_buttons();
  delay(5);
}
