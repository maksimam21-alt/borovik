# BOROVIK: Freenove ESP32-Wrover CAM migration

Current practical target: do not move all hardware at once. First make a working MVP on the Freenove ESP32-Wrover CAM:

1. OV2640 camera works.
2. GPS NEO-6M is read over UART.
3. ST7789 shows status.
4. MicroSD saves files.
5. One button triggers the capture/save flow.

## Build environments

- Old test stand: `pio run -e esp32-s3-n16r8`
- Freenove MVP: `pio run -e freenove-wrover-cam-mvp`

Board-specific pins live in `firmware/src/board_config.h`.

## Freenove MVP pin plan

| Signal | GPIO | Notes |
| --- | ---: | --- |
| GPS RX | 32 | NEO-6M TX -> ESP32 RX |
| GPS TX | 33 | Optional, ESP32 TX -> NEO-6M RX |
| TFT SCK | 14 | SPI clock, tested with hardware SPI |
| TFT MOSI | 13 | SPI data to display |
| TFT MISO | -1 | Not used by ST7789 |
| TFT CS | 15 | Tested: must be controlled by ESP32 for fast hardware SPI |
| TFT DC | 12 | Tested; moved off IO2 because IO2 has onboard LED |
| TFT RST | 2 | Tested: display needs a real reset pulse |
| TFT BLK | -1 | Tie display backlight to 3V3 for MVP |
| Button A | 0 | Use carefully: BOOT/strapping pin |
| SD CS | TBD | Needs a free CS; choose after testing display + GPS |

## Do not use while camera is active

OV2640 uses GPIO `4, 5, 18, 19, 21, 22, 23, 25, 26, 27, 34, 35, 36, 39`.

This means:

- Old encoder pins GPIO4/5 conflict with the camera.
- Old GPS GPIO17/18 is not suitable: GPIO18 conflicts with the camera.
- Old MPU GPIO21/22 conflicts with the camera.

## Migration steps

### Step 1: Board profile

Done:

- Added `board_config.h`.
- Added `freenove-wrover-cam-mvp` PlatformIO env.
- Kept old `esp32-s3-n16r8` env working.
- Disabled encoder and second button for Freenove MVP.

### Step 2: Hardware smoke tests

Done:

- Flash `freenove-wrover-cam-mvp`.
- Check Serial Monitor prints `Borovik v0.2 on Freenove ESP32-Wrover CAM MVP`.
- Check GPS on GPIO32/33.
- Check Button A on GPIO0 -> GND. Do not hold it during boot/reset.
- Check ST7789 with the final tested wiring below.
- Confirm fast screen switching with hardware SPI and controlled CS.

Final tested ST7789 wiring:

| ST7789 pin | Freenove pin | Notes |
| --- | --- | --- |
| VCC / 3V3 | 3V3 | Use 3.3V |
| GND | GND | Common ground |
| CS | IO15 | Required for fast hardware SPI; `CS -> GND` only worked with slow software SPI |
| RESET / RES | IO2 | Required; tying reset to 3V3 was unreliable |
| DC | IO12 | Command/data |
| SDI / MOSI | IO13 | Display data input |
| SCK | IO14 | SPI clock |
| LED / BLK | 3V3 | Backlight always on |

If the display stays white, first check `RESET`, `DC`, `SDI`, `SCK`, and `CS` continuity at the display pins.

### Step 3: Camera

- Add `esp_camera.h`.
- Configure `CAMERA_MODEL_WROVER_KIT`.
- Start with still capture only, no streaming.
- Show camera init status on Serial and display.

### Step 4: MicroSD

- Add `SD.h`.
- Share SPI with ST7789: SCK=14, MOSI=13.
- Pick a safe MISO pin; GPIO12 is already used by TFT DC in the working display wiring.
- Pick and test `SD_CS`.
- Save a small text file first.

### Step 5: Capture record

On button press:

- Capture JPEG.
- Read latest valid GPS fix.
- Save `IMG_XXXX.jpg`.
- Save `IMG_XXXX.json` with lat/lon/time/satellites/hdop.

### Later

- Restore encoder and extra buttons via MCP23017 or PCF8574.
- Restore MPU-9250 only after I2C pins are confirmed.
- Consider an ESP32-S3 camera board if Freenove GPIO limits become too painful.
