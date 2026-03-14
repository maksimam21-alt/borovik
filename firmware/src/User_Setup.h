// ============================================================
// Конфиг TFT_eSPI для дисплея ST7789 2.4" 320x240
// ESP32-S3-DevKitC-1
// ============================================================

// Выбираем драйвер дисплея
#define ST7789_DRIVER

// Размер дисплея
#define TFT_WIDTH  240
#define TFT_HEIGHT 320

// Пины SPI (ESP32-S3)
#define TFT_MOSI  11   // SDA  — данные
#define TFT_SCLK  12   // SCL  — тактирование
#define TFT_CS     8   // CS   — выбор чипа
#define TFT_DC     9   // DC   — данные/команда
#define TFT_RST   10   // RES  — сброс
#define TFT_BL    -1   // подсветка: -1 = всегда включена (через 3.3V)

// Частота SPI (27 МГц — стабильно для ST7789)
#define SPI_FREQUENCY  27000000

// Цветовой порядок (попробуй TFT_BGR если цвета перепутаны)
#define TFT_RGB_ORDER TFT_RGB

// Шрифты
#define LOAD_GLCD   // Font 1. Original Adafruit 8 pixel font
#define LOAD_FONT2  // Font 2. Small 16 pixel high font
#define LOAD_FONT4  // Font 4. Medium 26 pixel high font
#define LOAD_FONT6  // Font 6. Large 48 pixel font
#define LOAD_FONT7  // Font 7. 7 segment 48 pixel font
#define LOAD_FONT8  // Font 8. Large 75 pixel font
#define LOAD_GFXFF  // FreeFonts — много красивых шрифтов
#define SMOOTH_FONT // Сглаженные шрифты (нужен microSD или SPIFFS)
