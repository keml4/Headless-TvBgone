# Headless TV-B-Gone

ESP32 Universal TV closing firmware with separate North America/Asia and Europe firmwares.

## Hardware

- ESP32 Dev Module
- IR LED on GPIO 4

### Flash at https://keml4.github.io/Headless-TvBgone/.

Pick NA/Asia or Europe.

Thanks to Aryann019x for fixing Web flasher

## Flashing with Arduino IDE (manually)

1. Install the ESP32 board package.
2. Install the **IRremoteESP8266** library by `crankyoldgit`.
3. Open one sketch:
   - `NA-Tv-B-Gone/NA-Tv-B-Gone.ino`
   - `EU-Tv-B-Gone/EU-Tv-B-Gone.ino`
4. Select **ESP32 Dev Module** and your serial port.
5. Upload.

## NOW WITH BW16 SUPPORT
