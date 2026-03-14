#include <Arduino.h>
#include <SPI.h>

// Пины
#define TFT_SCLK 12
#define TFT_MOSI 11
#define TFT_CS   8
#define TFT_DC   9
#define TFT_RST  10

#define TFT_WIDTH  170
#define TFT_HEIGHT 320

SPIClass spi(HSPI);

void tft_cmd(uint8_t cmd) {
  digitalWrite(TFT_DC, LOW);
  digitalWrite(TFT_CS, LOW);
  spi.transfer(cmd);
  digitalWrite(TFT_CS, HIGH);
}

void tft_data(uint8_t dat) {
  digitalWrite(TFT_DC, HIGH);
  digitalWrite(TFT_CS, LOW);
  spi.transfer(dat);
  digitalWrite(TFT_CS, HIGH);
}

void tft_init() {
  pinMode(TFT_CS,  OUTPUT); digitalWrite(TFT_CS, HIGH);
  pinMode(TFT_DC,  OUTPUT); digitalWrite(TFT_DC, HIGH);
  pinMode(TFT_RST, OUTPUT);

  // Hard reset
  digitalWrite(TFT_RST, HIGH); delay(50);
  digitalWrite(TFT_RST, LOW);  delay(20);
  digitalWrite(TFT_RST, HIGH); delay(150);

  // Software reset
  tft_cmd(0x01); delay(150);
  // Sleep out
  tft_cmd(0x11); delay(255);

  // Colour mode: 16-bit
  tft_cmd(0x3A); tft_data(0x55);

  // MADCTL: RGB, top-left origin
  tft_cmd(0x36); tft_data(0x00);

  // Column addr set: 35..204 (170px centred in 240px controller)
  tft_cmd(0x2A);
  tft_data(0x00); tft_data(35);
  tft_data(0x00); tft_data(35 + TFT_WIDTH - 1);

  // Row addr set: 0..319
  tft_cmd(0x2B);
  tft_data(0x00); tft_data(0x00);
  tft_data(0x01); tft_data(0x3F);

  // Inversion ON (needed for most ST7789 1.9" modules)
  tft_cmd(0x21);

  // Display ON
  tft_cmd(0x29); delay(50);
}

void tft_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
  x0 += 35; x1 += 35;  // col offset
  tft_cmd(0x2A);
  tft_data(x0 >> 8); tft_data(x0 & 0xFF);
  tft_data(x1 >> 8); tft_data(x1 & 0xFF);
  tft_cmd(0x2B);
  tft_data(y0 >> 8); tft_data(y0 & 0xFF);
  tft_data(y1 >> 8); tft_data(y1 & 0xFF);
  tft_cmd(0x2C);
}

void tft_fill(uint16_t color) {
  tft_set_window(0, 0, TFT_WIDTH - 1, TFT_HEIGHT - 1);
  digitalWrite(TFT_DC, HIGH);
  digitalWrite(TFT_CS, LOW);
  uint32_t total = (uint32_t)TFT_WIDTH * TFT_HEIGHT;
  for (uint32_t i = 0; i < total; i++) {
    spi.transfer16(color);
  }
  digitalWrite(TFT_CS, HIGH);
}

// RGB565 colors
#define C_BLACK  0x0000
#define C_RED    0xF800
#define C_GREEN  0x07E0
#define C_BLUE   0x001F
#define C_WHITE  0xFFFF
#define C_CYAN   0x07FF
#define C_YELLOW 0xFFE0

void setRGB(uint8_t r, uint8_t g, uint8_t b) {
  neopixelWrite(48, r, g, b);
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("Raw SPI ST7789 1.9 test");

  setRGB(20, 0, 0); delay(200);
  setRGB(0, 0, 0);

  spi.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);
  spi.setFrequency(27000000);

  tft_init();
  Serial.println("Init done");

  tft_fill(C_RED);   setRGB(20,0,0); delay(500); Serial.println("RED");
  tft_fill(C_GREEN); setRGB(0,20,0); delay(500); Serial.println("GREEN");
  tft_fill(C_BLUE);  setRGB(0,0,20); delay(500); Serial.println("BLUE");
  tft_fill(C_WHITE); setRGB(10,10,10); delay(500); Serial.println("WHITE");
  tft_fill(C_BLACK); setRGB(0,0,0);

  Serial.println("Done!");
}

void loop() {
  setRGB(0, 8, 0); delay(500);
  setRGB(0, 0, 0); delay(500);
  Serial.println("ALIVE");
}
