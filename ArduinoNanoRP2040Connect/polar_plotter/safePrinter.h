#ifndef _POLAR_PLOTTER_SAFEPRINTER_
#define _POLAR_PLOTTER_SAFEPRINTER_

#include <Arduino.h>
#include "constants.h"

class SafePrinter : public Print {
public:
  void init() {
#if USE_SERIAL > 0
    Serial.begin(115200);
    unsigned long start = millis();

    // Wait for up to 3 seconds for the Serial device to become available
    while (!Serial && (millis() - start) <= 3000)
      ;

    if (Serial) Serial.println("Starting");
#endif
  }

  size_t write(uint8_t c) {
#if USE_SERIAL > 0
    if (Serial) {
      Serial.write(c);
    }
#endif
  }

  size_t write(const uint8_t* str, size_t len) {
#if USE_SERIAL > 0
    if (Serial) {
      Serial.write(str, len);
    }
#endif
  }
  using Print::write;  // pull in write(str) and write(buf, size) from Print
  using Print::print;
  using Print::println;
};
#endif
