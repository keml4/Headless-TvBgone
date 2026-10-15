# Headless TV-B-GONE

Universal TV closing firmware with separate North America/Asia and Europe firmwares.

## Hardware

- Microcontroller of your choice
- IR LED on GPIO 4 (GPIO 2 on esp8266)

### Flash at https://keml4.github.io/Headless-TvBgone/.
(flasher only has 
ESP32	
ESP32-S2	
ESP32-S3	
ESP32-C3	
ESP32-C6	
ESP32-H2	
ESP8266 boards)

Pick NA/Asia or Europe.

Thanks to Aryann019x for fixing Web flasher

## Flashing with Arduino IDE (manually)

1. Install the board package.
2. Install the **IRremoteESP8266** library by `crankyoldgit`.
3. Open one sketch:
   - `NA-Tv-B-Gone/NA-Tv-B-Gone.ino`
   - `EU-Tv-B-Gone/EU-Tv-B-Gone.ino`
4. Select **ESP32 Dev Module** and your serial port.
5. Upload.

## NOW WITH BW16 SUPPORT
