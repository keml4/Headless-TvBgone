#include <Arduino.h>

#if defined(ARDUINO_ARCH_AVR) || defined(__AVR__)
#define DISABLE_CODE_FOR_RECEIVER
#include <IRremote.hpp>

class AvrIRSenderWrapper {
public:
  AvrIRSenderWrapper(uint8_t pin) : pin_(pin) {}
  void begin() { IrSender.begin(pin_); }
  void sendSAMSUNG(uint32_t data, uint8_t nbits) { IrSender.sendSAMSUNG(data, nbits); }
  void sendNEC(uint32_t data, uint8_t nbits) { IrSender.sendNECMSB(data, nbits); }
  void sendSony(uint32_t data, uint8_t nbits) { IrSender.sendSony(data, nbits); }
  void sendRaw(const uint16_t *buf, uint16_t len, uint8_t hz) { IrSender.sendRaw(buf, len, hz); }
private:
  uint8_t pin_;
};
#define IR_SENDER_CLASS AvrIRSenderWrapper
#else
#include <IRsend.h>
#define IR_SENDER_CLASS IRsend
#endif

#include "ir_codes_na.hpp"

namespace {
// Default IR LED pin: 4 (ESP32-style in original). Use pin 12 for Arduino Uno R3 clones
// which commonly define ARDUINO_AVR_UNO. This allows the sketch to work on Unos by
// selecting pin 12 automatically when compiled for that board.
#if defined(ARDUINO_AVR_UNO)
constexpr uint16_t kIrLedPin = 12;
#else
constexpr uint16_t kIrLedPin = 4;
#endif
constexpr uint32_t kSerialBaud = 115200;
constexpr uint16_t kInterCodeDelayMs = 5;
constexpr uint16_t kLgInterCodeDelayMs = 100;
IR_SENDER_CLASS irsend(kIrLedPin);
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
    if (index >= code.timeValues) return false;
    const uint16_t timingIndex = index * 2;
    rawData[pair * 2] = code.times[timingIndex] * 10;
    rawData[pair * 2 + 1] = code.times[timingIndex + 1] * 10;
  }
  return true;
}

void sendCommonPowerCodes() {
  for (uint8_t i = 0; i < kSamsungCodeCount; ++i) {
    irsend.sendSAMSUNG(kSamsungPowerCodes[i], 32);
    delay(kInterCodeDelayMs);
  }
  for (uint8_t i = 0; i < kToshibaCodeCount; ++i) {
    irsend.sendNEC(kToshibaPowerCodes[i], 32);
    delay(kInterCodeDelayMs);
  }
  for (uint8_t i = 0; i < kSonyCodeCount; ++i) {
    irsend.sendSony(kSonyPowerCodes[i], 12);
    delay(kInterCodeDelayMs);
  }
  Serial.println(F("Sent Samsung, Toshiba, and Sony power codes"));
}

void sendLgPowerCodes() {
  for (uint8_t i = 0; i < kLgCodeCount; ++i) {
    irsend.sendNEC(kLgPowerCodes[i], 32);
    Serial.print(F("Sent LG power code "));
    Serial.print(i + 1);
    Serial.print('/');
    Serial.println(kLgCodeCount);
    delay(kLgInterCodeDelayMs);
    yield();
  }
}

void sendCode(uint8_t index) {
  const IrCode& code = *kPowerCodes[index];
  if (!expandCode(code)) {
    Serial.println(F("Invalid NA code data; transmission skipped."));
    return;
  }
  irsend.sendRaw(rawData, code.numPairs * 2, code.timerValue);
  Serial.print(F("Sent NA code "));
  Serial.print(index + 1);
  Serial.print('/');
  Serial.println(kPowerCodeCount);
}
}

void setup() {
  Serial.begin(kSerialBaud);
  delay(500);
  irsend.begin();
  Serial.print(F("NA transmitter ready: "));
  Serial.print(kPowerCodeCount);
  Serial.print(F(" NA codes plus "));
  Serial.print(kLgCodeCount);
  Serial.println(F(" LG codes"));
}

void loop() {
  sendLgPowerCodes();
  sendCommonPowerCodes();
  for (uint8_t index = 0; index < kPowerCodeCount; ++index) {
    sendCode(index);
    delay(kInterCodeDelayMs);
    yield();
  }
  Serial.println(F("NA cycle complete"));
  delay(2000);
}
