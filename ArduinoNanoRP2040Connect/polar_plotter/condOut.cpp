#include "condOut.h"

CondOut::CondOut(LiquidCrystal& lcd)
  : lcd(lcd) {
}

void CondOut::init() {
#if USE_LCD > 0
  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Setup");
#endif
  Serial.begin(115200);
  unsigned long start = millis();

  // Wait for up to 3 seconds for the Serial device to become available
  while (!Serial && (millis() - start) <= 3000)
    ;

  Serial.println("Starting");
}

void CondOut::print(String& value) {
  if (Serial) {
    Serial.print(value);
  }
}

void CondOut::print(const char value[]) {
  if (Serial) {
    Serial.print(value);
  }
}

void CondOut::print(int value, int base) {
  if (Serial) {
    Serial.print(value, base);
  }
}

void CondOut::print(unsigned int value, int base) {
  if (Serial) {
    Serial.print(value, base);
  }
}

void CondOut::print(float value, int base) {
  if (Serial) {
    Serial.print(value, base);
  }
}

void CondOut::print(double value, int base) {
  if (Serial) {
    Serial.print(value, base);
  }
}

void CondOut::print(long value, int base) {
  if (Serial) {
    Serial.print(value, base);
  }
}

void CondOut::print(unsigned long value, int base) {
  if (Serial) {
    Serial.print(value, base);
  }
}

void CondOut::print(String name, Point& value) {
  if (Serial) {
    Serial.print(name);
    Serial.print("=(");
    Serial.print(value.getX(), 4);
    Serial.print(", ");
    Serial.print(value.getY(), 4);
    Serial.print(", ");
    Serial.print(value.getRadius(), 4);
    Serial.print(", ");
    Serial.print(value.getAzimuth(), 4);
    Serial.print(")");
  }
}

void CondOut::println(String& value) {
  if (Serial) {
    Serial.println(value);
  }
}

void CondOut::println(const char value[]) {
  if (Serial) {
    Serial.println(value);
  }
}

void CondOut::println(int value, int base) {
  if (Serial) {
    Serial.println(value, base);
  }
}

void CondOut::println(unsigned int value, int base) {
  if (Serial) {
    Serial.println(value, base);
  }
}

void CondOut::println(float value, int base) {
  if (Serial) {
    Serial.println(value, base);
  }
}

void CondOut::println(double value, int base) {
  if (Serial) {
    Serial.println(value, base);
  }
}

void CondOut::println(long value, int base) {
  if (Serial) {
    Serial.println(value, base);
  }
}

void CondOut::println(unsigned long value, int base) {
  if (Serial) {
    Serial.println(value, base);
  }
}

void CondOut::println(String name, Point& value) {
  if (Serial) {
    this->print(name, value);
    Serial.println();
  }
}

void CondOut::clear() {
#if USE_LCD > 0
  lcd.clear();
#endif
}

void CondOut::setCursor(int col, int row) {
#if USE_LCD > 0
  lcd.setCursor(col, row);
#endif
}

void CondOut::lcdPrint(String& value) {
#if USE_LCD > 0
  lcd.print(value);
#endif
}

void CondOut::lcdPrint(const char value[]) {
#if USE_LCD > 0
  lcd.print(value);
#endif
}

void CondOut::lcdPrint(long value, int base) {
#if USE_LCD > 0
  lcd.print(value, base);
#endif
}

void CondOut::lcdPrint(const char line1[], String& line2) {
#if USE_LCD > 0
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
#endif
}

void CondOut::lcdPrint(const char line1[], const char line2[]) {
#if USE_LCD > 0
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
#endif
}
