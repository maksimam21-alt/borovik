# 🍄 Borovik

> Smart offline gadget for mushroom hunters.

**Photo a mushroom → ESP32-S3 identifies the species offline → GPS point saved to map automatically → collection grows itself.**

Works without internet. One hand, gloves, bright sun. Fits in a pocket.

---

## Architecture

`
borovik/
├── firmware/        # ESP32-S3 firmware (Arduino / PlatformIO)
└── android/         # Android app (Kotlin + OSMdroid + TFLite)
`

**Gadget hardware:**
- MCU: ESP32-S3-DevKitC-1 N16R8
- GPS: u-blox NEO-6M (1–2 m accuracy, offline)
- Display: 2.4" TFT LCD 320×240 ST7789 SPI (touch)
- Camera: OV2640 / OV5640
- IMU/Compass: MPU-9250
- Storage: microSD SPI
- Battery: Li-Po 2000 mAh JST-PH 2.0 (~7–9 h)
- Charging: TP4056 Type-C with protection
- Controls: KY-040 encoder + 2 tactile buttons
- Case: TPU+PC, IP67, ~7×7×1.5 cm

**Android app stack:**
- Kotlin, OSMdroid 6.1.18, Room 2.6.1
- CameraX 1.3.1, TFLite 2.14.0
- Navigation Component 2.7.6, Coroutines 1.7.3

---

## Roadmap

| Phase | Name | Status |
|-------|------|--------|
| 0 | Setup — IDE, first blink, emulator | 🔄 In progress |
| 1 | Breadboard prototype | ⏳ Pending |
| 2 | Firmware — GPS + camera + display + ML | ⏳ Pending |
| 3 | Android app — map, collection, history | ⏳ Pending |
| 4 | Gadget ↔ App sync (BT/WiFi) | ⏳ Pending |
| 5 | Case + final assembly | ⏳ Pending |
| 6 | Polish — gamification, UX tests | ⏳ Pending |

---

## Quick start

### Firmware
`ash
# Install PlatformIO, then:
cd firmware
pio run --target upload
`

### Android
`ash
cd android
./gradlew assembleDebug
`

---

## Project values

- **Offline first** — everything works in the forest without signal
- **One hand** — gloves, sunlight, button at the bottom center
- **Automation** — collection updates itself, no manual input
- **Accuracy** — GPS + species ID, not just a photo
- **Compact** — fits in a pocket, doesn't get in the way

---

*Project name: Borovik (боровик) — Russian for porcini mushroom, king of the forest.*