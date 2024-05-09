#include <Arduino.h>
#include "statusUpdate.h"
#include "constants.h"

#if USE_LCD > 0
#include <LiquidCrystal.h>
#endif
#if USE_BLE > 0
#include <ArduinoBLE.h>
#endif

class SafeStatus : public StatusUpdate {
private:
#if USE_LCD > 0
  LiquidCrystal lcd;
#endif
#if USE_BLE > 0
  BLEService& bleService;
  BLECharacteristic bleMaxRadius;
  BLECharacteristic bleRadiusStepSize;
  BLECharacteristic bleAzimuthStepSize;
  BLECharacteristic bleMarbleSize;
  BLEStringCharacteristic bleStatus;
  BLEStringCharacteristic bleDrawing;
  BLEStringCharacteristic bleStep;
  BLEStringCharacteristic blePosition;
  BLEStringCharacteristic bleState;
#endif
  String lastKey;
  String lastValue;
  bool preservePair;

  void safeLcdPrint(const String &value);
  void safeBlePrint(const String &value);

public:
  SafeStatus(
#if USE_BLE > 0
    BLEService& bleService
#endif
  );
  void init();

  void writeStatus(const String &key, const String &value);
  void setMaxRadius(const double value);
  void setRadiusStepSize(const double value);
  void setAzimuthStepSize(const double value);
  void setMarbleSizeInRadiusSteps(const int value);
  void setCurrentDrawing(const String &value);
  void setCurrentStep(const String &value);
  void setPosition(const String &value);
  void setState(const String &value);

  void save();
  void restore();
};
