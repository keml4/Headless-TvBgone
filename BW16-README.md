# BW16 support

The repository includes sketches for the Realtek RTL8720DN BW16 board:

- `BW16-NA-Tv-B-Gone/BW16-NA-Tv-B-Gone.ino`
- `BW16-EU-Tv-B-Gone/BW16-EU-Tv-B-Gone.ino`

The web page links to these instructions, but does not pretend to flash BW16 firmware: the AmebaD bootloader does not expose a documented Web Serial/WebUSB protocol compatible with ESP Web Tools. Upload BW16 firmware with the AmebaD Arduino tools.

## Hardware

- BW16 (RTL8720DN)
- IR LED on **GPIO 12** through an appropriate current-limiting/transistor driver

GPIO 12 is a default and can be changed in `BW16-NA-Tv-B-Gone/bw16_transmitter.hpp`.

## Arduino IDE

1. Add the AmebaD package URL to Additional Boards Manager URLs:
   `https://github.com/ambiot/ambd_arduino/raw/master/Arduino_package/package_realtek.com_amebad_index.json`
2. Install **AmebaD Boards** and select **BW16**.
3. Install **Arduino-IRremote**.
4. Open either BW16 sketch and upload it.
