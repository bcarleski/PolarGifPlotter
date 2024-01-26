#include <Arduino.h>

class SafePrinter : public Print {
public:
  void init();

  size_t write(uint8_t c);
  size_t write(const uint8_t*, size_t);
  using Print::write; // pull in write(str) and write(buf, size) from Print
  using Print::print;
  using Print::println;
};
