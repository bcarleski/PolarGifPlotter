#include "safeStatus.h"

void SafeStatus::writeStatus(const String &key, const String &value) {
  if (bleUpdater) bleUpdater->status(key, value);

  if (!this->preservePair) {
    this->lastKey = key;
    this->lastValue = value;
  }
}


#if USE_BLE > 0
void BleSafeStatus::setStringValue(BLECharacteristic &characteristic, const char * name, const String &value) {
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

void BleSafeStatus::setDoubleValue(BLECharacteristic &characteristic, const char * name, const double value) {
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

void BleSafeStatus::setIntValue(BLECharacteristic &characteristic, const char * name, const int value) {
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
