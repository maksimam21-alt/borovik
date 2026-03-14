#include <Arduino.h>
#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();  // Создаём объект дисплея

void setup() {
  Serial.begin(115200);
  Serial.println("Borovik display test");

  // Инициализация дисплея
  tft.init();
  tft.setRotation(0);  // 0=portrait, 1=landscape, 2=portrait flip, 3=landscape flip

  // === ТЕСТ 1: Заливка цветом ===
  // Проверяет что дисплей вообще работает
  tft.fillScreen(TFT_BLACK);
  delay(300);
  tft.fillScreen(TFT_RED);
  delay(300);
  tft.fillScreen(TFT_GREEN);
  delay(300);
  tft.fillScreen(TFT_BLUE);
  delay(300);
  tft.fillScreen(TFT_BLACK);

  // === ТЕСТ 2: Текст ===
  tft.setTextColor(TFT_WHITE, TFT_BLACK);  // цвет текста, фон
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.println("BOROVIK v0.1");

  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setTextSize(1);
  tft.setCursor(10, 40);
  tft.println("ESP32-S3 + ST7789 2.4\"");
  tft.setCursor(10, 55);
  tft.println("Display: OK");

  // === ТЕСТ 3: Геометрия ===
  // Прямоугольник
  tft.drawRect(10, 80, 100, 50, TFT_YELLOW);
  tft.fillRect(15, 85, 90, 40, TFT_DARKGREY);

  // Круг
  tft.fillCircle(180, 105, 30, TFT_CYAN);
  tft.drawCircle(180, 105, 30, TFT_WHITE);

  // Линия
  tft.drawLine(10, 145, 230, 145, TFT_RED);

  // === ТЕСТ 4: Боровик-заглушка (текстовый "гриб") ===
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.setCursor(10, 160);
  tft.println("Найден гриб:");
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(10, 175);
  tft.println("Borovik");
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setTextSize(1);
  tft.setCursor(10, 200);
  tft.println("Lat: 60.1699");
  tft.setCursor(10, 215);
  tft.println("Lon: 24.9384");
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setCursor(10, 235);
  tft.println("Confidence: 94%");

  Serial.println("Display test done!");
}

void loop() {
  // Мигаем рамкой чтобы видеть что цикл работает
  tft.drawRect(5, 5, 230, 310, TFT_WHITE);
  delay(500);
  tft.drawRect(5, 5, 230, 310, TFT_BLACK);
  delay(500);
}
