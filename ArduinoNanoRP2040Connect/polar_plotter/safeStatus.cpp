#include "safeStatus.h"

SafeStatus::SafeStatus(SafePrinter printer
#if USE_BLE > 0
    , BLEService& bleService
#endif
  )
  : printer(printer)
#if USE_LCD > 0
  , lcd(LiquidCrystal(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7))
#endif
#if USE_BLE > 0
  , bleService(bleService),
  bleMaxRadius(BLEDoubleCharacteristic(BLE_MAX_RADIUS_UUID, BLERead | BLENotify)),
  bleRadiusStepSize(BLEDoubleCharacteristic(BLE_RADIUS_STEP_SIZE_UUID, BLERead | BLENotify)),
  bleAzimuthStepSize(BLEDoubleCharacteristic(BLE_AZIMUTH_STEP_SIZE_UUID, BLERead | BLENotify)),
  bleMarbleSize(BLEIntCharacteristic(BLE_MARBLE_SIZE_UUID, BLERead | BLENotify)),
  bleStatus(BLEStringCharacteristic(BLE_STATUS_UUID, BLERead | BLENotify, BLE_STRING_SIZE)),
  bleDrawing(BLEStringCharacteristic(BLE_DRAWING_UUID, BLERead | BLENotify, BLE_STRING_SIZE)),
  bleCommand(BLEStringCharacteristic(BLE_STEP_UUID, BLERead | BLENotify, BLE_STRING_SIZE)),
  bleStep(BLEIntCharacteristic(BLE_MARBLE_SIZE_UUID, BLERead | BLENotify)),
  bleRadius(BLEDoubleCharacteristic(BLE_RADIUS_UUID, BLERead | BLENotify)),
  bleAzimuth(BLEDoubleCharacteristic(BLE_AZIMUTH_UUID, BLERead | BLENotify)),
  bleState(BLEStringCharacteristic(BLE_STATE_UUID, BLERead | BLENotify, BLE_STRING_SIZE))
#endif
{
  this->lastKey = "";
  this->lastValue = "";
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
  bleService.addCharacteristic(bleMaxRadius);
  bleService.addCharacteristic(bleRadiusStepSize);
  bleService.addCharacteristic(bleAzimuthStepSize);
  bleService.addCharacteristic(bleMarbleSize);
  bleService.addCharacteristic(bleStatus);
  bleService.addCharacteristic(bleDrawing);
  bleService.addCharacteristic(bleCommand);
  bleService.addCharacteristic(bleStep);
  bleService.addCharacteristic(bleRadius);
  bleService.addCharacteristic(bleAzimuth);
  bleService.addCharacteristic(bleState);
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

#if USE_BLE > 0
void setStringValue(BLECharacteristic &characteristic, const char * name, const String &value) {
  String val = value;
  if (val.length() > BLE_STRING_SIZE) {
    val = val.substring(0, BLE_STRING_SIZE);
  }

#if BLE_DEBUG > 0
  printer.print("Writing BLE ");
  printer.print(name);
  printer.print(": ");
  printer.print(val);
  printer.print(" - ");
#endif
  int ret = characteristic.writeValue(val.c_str(), false);
#if BLE_DEBUG > 0
  printer.println(ret);
#endif
}

void setDoubleValue(BLECharacteristic &characteristic, const char * name, const double value) {
#if BLE_DEBUG > 0
  printer.print("Writing BLE ");
  printer.print(name);
  printer.print(": ");
  printer.print(value);
  printer.print(" - ");
#endif

  byte arr[8];
  memcpy(arr, (uint8_t *) &value, 8);
  int ret = characteristic.writeValue(arr, 8, false);
#if BLE_DEBUG > 0
  printer.println(ret);
#endif
}

void setIntValue(BLECharacteristic &characteristic, const char * name, const int value) {
#if BLE_DEBUG > 0
  printer.print("Writing BLE ");
  printer.print(name);
  printer.print(": ");
  printer.print(value);
  printer.print(" - ");
#endif

  byte arr[4];
  memcpy(arr, (uint8_t *) &value, 4);
  int ret = characteristic.writeValue(arr, 4, false);
#if BLE_DEBUG > 0
  printer.println(ret);
#endif
}
#endif

void SafeStatus::setMaxRadius(const double value) {
#if USE_BLE > 0
  setDoubleValue(bleMaxRadius, "Max Radius", value);
#endif
}

void SafeStatus::setRadiusStepSize(const double value) {
#if USE_BLE > 0
  setDoubleValue(bleRadiusStepSize, "Radius Step Size", value);
#endif
}

void SafeStatus::setAzimuthStepSize(const double value) {
#if USE_BLE > 0
  setDoubleValue(bleAzimuthStepSize, "Azimuth Step Size", value);
#endif
}

void SafeStatus::setMarbleSizeInRadiusSteps(const int value) {
#if USE_BLE > 0
  setIntValue(bleMarbleSize, "Marble Size in Radius Steps", value);
#endif
}

void SafeStatus::setCurrentDrawing(const String &value) {
#if USE_BLE > 0
  setStringValue(bleDrawing, "Drawing", value);
#endif
}

void SafeStatus::setCurrentCommand(const String &value) {
#if USE_BLE > 0
  setStringValue(bleCommand, "Command", value);
#endif
}

void SafeStatus::setCurrentStep(const int value) {
#if USE_BLE > 0
  unsigned long cur = millis();
  if (cur < nextStepUpdate) return;

  nextStepUpdate = cur + STEP_UPDATE_INTERVAL;
  setIntValue(bleStep, "Step", value);
#endif
}

void SafeStatus::setPosition(const double radius, const double azimuth) {
#if USE_BLE > 0
  unsigned long cur = millis();
  if (cur < nextPositionUpdate) return;

  nextPositionUpdate = cur + POSITION_UPDATE_INTERVAL;
  setDoubleValue(bleRadius, "Radius", radius);
  setDoubleValue(bleAzimuth, "Azimuth", azimuth);
#endif
}

void SafeStatus::setState(const String &value) {
#if USE_BLE > 0
  setStringValue(bleState, "State", value);
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
  setStringValue(bleStatus, "Status", key + (key.length() > 0 && value.length() > 0 ? ": " : "") + value);
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
