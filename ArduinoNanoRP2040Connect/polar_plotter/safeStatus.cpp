#include "safeStatus.h"

SafeStatus::SafeStatus(
#if USE_BLE > 0
    BLEService& bleService
#endif
  )
#if USE_LCD > 0 || USE_BLE > 0
  :
#endif
#if USE_LCD > 0
  lcd(LiquidCrystal(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7))
#endif
#if USE_LCD > 0 && USE_BLE > 0
  ,
#endif
#if USE_BLE > 0
  bleService(bleService),
  bleStatus(BLEStringCharacteristic(BLE_STATUS_UUID, BLERead | BLENotify, BLE_STATUS_SIZE))
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
#if USE_BLE > 0
  // add the characteristic to the service
  bleService.addCharacteristic(bleStatus);
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

void SafeStatus::safeBlePrint(const String &value) {
  String val = value;
  if (val.length() > BLE_STATUS_SIZE) {
    val = val.substring(0, BLE_STATUS_SIZE);
  }

#if USE_BLE > 0
  bleStatus.writeValue(val);
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
#if USE_BLE > 0
  this->safeBlePrint(key + (key.length() > 0 && value.length() > 0 ? ": " : "") + value);
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
