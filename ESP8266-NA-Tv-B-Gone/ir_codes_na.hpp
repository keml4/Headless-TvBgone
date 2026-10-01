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

// Add further North American/Asian entries here.
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

// Samsung power, 0xE0E09966
constexpr uint16_t kSam001Times[] = {448, 448, 56, 168, 56, 56, 56, 500};
constexpr uint8_t kSam001Codes[] = {0x15, 0xAA, 0x95, 0xAA, 0x9A, 0x5A, 0x65, 0xA5, 0xB0};
constexpr IrCode kSam001Code = {38, 34, 2, 8, sizeof(kSam001Codes), kSam001Times, kSam001Codes};

// Sony SIRC-12 power, classic 0xA90
constexpr uint16_t kSony000Times[] = {240, 60, 60, 60, 120, 60, 60, 500};
constexpr uint8_t kSony000Codes[] = {0x15, 0x65, 0x99, 0xB0};
constexpr IrCode kSony000Code = {40, 14, 2, 8, sizeof(kSony000Codes), kSony000Times, kSony000Codes};

// Sony SIRC-12 power, alternate 0xA91
constexpr uint16_t kSony001Times[] = {240, 60, 120, 60, 60, 60, 60, 500};
constexpr uint8_t kSony001Codes[] = {0x1A, 0x9A, 0x66, 0x70};
constexpr IrCode kSony001Code = {40, 14, 2, 8, sizeof(kSony001Codes), kSony001Times, kSony001Codes};

constexpr const IrCode* kPowerCodes[] = {&kNec000, &kNec001, &kNec002, &kSam000, &kSam001, &kSony000, &kSony001};
constexpr uint8_t kPowerCodeCount = sizeof(kPowerCodes) / sizeof(kPowerCodes[0]);
