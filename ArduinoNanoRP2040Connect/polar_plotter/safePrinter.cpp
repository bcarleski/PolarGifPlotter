#include "safePrinter.h"


void SafePrinter::init() {
  Serial.begin(115200);
  unsigned long start = millis();

  // Wait for up to 3 seconds for the Serial device to become available
  while (!Serial && (millis() - start) <= 3000)
    ;

  if (Serial) Serial.println("Starting");
}

size_t SafePrinter::write(uint8_t c) {
  if (Serial) {
    Serial.write(c);
  }
}

size_t SafePrinter::write(const uint8_t* str, size_t len) {
  if (Serial) {
    Serial.write(str, len);
  }
}
