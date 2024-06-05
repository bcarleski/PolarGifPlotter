#include <Arduino.h>

class SafePrinter : public Print {
public:
  void init() {
    Serial.begin(115200);
    unsigned long start = millis();

    // Wait for up to 3 seconds for the Serial device to become available
    while (!Serial && (millis() - start) <= 3000)
      ;

    if (Serial) Serial.println("Starting");
  }

  size_t write(uint8_t c) {
    if (Serial) {
      Serial.write(c);
    }
  }

  size_t write(const uint8_t*, size_t) {
    if (Serial) {
      Serial.write(str, len);
    }
  }
  using Print::write;  // pull in write(str) and write(buf, size) from Print
  using Print::print;
  using Print::println;
};
