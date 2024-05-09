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
  bleMaxRadius(BLECharacteristic(BLE_MAX_RADIUS_UUID, BLERead | BLENotify, 4)),
  bleRadiusStepSize(BLECharacteristic(BLE_RADIUS_STEP_SIZE_UUID, BLERead | BLENotify, 4)),
  bleAzimuthStepSize(BLECharacteristic(BLE_AZIMUTH_STEP_SIZE_UUID, BLERead | BLENotify, 4)),
  bleMarbleSize(BLECharacteristic(BLE_MARBLE_SIZE_UUID, BLERead | BLENotify, 4)),
  bleStatus(BLEStringCharacteristic(BLE_STATUS_UUID, BLERead | BLENotify, BLE_STRING_SIZE)),
  bleDrawing(BLEStringCharacteristic(BLE_DRAWING_UUID, BLERead | BLENotify, BLE_STRING_SIZE)),
  bleStep(BLEStringCharacteristic(BLE_STEP_UUID, BLERead | BLENotify, BLE_STRING_SIZE)),
  blePosition(BLEStringCharacteristic(BLE_POSITION_UUID, BLERead | BLENotify, BLE_STRING_SIZE)),
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
  bleService.addCharacteristic(bleStep);
  bleService.addCharacteristic(blePosition);
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

void SafeStatus::safeBlePrint(const String &value) {
  String val = value;
  if (val.length() > BLE_STRING_SIZE) {
    val = val.substring(0, BLE_STRING_SIZE);
  }

#if USE_BLE > 0
  bleStatus.writeValue(val);
  Serial.print("Writing BLE status: ");
  Serial.println(val);
#endif
}

void setFloatValue(BLECharacteristic &characteristic, const float value) {
  byte arr[4];
  memcpy(arr, (uint8_t *) &value, 4);
  characteristic.writeValue(arr, 4);
}

void setIntValue(BLECharacteristic &characteristic, const int value) {
  byte arr[4];
  memcpy(arr, (uint8_t *) &value, 4);
  characteristic.writeValue(arr, 4);
}

void SafeStatus::setMaxRadius(const float value) {
#if USE_BLE > 0
  setFloatValue(bleMaxRadius, value);
  Serial.print("Writing BLE max radius: ");
  Serial.println(value);
#endif
}

void SafeStatus::setRadiusStepSize(const float value) {
#if USE_BLE > 0
  setFloatValue(bleRadiusStepSize, value);
  Serial.print("Writing BLE radius step size: ");
  Serial.println(value);
#endif
}

void SafeStatus::setAzimuthStepSize(const float value) {
#if USE_BLE > 0
  setFloatValue(bleAzimuthStepSize, value);
  Serial.print("Writing BLE azimuth step size: ");
  Serial.println(value);
#endif
}

void SafeStatus::setMarbleSizeInRadiusSteps(const int value) {
#if USE_BLE > 0
  setIntValue(bleMarbleSize, value);
  Serial.print("Writing BLE marble size in radius steps: ");
  Serial.println(value);
#endif
}

void SafeStatus::setCurrentDrawing(const String &value) {
#if USE_BLE > 0
  String val = value;
  if (val.length() > BLE_STRING_SIZE) {
    val = val.substring(0, BLE_STRING_SIZE);
  }

  bleDrawing.writeValue(val);
  Serial.print("Writing BLE drawing: ");
  Serial.println(val);
#endif
}

void SafeStatus::setCurrentStep(const String &value) {
#if USE_BLE > 0
  String val = value;
  if (val.length() > BLE_STRING_SIZE) {
    val = val.substring(0, BLE_STRING_SIZE);
  }

  bleStep.writeValue(val);
  Serial.print("Writing BLE current step: ");
  Serial.println(val);
#endif
}

void SafeStatus::setPosition(const String &value) {
#if USE_BLE > 0
  String val = value;
  if (val.length() > BLE_STRING_SIZE) {
    val = val.substring(0, BLE_STRING_SIZE);
  }

  blePosition.writeValue(val);
  Serial.print("Writing BLE position: ");
  Serial.println(val);
#endif
}

void SafeStatus::setState(const String &value) {
#if USE_BLE > 0
  String val = value;
  if (val.length() > BLE_STRING_SIZE) {
    val = val.substring(0, BLE_STRING_SIZE);
  }

  bleState.writeValue(val);
  Serial.print("Writing BLE state: ");
  Serial.println(val);
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
