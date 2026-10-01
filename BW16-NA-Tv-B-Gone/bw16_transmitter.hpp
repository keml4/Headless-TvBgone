#include <Arduino.h>
#include <IRremote.hpp>

#ifdef BW16_NA
#include "../NA-Tv-B-Gone/ir_codes_na.hpp"
constexpr const char* kRegionName = "NA/Asia";
#else
#include "../EU-Tv-B-Gone/ir_codes_eu.hpp"
constexpr const char* kRegionName = "EU";
#endif

namespace {
// BW16 GPIO numbers are the GPIO numbers, not ESP32 GPIO labels.
// Move the IR LED to another GPIO by changing this value if required.
constexpr uint16_t kIrLedPin = 12;
constexpr uint32_t kSerialBaud = 115200;
constexpr uint16_t kInterCodeDelayMs = 5;
uint16_t rawData[300];

bool expandCode(const IrCode& code) {
  uint16_t codeByte = 0;
  uint8_t bitsLeft = 0;
  uint16_t codePtr = 0;

  for (uint8_t pair = 0; pair < code.numPairs; ++pair) {
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
    rawData[pair * 2] = code.times[timingIndex] * 10;
    rawData[pair * 2 + 1] = code.times[timingIndex + 1] * 10;
  }
  return true;
}

void sendCode(uint8_t index) {
  const IrCode& code = *kPowerCodes[index];
  if (!expandCode(code)) {
    Serial.println(F("Invalid timing data; transmission skipped."));
    return;
  }

  // Arduino-IRremote is portable to AmebaD/BW16. Its raw sender takes
  // microsecond durations and the carrier frequency in kHz.
  IrSender.sendRaw(rawData, code.numPairs * 2, code.timerValue);
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
  IrSender.begin(kIrLedPin);
  Serial.print(kRegionName);
  Serial.print(F(" BW16 transmitter ready: "));
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
