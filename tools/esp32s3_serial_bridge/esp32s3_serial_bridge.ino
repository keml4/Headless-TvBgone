// ESP32-S3 USB <-> UART bridge for flashing ESP-01/ESP-01S (ESP8266) modules.
//
// Flash this to the S3 first (board "ESP32S3 Dev Module", USB CDC On Boot:
// Enabled). The S3 then shows up as a plain USB serial port that pipes
// everything to the ESP-01's bootloader.
//
// Wiring (ESP32-S3 -> ESP-01S, all 3.3V, no level shifter needed):
//   3V3   -> VCC  and CH_PD (EN)
//   GND   -> GND
//   GPIO17 (TX) -> ESP RX
//   GPIO18 (RX) -> ESP TX
//   GPIO4       -> ESP GPIO0   (flash-mode control)
//   GPIO5       -> ESP RST
//
// Serial Monitor commands (only recognized when the line has been idle
// for 2 s, so they can never fire in the middle of an upload):
//   F  = hold GPIO0 low + reset the ESP-01  -> bootloader (flash) mode
//   R  = release GPIO0 + reset              -> run the uploaded sketch
//
// Flashing flow: type F in the monitor, close the monitor, then hit Upload
// in the IDE with the ESP8266 board settings for the ESP-01. After the
// upload, reopen the monitor and type R to start the sketch.

#include <Arduino.h>

namespace {
constexpr uint8_t kPinRx = 18;    // S3 RX -> ESP-01 TX
constexpr uint8_t kPinTx = 17;    // S3 TX -> ESP-01 RX
constexpr uint8_t kPinIo0 = 4;    // -> ESP-01 GPIO0
constexpr uint8_t kPinRst = 5;    // -> ESP-01 RST
constexpr uint32_t kBaud = 115200;
constexpr uint32_t kIdleGateMs = 2000;

uint32_t lastForwardMs = 0;

void pulseReset() {
  pinMode(kPinRst, OUTPUT);
  digitalWrite(kPinRst, LOW);
  delay(120);
  pinMode(kPinRst, INPUT);  // module's own pull-up takes over
}

void enterFlashMode() {
  pinMode(kPinIo0, OUTPUT);
  digitalWrite(kPinIo0, LOW);
  delay(10);
  pulseReset();
  Serial.println("[bridge] ESP-01 in FLASH mode (GPIO0 low). Upload now.");
}

void enterRunMode() {
  pinMode(kPinIo0, INPUT);  // release; module pull-up makes it high
  delay(10);
  pulseReset();
  Serial.println("[bridge] ESP-01 is running the uploaded sketch.");
}

bool lineIdle() {
  return millis() - lastForwardMs > kIdleGateMs;
}
}  // namespace

void setup() {
  Serial.begin(kBaud);                                   // USB-CDC side
  Serial1.setRxBufferSize(1024);
  Serial1.setTxBufferSize(1024);
  Serial1.begin(kBaud, SERIAL_8N1, kPinRx, kPinTx);      // ESP-01 side
  pinMode(kPinIo0, INPUT);
  pinMode(kPinRst, INPUT);
  delay(500);
  Serial.println();
  Serial.println("[bridge] ESP32-S3 <-> ESP-01 ready at 115200");
  Serial.println("[bridge] type F to enter flash mode, R to run the sketch");
}

void loop() {
  // USB -> UART (with idle-gated single-letter commands)
  while (Serial.available()) {
    const uint8_t b = Serial.read();
    if (lineIdle() && (b == 'F' || b == 'R')) {
      while (Serial.available()) {  // swallow the newline
        const uint8_t n = Serial.peek();
        if (n != '\r' && n != '\n') break;
        Serial.read();
      }
      if (b == 'F') enterFlashMode(); else enterRunMode();
      continue;
    }
    Serial1.write(b);
    lastForwardMs = millis();
  }

  // UART -> USB
  while (Serial1.available()) {
    Serial.write(Serial1.read());
  }
}
