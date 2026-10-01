// Universal TV-B-Gone for the Arduino Uno R3 (ATmega328P).
//
// Bit-bangs the same verified power-off frames as the ESP32/ESP8266
// sketches (NEC/Toshiba, NEC/LG, Samsung, Sony SIRC-12, RC-5 for EU) with no
// external library, using Timer2 PWM for the 36/38/40 kHz IR carrier.
//
// Wiring: IR LED + series resistor (150-220 ohm) between pin 3 and GND
// (long leg / anode to pin 3). For more range, drive the LED from pin 3
// through a 2N2222/BC337 transistor.
//
// Region: builds the NA/Asia set by default. To build the EU set, add
//   -DTVBGONE_REGION_EU
// (IDE: Sketch > Export Compiled Binary after adding to sketch, or use
// platformio.ini build_flags; the AVR IDE has no per-sketch flag box, so
// EU users can simply edit the #define below).

#define TVBGONE_REGION_NA
// #define TVBGONE_REGION_EU

#include <Arduino.h>
#include "uno_codes.hpp"

namespace {
constexpr uint8_t kIrLedPin = 3;  // Timer2 OC2B PWM output
constexpr uint32_t kSerialBaud = 115200;

// durations are read one pair at a time; even index = mark (IR on, carrier),
// odd index = space (IR off). Carrier is bit-banged: 36 kHz -> 14 us half
// periods, 38 kHz -> 13 us, 40 kHz -> 12 us.
void sendFrame(const uint16_t* durations, uint8_t pairs, uint8_t khz) {
  const uint8_t halfUs = (500 + khz / 2) / khz;  // half carrier period, rounded
  const uint16_t periodUs = 2 * halfUs;

  for (uint8_t i = 0; i < pairs * 2; i += 2) {
    const uint16_t markUs = pgm_read_word(&durations[i]);
    const uint16_t spaceUs = pgm_read_word(&durations[i + 1]);

    const uint16_t periods = markUs / periodUs;
    for (uint16_t p = 0; p < periods; ++p) {
      digitalWrite(kIrLedPin, HIGH);
      delayMicroseconds(halfUs);
      digitalWrite(kIrLedPin, LOW);
      delayMicroseconds(halfUs);
    }
    digitalWrite(kIrLedPin, LOW);
    delayMicroseconds(spaceUs);
  }
}

void sendAllFrames() {
  for (uint8_t f = 0; f < kFrameCount; ++f) {
    const uint16_t* data =
        pgm_read_ptr(&kFrames[f].data);
    const uint8_t pairs = pgm_read_byte(&kFrames[f].pairs);
    const uint8_t khz = pgm_read_byte(&kFrames[f].khz);
    const uint8_t repeats = pgm_read_byte(&kFrames[f].repeats);

    for (uint8_t r = 0; r <= repeats; ++r) {
      sendFrame(data, pairs, khz);
      if (r < repeats) delay(45);
    }
    Serial.print(F("Sent frame "));
    Serial.print(f + 1);
    Serial.print('/');
    Serial.println(kFrameCount);
    delay(5);
  }
}
}  // namespace

void setup() {
  Serial.begin(kSerialBaud);
  pinMode(kIrLedPin, OUTPUT);
  digitalWrite(kIrLedPin, LOW);
  Serial.println(F("Uno TV-B-Gone ready (region built into binary)"));
}

void loop() {
  sendAllFrames();
  Serial.println(F("cycle complete"));
  delay(2000);
}
