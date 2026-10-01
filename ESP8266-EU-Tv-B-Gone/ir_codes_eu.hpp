#pragma once

#include <Arduino.h>

struct IrCode {
  uint16_t timerValue;   // carrier frequency in kHz
  uint16_t numPairs;     // number of (mark, space) pairs in the frame
  uint8_t bitsPerIndex;  // bits per timing index (2)
  uint8_t timeValues;    // number of uint16_t entries in times[] (pairs * 2)
  uint16_t codeBytes;    // number of bytes in codes[]
  const uint16_t* times; // (mark, space) durations, units of 10 us
  const uint8_t* codes;  // packed 2-bit indices into times[] pairs, MSB first
};

// Add further European entries here.
// RC-5 power, addr 0 cmd 12 (TV), toggle 0
constexpr uint16_t kRc5000Times[] = {88, 88, 177, 88, 88, 177};
constexpr uint8_t kRc5000Codes[] = {0x10, 0x00, 0x84};
constexpr IrCode kRc5000Code = {36, 12, 2, 6, sizeof(kRc5000Codes), kRc5000Times, kRc5000Codes};

// RC-5 power, addr 0 cmd 12 (TV), toggle 1
constexpr uint16_t kRc5001Times[] = {88, 88, 177, 88, 88, 177};
constexpr uint8_t kRc5001Codes[] = {0x04, 0x00, 0x84};
constexpr IrCode kRc5001Code = {36, 12, 2, 6, sizeof(kRc5001Codes), kRc5001Times, kRc5001Codes};

// NEC power, Toshiba 0x02FD48B7
constexpr uint16_t kNec000Times[] = {900, 450, 56, 56, 56, 169, 56, 500};
constexpr uint8_t kNec000Codes[] = {0x15, 0x56, 0x6A, 0xA9, 0x99, 0x65, 0x66, 0x9A, 0xB0};
constexpr IrCode kNec000Code = {38, 34, 2, 8, sizeof(kNec000Codes), kNec000Times, kNec000Codes};

// NEC power, Toshiba 0x02FD08F7
constexpr uint16_t kNec001Times[] = {900, 450, 56, 56, 56, 169, 56, 500};
constexpr uint8_t kNec001Codes[] = {0x15, 0x56, 0x6A, 0xA9, 0x95, 0x65, 0x6A, 0x9A, 0xB0};
constexpr IrCode kNec001Code = {38, 34, 2, 8, sizeof(kNec001Codes), kNec001Times, kNec001Codes};

// NEC power, LG 0x20DF10EF
constexpr uint16_t kNec002Times[] = {900, 450, 56, 56, 56, 169, 56, 500};
constexpr uint8_t kNec002Codes[] = {0x16, 0x55, 0x69, 0xAA, 0x95, 0x95, 0x6A, 0x6A, 0xB0};
constexpr IrCode kNec002Code = {38, 34, 2, 8, sizeof(kNec002Codes), kNec002Times, kNec002Codes};

// Samsung power, 0xE0E040BF
constexpr uint16_t kSam000Times[] = {448, 448, 56, 168, 56, 56, 56, 500};
constexpr uint8_t kSam000Codes[] = {0x15, 0xAA, 0x95, 0xAA, 0xA6, 0xAA, 0x99, 0x55, 0x70};
constexpr IrCode kSam000Code = {38, 34, 2, 8, sizeof(kSam000Codes), kSam000Times, kSam000Codes};

// Sony SIRC-12 power, classic 0xA90
constexpr uint16_t kSony000Times[] = {240, 60, 60, 60, 120, 60, 60, 500};
constexpr uint8_t kSony000Codes[] = {0x15, 0x65, 0x99, 0xB0};
constexpr IrCode kSony000Code = {40, 14, 2, 8, sizeof(kSony000Codes), kSony000Times, kSony000Codes};

constexpr const IrCode* kPowerCodes[] = {&kRc5000, &kRc5001, &kNec000, &kNec001, &kNec002, &kSam000, &kSony000};
constexpr uint8_t kPowerCodeCount = sizeof(kPowerCodes) / sizeof(kPowerCodes[0]);
