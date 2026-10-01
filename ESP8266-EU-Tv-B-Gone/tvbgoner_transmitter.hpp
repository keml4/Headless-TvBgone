#pragma once

#include <Arduino.h>
#include <IRsend.h>

#ifdef TVBGONE_NA
#include "ir_codes_na.hpp"
constexpr const char* kRegionName = "NA/Asia";
#else
#include "ir_codes_eu.hpp"
constexpr const char* kRegionName = "EU";
#endif

// Default IR LED pin: GPIO 2 on the ESP-01 (its only free pin); GPIO 4 on
// ESP32 dev boards. Override with -DIR_LED_PIN=n for anything else.
#if defined(ESP32) && !defined(IR_LED_PIN)
#define IR_LED_PIN 4
#endif
#ifndef IR_LED_PIN
#define IR_LED_PIN 2
#endif

namespace {
constexpr uint32_t kSerialBaud = 115200;
constexpr uint16_t kInterCodeDelayMs = 5;
constexpr uint16_t kRepeatGapMs = 45;  // gap between repeated frames
constexpr uint16_t kMaxRawDurations = 300;

IRsend irsend(IR_LED_PIN);
uint16_t rawData[kMaxRawDurations];

// Each index consumes bitsPerIndex bits, most-significant bit first, exactly
// like the reference TV-B-Gone packs them (two indices per byte, high nibble
// first).
bool expandCode(const IrCode& code) {
  uint8_t bitsLeft = 0;
  uint16_t codeByte = 0;
  uint16_t codePtr = 0;

  for (uint16_t pair = 0; pair < code.numPairs; ++pair) {
    uint16_t index = 0;
    for (uint8_t bit = 0; bit < code.bitsPerIndex; ++bit) {
      if (bitsLeft == 0) {
        if (codePtr >= code.codeBytes) return false;
        codeByte = code.codes[codePtr++];
        bitsLeft = 8;
      }
      --bitsLeft;
      index = static_cast<uint16_t>((index << 1) | ((codeByte >> bitsLeft) & 1));
    }

    const uint16_t timingIndex = index * 2;
    if (timingIndex + 1 >= code.timeValues) return false;
    // Times are stored in 10-us units; clamp each entry so a long gap can
    // never overflow the uint16_t send buffer.
    rawData[pair * 2] = static_cast<uint16_t>(
        min(code.times[timingIndex] * 10UL, 0xFFFFUL));
    rawData[pair * 2 + 1] = static_cast<uint16_t>(
        min(code.times[timingIndex + 1] * 10UL, 0xFFFFUL));
  }
  return true;
}

void sendCode(uint8_t index) {
  const IrCode& code = *kPowerCodes[index];
  if (!expandCode(code)) {
    Serial.print(F("Invalid "));
    Serial.print(kRegionName);
    Serial.println(F(" code data; transmission skipped."));
    return;
  }

  // Sony SIRC (40 kHz) frames must be sent at least 3 times to be accepted;
  // the other protocols work from a single frame.
  const uint8_t extraFrames = (code.timerValue >= 40) ? 2 : 0;
  for (uint8_t frame = 0; frame <= extraFrames; ++frame) {
    irsend.sendRaw(rawData, code.numPairs * 2, code.timerValue);
    if (frame < extraFrames) delay(kRepeatGapMs);
  }

  Serial.print(F("Sent "));
  Serial.print(kRegionName);
  Serial.print(F(" code "));
  Serial.print(index + 1);
  Serial.print('/');
  Serial.println(kPowerCodeCount);
}
}  // namespace

void setup() {
  Serial.begin(kSerialBaud);
  delay(500);
  irsend.begin();
  Serial.print(kRegionName);
  Serial.print(F(" transmitter ready on GPIO "));
  Serial.print(IR_LED_PIN);
  Serial.print(F(": "));
  Serial.print(kPowerCodeCount);
  Serial.println(F(" timing codes"));
}

void loop() {
  for (uint8_t index = 0; index < kPowerCodeCount; ++index) {
    sendCode(index);
    delay(kInterCodeDelayMs);
    yield();
  }
  Serial.print(kRegionName);
  Serial.println(F(" cycle complete"));
  delay(2000);
}
