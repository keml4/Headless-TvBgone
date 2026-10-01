# Headless TV-B-Gone

Universal TV power-off firmware for ESP32 and ESP8266 with separate North America/Asia and Europe firmwares.

## Hardware

- ESP32 Dev Module, **ESP32-S3**, or ESP8266 (ESP-01/ESP-01S, NodeMCU, D1 mini)
- IR LED: **GPIO 4** on ESP32 / ESP32-S3, **GPIO 2** on ESP8266 (ESP-01)

### Flash ESP32 at https://keml4.github.io/Headless-TvBgone/.

Pick NA/Asia or Europe.

Thanks to Aryann019x for fixing Web flasher

## Building with Arduino IDE (manually)

1. Install the board package for your board:
   - ESP32 / ESP32-S3: **esp32 by Espressif Systems** (board: "ESP32S3 Dev Module" for S3)
   - ESP8266: **esp8266 by ESP8266 Community** (board: "Generic ESP8266 Module" or "ESP-01S")
   - Arduino Uno R3: no board package needed; use `Uno-Tv-B-Gone/Uno-Tv-B-Gone.ino` (no library required)
2. Install the **IRremoteESP8266** library by `crankyoldgit`.
3. Open one sketch:
   - `NA-Tv-B-Gone/NA-Tv-B-Gone.ino` or `EU-Tv-B-Gone/EU-Tv-B-Gone.ino` (ESP32)
   - `ESP8266-NA-Tv-B-Gone/ESP8266-NA-Tv-B-Gone.ino` or `ESP8266-EU-Tv-B-Gone/ESP8266-EU-Tv-B-Gone.ino` (ESP8266)
4. Select your board and serial port.
5. Upload.

All four sketches share `tvbgoner_transmitter.hpp`. The IR LED pin defaults to GPIO 4 on ESP32 and GPIO 2 on ESP8266 and can be overridden with `-DIR_LED_PIN=n`.

## How the codes work

`ir_codes_na.hpp` / `ir_codes_eu.hpp` hold TV-B-Gone-style raw frames: a times table in 10 us units plus bytes of 2-bit indices into (mark, space) pairs. The NEC (Toshiba, LG), Samsung, Sony SIRC-12, and RC-5 (EU) frames are generated from the protocol timings used by IRremoteESP8266 and verified round-trip by `tools/generate_codes.py` — run `python tools/generate_codes.py` to regenerate both tables after adding codes.
