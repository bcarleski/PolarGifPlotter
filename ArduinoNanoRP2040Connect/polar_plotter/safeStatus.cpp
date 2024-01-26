#include "safeStatus.h"

SafeStatus::SafeStatus()
#if USE_LCD > 0
  : lcd(LiquidCrystal(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7))
#endif
{
}

void SafeStatus::init() {
#if USE_LCD > 0
  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Setup");
#endif
}

void SafeStatus::safeLcdPrint(const String &value) {
  String val = value;
  if (val.length() > 16) {
    val = val.substring(0, 16);
  }

#if USE_LCD > 0
  lcd.print(val);
#endif
}

void SafeStatus::writeStatus(const String &key, const String &value) {
#if USE_LCD > 0
  lcd.clear();
  lcd.setCursor(0, 0);
  this->safeLcdPrint(key);
  lcd.setCursor(0, 1);
  this->safeLcdPrint(value);
#endif

  if (!this->preservePair) {
    this->lastKey = key;
    this->lastValue = value;
  }
}

void SafeStatus::save() {
  this->preservePair = true;
}

void SafeStatus::restore() {
  this->writeStatus(this->lastKey, this->lastValue);
  this->preservePair = false;
}
